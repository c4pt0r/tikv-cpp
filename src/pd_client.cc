#include <string>
#include <chrono>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "logging.h"
#include "utils.hpp"
#include "cpp-btree/btree_map.h"

#include "pd_client.h"

namespace tikv {

pd_client::pd_client(const std::string& addr) {
  init(addr);
}

// public APIs 
Result<std::pair<region_info, peer_info>, Error>
pd_client::get_region(const std::string& key) {
  auto r = get_region_inner(get_leader_stub(), key);
  if (!r.isOk()) {
    LOG("rpc call error: " << r.unwrapErr().error);
    // force update leader rpc connect and retry
    return get_region_inner(get_leader_stub(true), key);
  }
  return r;
}

Result<store_info, Error>
pd_client::get_store_by_id(uint64_t store_id) {  
  auto r = get_store_by_id_inner(get_leader_stub(), store_id);
  if (!r.isOk()) {
    LOG("rpc call error: " << r.unwrapErr().error);
    // force update leader rpc connect and retry
    return get_store_by_id_inner(get_leader_stub(true), store_id);
  }
  return r; 
}

Result<std::vector<store_info>, Error> 
pd_client::get_all_stores() {
  auto r = get_all_stores_inner(get_leader_stub());
  if (!r.isOk()) {
    LOG("rpc call error: " << r.unwrapErr().error);
    // force update leader rpc connect and retry
    return get_all_stores_inner(get_leader_stub(true));
  }
  return r;  
}

bool
pd_client::init(const std::string& pdaddr) {
  // create a temp api stub to fill member conn pool
  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(strip_url(pdaddr),
                                                                grpc::InsecureChannelCredentials());
  std::unique_ptr<pdpb::PD::Stub> stub = pdpb::PD::NewStub(channel);
  return update_leader(stub.get());
}

pdpb::PD::Stub* pd_client::get_leader_stub(bool force) {
  if (leader_stub_ == nullptr || force) {
    leader_stub_.reset();
    LOG("update pd leader client stub");
    do {
      // pick a pd peer from member pool
      auto member = select_randomly(members_.begin(), members_.end());
      auto addr = select_randomly(member->second.client_urls.begin(),member->second.client_urls.end());

      std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(strip_url(*addr), 
                                                                    grpc::InsecureChannelCredentials());
      std::unique_ptr<pdpb::PD::Stub> stub = pdpb::PD::NewStub(channel);
      update_leader(stub.get());

      if (leader_stub_ == nullptr) {
        LOG("get pd leader failed...sleep 1000ms then retry...");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    } while(leader_stub_ == nullptr);
  }
  return leader_stub_.get();
}

bool 
pd_client::update_leader(pdpb::PD::Stub* stub) {
  // update pd addrs
  uint64_t cluster_id;
  Result<std::pair<std::map<uint64_t, pd_server_info>, pd_server_info>, Error> r = get_pd_members_inner(stub, &cluster_id);
  if (r.isOk()) {
    boost::unique_lock<boost::shared_mutex> lock(rwlock_);
    members_ = r.unwrap().first;
    leader_ = r.unwrap().second;
    cluster_id_ = cluster_id;

    // check if need update leader
    for (auto it = members_.begin(); it != members_.end(); it++) {
      if (it->second.is_leader && (it->second.id != leader_.id || leader_stub_ == nullptr)) {
        leader_ = it->second;
        LOG("update leader rpc stub");
        std::string addr = *select_randomly(leader_.client_urls.begin(), leader_.client_urls.end());
        leader_channel_ = grpc::CreateChannel(strip_url(addr), grpc::InsecureChannelCredentials());
        leader_stub_ = pdpb::PD::NewStub(leader_channel_);
        return true;
      }
    }
    LOG("can't find leader");
    return false;
  } else {
    LOG("update leader error:" << r.unwrapErr().error << std::endl);
    return false;
  }
}

Result<std::pair<std::map<uint64_t, pd_server_info>, pd_server_info>, Error>
pd_client::get_pd_members_inner(pdpb::PD::Stub* stub, uint64_t* out_cluster_id) {
  pdpb::GetMembersRequest req;
  pdpb::GetMembersResponse resp;
  grpc::ClientContext ctx;
  grpc::Status st = stub->GetMembers(&ctx, req, &resp);
  std::map<uint64_t, pd_server_info> ret;
  if (st.ok()) {
    for (auto it = resp.members().begin(); it != resp.members().end(); it++) {
      pd_server_info info;
      info.name = it->name();
      info.id = it->member_id();
      info.is_leader = false;
      std::copy(it->client_urls().begin(), it->client_urls().end(), 
                std::back_inserter(info.client_urls));
      
      ret[info.id] = info;
    }
    pd_server_info leader;
    leader.name = resp.leader().name();
    leader.id = resp.leader().member_id();
    leader.is_leader = true;
    std::copy(resp.leader().client_urls().begin(), resp.leader().client_urls().end(), 
                std::back_inserter(leader.client_urls));
    ret[leader.id] = leader;

    if (out_cluster_id != nullptr) 
      *out_cluster_id = resp.header().cluster_id();
    return Ok(std::make_pair(ret, leader));
  } else {
    return Err(Error(st.error_message()));
  }
}

void convert_from_pb(const metapb::Store* store, store_info* ret) {
  ret->id = store->id();
  ret->addr = store->address();
  switch (store->state()) {
    case metapb::StoreState::Up: 
      ret->state = tikv::store_state::UP;
      break;
    case metapb::StoreState::Offline:
      ret->state = tikv::store_state::OFFLINE;
      break;
    case metapb::StoreState::Tombstone:
      ret->state = tikv::store_state::TOMBSTONE;
      break;
    default:
      LOG("store state error, no such state" << store->state());
      assert(0);
  }
  for (auto it = store->labels().begin(); it != store->labels().end(); it++) {
    ret->labels[it->key()] = it->value();
  }
}

Result<store_info, Error>
pd_client::get_store_by_id_inner(pdpb::PD::Stub* stub,uint64_t store_id) {
  assert(cluster_id_ > 0);
  pdpb::GetStoreRequest req;
  pdpb::GetStoreResponse resp;

  req.mutable_header()->set_cluster_id(cluster_id_);
  req.set_store_id(store_id);
  grpc::ClientContext ctx;
  grpc::Status st = stub->GetStore(&ctx, req, &resp);
  if (st.ok()) {
    store_info ret;
    metapb::Store s = resp.store();
    convert_from_pb(&s, &ret);
    return Ok(ret);
  } else {
    return Err(Error(st.error_message()));
  }
}
Result<std::vector<store_info>, Error>
pd_client::get_all_stores_inner(pdpb::PD::Stub* stub) {
  assert(cluster_id_ > 0);
  pdpb::GetAllStoresRequest req;
  pdpb::GetAllStoresResponse resp;

  req.mutable_header()->set_cluster_id(cluster_id_);
  grpc::ClientContext ctx;
  grpc::Status st = stub->GetAllStores(&ctx, req, &resp);

  if (st.ok()) {
    std::vector<store_info> ret;
    for (auto it = resp.stores().begin(); it != resp.stores().end(); it++) {
      store_info info;
      convert_from_pb(&(*it), &info);
      ret.push_back(info);
    }
    return Ok(ret);
  } else {
    return Err(Error(st.error_message()));
  }
}

Result<std::pair<region_info, peer_info>, Error>
pd_client::get_region_inner(pdpb::PD::Stub* stub, const std::string& key) {
  // make sure client has already been initialized.
  assert(cluster_id_ > 0);
  pdpb::GetRegionRequest req;
  pdpb::GetRegionResponse resp;

  req.mutable_header()->set_cluster_id(cluster_id_);
  req.set_region_key(key);

  grpc::ClientContext ctx;
  grpc::Status st = stub->GetRegion(&ctx, req, &resp);

  if (st.ok()) {
    region_info ret;
    peer_info leader;
    ret.ver_id.id = resp.region().id();
    ret.ver_id.conf_ver = resp.region().region_epoch().conf_ver();
    ret.ver_id.ver = resp.region().region_epoch().version();
    ret.start_key = resp.region().start_key();
    ret.end_key = resp.region().end_key();
    for (auto it = resp.region().peers().begin(); it != resp.region().peers().end(); it++) {
      peer_info peer;
      peer.id = it->id();
      peer.store_id = it->store_id();
      ret.peers.push_back(peer);
    }
    // ret->leader_store_id = resp.leader().id();
    leader.store_id = resp.leader().store_id();
    leader.id = resp.leader().id();
    return Ok(std::make_pair(ret, leader));
  } else {
    return Err(Error(st.error_message()));
  }
}

} // namespace tikv
