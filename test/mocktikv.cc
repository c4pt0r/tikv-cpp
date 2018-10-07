#include "mocktikv.h"

namespace tikv {

// Split region and generate new region
Region Region::split(uint64_t new_region_id, 
                     const std::vector<uint64_t>& peer_ids,
                     uint64_t leader_id, 
                     const std::string& split_key) {

  assert(peer_ids.size() == peers.size());
  Region r;
  std::vector<uint64_t> store_ids;
  
  for (auto it = peers.begin(); it != peers.end(); it++) {
    store_ids.push_back(it->store_id);
  }

  std::vector<Peer> new_peers;
  for (int i = 0;i < peer_ids.size(); i++) {
    Peer p;
    p.id = peer_ids[i];
    p.store_id =  store_ids[i];

    new_peers.push_back(p);
  }

  r.ver_id.id = new_region_id;
  r.peers = new_peers;
  r.switch_leader_by_peer_id(leader_id);
  r.update_key_range(split_key, end_key);
  update_key_range(start_key, split_key);
  return r;

}

void Cluster::add_store(uint64_t store_id, const std::string& addr) {
  // TODO lock
  stores_[store_id] = Store(store_id, addr);
}

boost::optional<Store> Cluster::get_store(uint64_t store_id) {
  // TODO lock
  auto it = stores_.find(store_id);
  if (it == stores_.end()) {
    return boost::none;
  }
  return it->second;
}

void Cluster::remove_store(uint64_t store_id) {
  // TODO lock
  auto it = stores_.find(store_id);
  stores_.erase(store_id);
}

void Cluster::split(uint64_t region_id, uint64_t new_region_id, 
                        const std::string& key, 
                        const std::vector<uint64_t>& peer_ids, 
                        uint64_t leader_id) {
  // TODO lock
  Region new_region = regions_[region_id].split(new_region_id, peer_ids, leader_id, key);
  regions_[new_region_id] = new_region;
}

void Cluster::dump() {
  Json::Value root;
  Json::Value region_arr_object;

  for (auto it = regions_.begin(); it != regions_.end(); it++) {
    Json::Value region_obj;
    region_obj["id"] = it->first;
    region_obj["start"] = it->second.start_key;
    region_obj["end"] = it->second.end_key;
    Json::Value peers;
    for (auto itt = it->second.peers.begin(); itt != it->second.peers.end(); itt++) {
      Json::Value p;
      p["id"] = itt->id;
      p["store_id"] = itt->store_id;
      if (itt->id == it->second.leader.id) {
        p["is_leader"] = true;
      }
      peers.append(p);
    }
    region_obj["peers"] = peers;

    region_arr_object.append(region_obj);
  }
  root["regions"] = region_arr_object;
  LOG(root.toStyledString());
}

void Cluster::bootstrap(uint64_t region_id, 
                       const std::vector<uint64_t>& store_ids, 
                       const std::vector<uint64_t>& peer_ids,
                       u_int64_t leader_peer_id) {
  // create first region
  Region r(region_id, store_ids, peer_ids, leader_peer_id);
  regions_[region_id] = r;
}

uint64_t Cluster::allocate_id_inner() {
  return ++id_;
}

};
