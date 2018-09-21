#pragma once
#include <string>
#include <unistd.h>
#include <map>
#include <vector>

namespace tikv {

struct PDServerInfo {
  uint64_t id         = 0;
  uint64_t cluster_id = 0;
  std::string name;
  std::vector<std::string> client_urls;
  bool is_leader = false;
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
  Peer* leader = nullptr;

  bool contains(const std::string& key) {
    return start_key.compare(key) <= 0 
              && (end_key.compare(key) > 0 || end_key.size() == 0);
  }

  void switch_leader(uint64_t store_id) {
    for (auto it= peers.begin(); it!= peers.end();it++) {
      if (it->store_id == store_id) {
        leader = &*it;
      }
    }
  }

 private:
  // only used by mocktikv
  Region split(std::vector<uint64_t> peer_ids, 
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
  StoreState state = OFFLINE;

  std::string addr;
  std::map<std::string, std::string> labels;
};

struct Error {
  std::string error;
  Error(const std::string& errmsg): error(errmsg) {}
};

}; // tikv
