#pragma once
#include <string>
#include <unistd.h>
#include <map>
#include <vector>

namespace tikv {

struct pd_server_info {
  uint64_t id = 0;
  uint64_t cluster_id = 0;
  std::string name;
  std::vector<std::string> client_urls;
  bool is_leader = false;
};

struct region_version_id {
  uint64_t id = 0;
  uint64_t conf_ver = 0;
  uint64_t ver = 0;
  bool operator< (const region_version_id& b) const {
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

struct peer_info {
  uint64_t id = 0;
  uint64_t store_id = 0;
};

struct region_info {
  region_version_id ver_id;
  std::string start_key;
  std::string end_key;
  std::vector<peer_info> peers;
  peer_info leader;

  bool contains(const std::string& key) {
    return start_key.compare(key) <= 0 
              && (end_key.compare(key) > 0 || end_key.size() == 0);
  }

  void switch_leader(uint64_t store_id) {
    for (auto it= peers.begin(); it!= peers.end();it++) {
      if (it->store_id == store_id) {
        leader = *it;
      }
    }
  }

};

enum store_state {
    UP,
    OFFLINE,
    TOMBSTONE,
};

struct store_info {
  uint64_t id = 0;
  std::string addr;
  store_state state;
  std::map<std::string, std::string> labels;
};

struct Error {
  std::string error;
  Error(const std::string& errmsg): error(errmsg) {}
};

};
