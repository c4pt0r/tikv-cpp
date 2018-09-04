#pragma once
#include <boost/thread/shared_mutex.hpp>
#include <boost/optional.hpp>

#include "3rd_party/cpp-btree/btree_map.h"
#include "3rd_party/result/result.h"
#include "logging.h"
#include "meta.h"
#include "utils.h"
#include "pd_client.h"

namespace tikv {

class region_cache {

 public:
  struct key_loc {
    std::string start_key;
    std::string end_key;
    region_version_id region_ver_id;

    bool contains(const std::string& key) {
      return start_key.compare(key) <= 0 
                && (end_key.compare(key) > 0 || end_key.size() == 0);
    }
  };

  struct rpc_context {
    region_version_id region_ver_id;
    region_info meta;
    peer_info peer;
    std::string addr;
  };

  // region object stores region info and its leader peer
  struct cached_region {
    region_info region;
    peer_info leader;
    std::atomic<uint64_t> last_access;

    cached_region(): last_access(0) {};

    cached_region(const cached_region& b):
      region(b.region), leader(b.leader), last_access(b.last_access.load()) {}

    cached_region& operator=(const cached_region &b) {
      region = b.region;
      leader = b.leader;
      last_access = b.last_access.load();
      return *this; 
    }

    bool is_valid() {
      uint64_t dur = ::time(NULL) - last_access;
      return dur < kDefaultRegionCacheTTL;
    }

  };

 public:
  region_cache(std::shared_ptr<pd_client> pd_client):
      pd_client_(pd_client) {}

  Result<key_loc,Error> locate_key(const std::string& key);

  void dump_cache();

 private:
  void insert_region_to_cache(const region_info& r);
  boost::optional<region_info> search_cache(const std::string& key);
  boost::optional<region_info> get_cached_region(region_version_id verid);  
  Result<region_info, Error> load_region_from_pd(const std::string& key);  
  Result<region_info, Error> load_region_from_pd_by_id(uint64_t region_id);  

 private:
  // region cache ttl 600s
  static const int kDefaultRegionCacheTTL = 10 * 60;

 private:
  boost::shared_mutex region_lock_;
  // end_key => region
  btree::btree_map<std::string, region_info> sorted_;
  // region_id => region
  std::map<region_version_id, cached_region> cached_regions_;

  boost::shared_mutex store_lock_;
  // store id => store
  std::map<uint64_t, store_info> stores_;
  // pd client, for quering region info and store info
  std::shared_ptr<pd_client> pd_client_;

}; // end of region_cache

}; // end of tikv
