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
  inline Peer* leader() const { return leader_; }

  // copy from other region info
  Region(const Region& x) {
    *this = x;
  }

  Region& operator=(const Region& x) {
    ver_id = x.ver_id;
    start_key = x.start_key;
    end_key = x.end_key;
    peers = x.peers;
    switch_leader(x.leader()->store_id);
    return *this;
  }

  Region() {}
  ~Region() {}

  inline bool contains(const std::string& key) {
    return start_key.compare(key) <= 0 
              && (end_key.compare(key) > 0 || end_key.size() == 0);
  }

  inline void switch_leader(uint64_t store_id) {
    for (auto it= peers.begin(); it!= peers.end();it++) {
      if (it->store_id == store_id) {
        leader_ = &*it;
      }
    }
  }

  inline void switch_leader_by_peer_id(uint64_t peer_id) {
    for (auto it= peers.begin(); it!= peers.end();it++) {
      if (it->id == peer_id) {
        leader_ = &*it;
      }
    }
  }

 private:
  // only used by mocktikv
  void incr_ver() { ver_id.ver++; }
  void incr_conf_ver() { ver_id.conf_ver++; }

  void update_key_range(const std::string& start, 
                        const std::string& end) {
    start_key = start;
    end_key = end;
    incr_ver();
  }

  
  Region split(std::vector<uint64_t> peer_ids, 
                    uint64_t leader_id, 
                    const std::string& split_key);




 private:
  // set by switch leader
  Peer* leader_ = nullptr;
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
};

struct Error {
  std::string error;
  Error(const std::string& msg): error(msg) {}
};

}; // tikv
