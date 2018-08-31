#pragma once
#include <boost/thread/shared_mutex.hpp>
#include "3rd_party/cpp-btree/btree_map.h"
#include "meta.h"

namespace tikv {

class region_cache {
public:
  
private:
  boost::shared_mutex region_lock_;
  // end_key => region
  btree::btree_map<std::string, region_info> sorted_;
  // region_id => region
  std::map<uint64_t, region_info> regions_;

  boost::shared_mutex store_lock_;
  // store id => store
  std::map<uint64_t, store_info> stores_;
};


};
