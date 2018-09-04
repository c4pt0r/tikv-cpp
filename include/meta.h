#pragma once
#include <string>
#include <unistd.h>
#include <map>
#include <vector>

namespace tikv {

struct pd_server_info {
  uint64_t id;
  uint64_t cluster_id;
  std::string name;
  std::vector<std::string> client_urls;
  bool is_leader;
};

struct region_version_id {
  uint64_t id;
  uint64_t conf_ver;
  uint64_t ver;
  bool operator< (const region_version_id& b) const {
    if (ver < b.ver) {
      return true;
    } else if (ver == b.ver) {
      return conf_ver < b.conf_ver;
    }
    return false;
  }
};

struct peer_info {
  uint64_t id;
  uint64_t store_id;
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
  uint64_t id;
  std::string addr;
  store_state state;
  std::map<std::string, std::string> labels;
};

struct Error {
  std::string error;
  Error(const std::string& errmsg): error(errmsg) {}
};

/*
class resp {
 public:
  resp(void):
    is_err_(false) {}

  resp(const std::string& key, const std::string& val):
    is_err_(false), key_(key), val_(val) {}

  resp(const std::vector<tikv::resp>& resps):
    is_err_(false), batchval_(resps) {}

 public:
  ~resp(void) = default;
  resp(const resp&) = default;
  resp& operator=(const resp&) = default;

 public:
  void set_error_msg(const std::string& err) { is_err_= true; errmsg_ = err; }
  void set_key(const std::string& key) { key_ = key; }
  void set_value(const std::string& val) { val_ = val; }
  resp& operator<<(const resp& r); 

 public:
  bool ok() const { return is_err_ == false; }
  const std::string& error_msg() const { return errmsg_; }

  const std::vector<resp>& resps() const;
  const std::string& key() const;
  const std::string& value()const;

 private:
  bool 			    is_err_;
  std::string 	errmsg_;
  std::string   key_;
  std::string 	val_;
  std::vector<tikv::resp> batchval_;
};

static resp respok = resp();
*/

};
