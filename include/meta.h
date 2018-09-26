#pragma once
#include <string>
#include <unistd.h>
#include <map>
#include <vector>

namespace tikv {

struct PDServerInfo {
  uint64_t id         = 0;
  uint64_t cluster_id = 0;
  bool is_leader      = false;
  std::string name;
  std::vector<std::string> client_urls;
};

struct RegionVerID {
  uint64_t id       = 0;
  uint64_t conf_ver = 0;
  uint64_t ver      = 0;

  bool operator< (const RegionVerID& b) const {
    if (id == b.id) {
      if (ver == b.ver) {
        return conf_ver < b.conf_ver;
      } else {
        return ver < b.ver;
      }
    } else {
      return id < b.id;
    }
  }
};

struct Peer {
  uint64_t id       = 0; // peer id
  uint64_t store_id = 0; // store id
};

struct Region {
  RegionVerID ver_id;
  std::string start_key;
  std::string end_key;
  std::vector<Peer> peers;
  Peer leader;

  Region() {}
  ~Region() {}

  Region(uint64_t region_id,
         const std::vector<uint64_t>& store_ids,
         const std::vector<uint64_t>& peer_ids,
         uint64_t leader_peer_id) {
    assert(store_ids.size() == peer_ids.size());

    std::vector<Peer> peers;
    for (int i = 0;i < peer_ids.size(); i++) {
      Peer p;
      p.id = peer_ids[i];
      p.store_id =  store_ids[i];
      peers.push_back(p);
    }
    this->ver_id.id = region_id;
    this->peers = peers;
    this->switch_leader_by_peer_id(leader_peer_id);
  }

  inline bool contains(const std::string& key) {
    return start_key.compare(key) <= 0 
              && (end_key.compare(key) > 0 || end_key.size() == 0);
  }

  inline void switch_leader(uint64_t store_id) {
    for (auto it= peers.begin(); it!= peers.end();it++) {
      if (it->store_id == store_id) {
        leader = *it;
      }
    }
  }

  inline void switch_leader_by_peer_id(uint64_t peer_id) {
    for (auto it= peers.begin(); it!= peers.end();it++) {
      if (it->id == peer_id) {
        leader = *it;
      }
    }
  }

 private:
  friend class Cluster;
  // only used by mocktikv
  void incr_ver() { ver_id.ver++; }
  void incr_conf_ver() { ver_id.conf_ver++; }

  void update_key_range(const std::string& start, 
                        const std::string& end) {
    start_key = start;
    end_key = end;
    incr_ver();
  }

  Region split(uint64_t new_region_id, 
               const std::vector<uint64_t>& peer_ids,
               uint64_t leader_id, 
               const std::string& split_key);

};

enum StoreState {
    UP,
    OFFLINE,
    TOMBSTONE,
};

struct Store {
  uint64_t id       = 0;
  StoreState state  = OFFLINE;

  std::string addr;
  std::map<std::string, std::string> labels;

  Store(uint64_t store_id, const std::string& addr):
    id(store_id), addr(addr) {}

  Store() {}
  ~Store() {}

  void set_state(StoreState s) { state = s; }
};

struct Error {
  std::string error;
  Error(const std::string& msg): error(msg) {}
};

}; // tikv
