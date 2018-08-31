#pragma once
#include <functional>
#include <string>
#include <vector>
#include "meta.h"

namespace tikv {

const std::string kDefaultCf = "df";

class client {
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

private:
  // get_store_stub_by_key(const std::string& key), 
  // 1. find region by key  
  // 2. get store info 
  // 3. get conn through store addr
};

};
