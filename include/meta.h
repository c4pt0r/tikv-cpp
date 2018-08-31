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

};
