#pragma once
#include <functional>
#include <string>
#include <vector>

namespace tikv {

const std::string kDefaultCf = "df";

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

class client {

 public:

  typedef std::function<void(resp&)> resp_callback_t;

 public:

  resp get(const std::string& key, const std::string& cf);
  resp put(const std::string& key, const std::string& val, const std::string& cf);

  resp batch_get(const std::vector<std::string>& keys, const std::string& cf);
  resp batch_put(const std::vector<std::string>& keys,
                   const std::vector<std::string>& vals,
                   const std::string& cf);

  resp scan(const std::string& key_prefix,
              std::size_t limit,
              bool key_only = false,
              const std::string& cf = kDefaultCf);

  resp range_scan(const std::string& start_key, const std::string& end_key,
                    std::size_t limit,
                    bool key_only,
                    const std::string& cf);
              
};

};
