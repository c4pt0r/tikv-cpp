#pragma once
#include <string>
#include <unistd.h>
#include <map>
#include <vector>

namespace tikv {

struct pd_server_info {
  uint64_t id;
  std::string name;
  std::vector<std::string> client_urls;
  bool is_leader;
};

struct region_info {
  uint64_t id;
  std::string start_key;
  std::string end_key;
  uint64_t leader_store_id;
};

enum store_state {
    UP,
    OFFLINE,
    TOMBSTONE,
};

struct store_info {
  uint64_t id;
  std::string addr;
  store_state state;
  std::map<std::string, std::string> labels;
};

};
