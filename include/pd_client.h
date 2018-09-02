#pragma once
#include <string>
#include <unistd.h>
#include <boost/thread/shared_mutex.hpp>

#include "kvproto/pdpb.grpc.pb.h"
#include "result/result.h"
#include "meta.h"

namespace tikv {

class pd_client {
 public:
  pd_client(const std::string& addr);

  bool init(const std::string& pdaddr);

  void close();

  // return val pair=>{{id=>pd_server}, leader_pd}
  Result<std::pair<std::map<uint64_t, pd_server_info>, pd_server_info>, Error> get_pd_members();
  // GetRegion gets a region and its leader Peer from PD by key.
  // The region may expire after split. Caller is responsible for caching and
  // taking care of region change.
  // Also it may return nil if PD finds no Region for the key temporarily,
  // client should retry later.
  Result<std::pair<region_info, peer_info>, Error> get_region(const std::string& key); 
  // GetRegionByID gets a region and its leader Peer from PD by id.
  Result<std::pair<region_info, peer_info>, Error> get_region_by_id(uint64_t region_id);
  // GetStore gets a store from PD by store id.
	// The store may expire later. Caller is responsible for caching and taking care
	// of store change.
  Result<store_info, Error> get_store_by_id(uint64_t store_id);
  // GetAllStores gets all stores from pd.
	// The store may expire later. Caller is responsible for caching and taking care
	// of store change.
  Result<std::vector<store_info>, Error> get_all_stores();
  uint64_t get_cluster_id() { return cluster_id_; }

private:
  Result<std::pair<std::map<uint64_t, pd_server_info>, pd_server_info>, Error> get_pd_members_inner(pdpb::PD::Stub* stub, uint64_t* out_cluster_id);
  Result<std::pair<region_info, peer_info>, Error> get_region_inner(pdpb::PD::Stub* stub, const std::string& key); 
  Result<std::pair<region_info, peer_info>, Error> get_region_by_id_inner(pdpb::PD::Stub* stub, uint64_t region_id);
  Result<store_info, Error> get_store_by_id_inner(pdpb::PD::Stub* stub, uint64_t store_id); 
  Result<std::vector<store_info>, Error> get_all_stores_inner(pdpb::PD::Stub* stub);

  bool update_leader(pdpb::PD::Stub* stub);
  pdpb::PD::Stub* get_leader_stub(bool force=false);

private:
  // leader conn
  std::map<uint64_t, pd_server_info> members_;
  pd_server_info leader_;

  // buffered rpc channels and stubs
  boost::shared_mutex rwlock_;
  std::map<std::string, std::shared_ptr<grpc::Channel>> rpc_channels_;
  std::map<uint64_t, std::unique_ptr<pdpb::PD::Stub>> rpc_stubs_;
  // buffered leader stub
  std::shared_ptr<grpc::Channel> leader_channel_;
  std::unique_ptr<pdpb::PD::Stub> leader_stub_;
  
  uint64_t cluster_id_;
  std::string err_;

};

};
