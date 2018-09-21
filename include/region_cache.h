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

class RegionCache {

 public:
  struct key_loc {
    std::string start_key;
    std::string end_key;
    RegionVerID region_ver_id;

    bool contains(const std::string& key) {
      return start_key.compare(key) <= 0 
                && (end_key.compare(key) > 0 || end_key.size() == 0);
    }
  };

  struct rpc_context {
    RegionVerID region_ver_id;
    Region meta;
    Peer peer;
    std::string addr;
  };

  // region object stores region info and its leader peer
  struct cached_region {
    Region region;
    Peer leader;
    std::atomic<uint64_t> last_access;

    cached_region(): last_access(0) {};

    cached_region(const cached_region& b):
      region(b.region), last_access(b.last_access.load()) {
      leader = *region.leader();
    }

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
  RegionCache(std::shared_ptr<PDClient> PDClient):
      PDClient_(PDClient) {}

  Result<key_loc,Error> locate_key(const std::string& key, bool& cache_hit);
  Result<rpc_context, Error> get_rpc_context(RegionVerID ver_id);
  Result<std::string, Error> get_store_addr(uint64_t store_id);
  Result<std::string, Error> reload_store_addr(uint64_t store_id);

  void update_region_leader(RegionVerID ver_id, uint16_t leader_store_id);
  void drop_region(RegionVerID region_id);

  void dump_cache();

 private:
  void insert_region_to_cache(const Region& r);
  void drop_region_from_cache(RegionVerID region_id);
  boost::optional<Region&> search_cache(const std::string& key);
  boost::optional<Region&> get_cached_region(RegionVerID ver_id);  

  // get result throught RPC
  Result<Region, Error> load_region_from_pd(const std::string& key);  
  Result<Region, Error> load_region_from_pd_by_id(uint64_t region_id);  
  Result<Store, Error> load_store_from_pd(uint64_t store_id);  

 private:
  // region cache ttl 600s
  static const int kDefaultRegionCacheTTL = 10 * 60;

 private:
  boost::shared_mutex region_lock_;
  // end_key => region
  btree::btree_map<std::string, Region> sorted_;
  // region_id => region
  std::map<RegionVerID, cached_region> cached_regions_;

  boost::shared_mutex store_lock_;
  // store id => store
  std::map<uint64_t, Store> stores_;
  // pd client, for quering region info and store info
  std::shared_ptr<PDClient> PDClient_;

}; // end of RegionCache

}; // end of tikv
