#pragma once
#include <boost/thread/shared_mutex.hpp>
#include "3rd_party/cpp-btree/btree_map.h"
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
  class cached_region {
   public:
    void switch_leader(uint64_t store_id) {
      for (auto it= region_info_.peers.begin(); it!= region_info_.peers.end();it++) {
          if (it->store_id == store_id) {
              LOG("leader of region " << region_info_.ver_id.id << " switch to" << store_id);
              leader_ = *it;
          }
      }
    }
   private:
    region_info region_info_;
    peer_info leader_;
    uint64_t last_access_;
  };

 public:
  region_cache(std::shared_ptr<pd_client> pd_client):
      pd_client_(pd_client) {}

  key_loc locate_key(const std::string& key);

 private:
  cached_region search_cache(const std::string& key);
  cached_region get_cached_region(region_version_id verid);  
  cached_region load_region_from_pd(const std::string& key);  

 private:
  boost::shared_mutex region_lock_;
  // end_key => region
  btree::btree_map<std::string, region_info> sorted_;
  // region_id => region
  std::map<uint64_t, cached_region> regions_;

  boost::shared_mutex store_lock_;
  // store id => store
  std::map<uint64_t, store_info> stores_;

  // pd client, for quering region info and store info
  std::shared_ptr<pd_client> pd_client_;

}; // end of region_cache

}; // end of tikv
