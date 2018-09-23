#include "mocktikv.h"

namespace tikv {


Region Region::split(std::vector<uint64_t> peer_ids,
                               uint64_t leader_id, 
                               const std::string& split_key) {

  assert(peer_ids.size() == peers.size());
  Region r;
  std::vector<Peer> peers;
  std::vector<uint64_t> store_ids;
  
  for (auto it = peers.begin(); it != peers.end(); it++) {
    store_ids.push_back(it->store_id);
  }

  for (int i = 0;i < peer_ids.size(); i++) {
    Peer p;
    p.id = peer_ids[i];
    p.store_id =  store_ids[i];

    peers.push_back(p);
  }

  r.peers = peers;
  r.switch_leader_by_peer_id(leader_id);
  r.update_key_range(split_key, end_key);
  update_key_range(start_key, split_key);
  return r;

}

uint64_t Cluster::allocate_id_inner() {
  return ++id_;
}


};
