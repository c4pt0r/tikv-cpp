#include "region_cache.h"


namespace tikv {

/*
region_cache::key_loc region_cache::locate_key(const std::string& key) {
  // load loc from cache
  region_lock_.lock_shared();
  region_lock_.unlock_shared();

  // if cache miss, request pd
  region_info r = load_region(key) 

}

region_cache::cached_region region_cache::search_cache(const std::string& key) {
  
}

region_cache::cached_region region_cache::get_cached_region(region_version_id verid) {
}

region_cache::cached_region egion_cache::load_region_from_pd(const std::string& key) {
  region_info r;
  peer_info leader;
  resp ret = pd_client_->get_region(key, &r, &leader);
  if (ret.ok()) {
    
  }

}
*/

} // name tikv