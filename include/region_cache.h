#pragma once
#include <boost/thread/shared_mutex.hpp>
#include <boost/optional.hpp>

#include "3rd_party/cpp-btree/btree_map.h"
#include "3rd_party/result/result.h"
#include "logging.h"
#include "meta.h"
#include "pd_client.h"

namespace tikv {

class region_cache {

 public:
  class key_loc {
   public:
    bool contains(const std::string& key) {
      return start_key_.compare(key) <= 0 
                && (end_key_.compare(key) > 0 || end_key_.size() == 0);
    }
   private:
    std::string start_key_;
    std::string end_key_;
    region_version_id region_ver_id;
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

    cached_region(const region_info& ri, const peer_info& pi):
      region(ri), leader(pi), last_access(0) {}

    cached_region(const cached_region& b):
      region(b.region), leader(b.leader), last_access(b.last_access.load()) {}
    
    void switch_leader(uint64_t store_id) {
      for (auto it= region.peers.begin(); it!= region.peers.end();it++) {
          if (it->store_id == store_id) {
            LOG("leader of region " << region.ver_id.id << " switch to" << store_id);
            leader = *it;
          }
      }
    }
  };

 public:
  region_cache(std::shared_ptr<pd_client> pd_client):
      pd_client_(pd_client) {}

  key_loc locate_key(const std::string& key);

 private:
  boost::optional<region_info> search_cache(const std::string& key);
  boost::optional<region_info> get_cached_region(region_version_id verid);  
  Result<cached_region, Error> load_region_from_pd(const std::string& key);  
  Result<cached_region, Error> load_region_from_pd_by_id(uint64_t region_id);  

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
