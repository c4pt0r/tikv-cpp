#include <atomic>
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

*/

boost::optional<region_info> region_cache::get_cached_region(region_version_id verid) {
  boost::shared_lock<boost::shared_mutex> l(region_lock_);
  auto it = cached_regions_.find(verid);
  if (it != cached_regions_.end()) {
    // update access time
    uint64_t now = ::time(NULL);
    std::atomic_compare_exchange_strong(&(it->second.last_access), &now, now);
    return it->second.region;
  }
  return boost::none;
}

boost::optional<region_info> region_cache::search_cache(const std::string& key) {
  boost::shared_lock<boost::shared_mutex> l(region_lock_);
  auto it = sorted_.upper_bound(key);
  // TODO check region cache's TTL
  if (it != sorted_.end() && it->second.contains(key)) {
    return get_cached_region(it->second.ver_id);
  }
  return boost::none;
}

Result<region_cache::cached_region, Error> region_cache::load_region_from_pd(const std::string& key) {
  Result<std::pair<region_info, peer_info>, Error> ret = pd_client_->get_region(key);
  if (!ret.isOk()) {
    return Err(ret.unwrapErr());
  } 
  const region_info &r = ret.unwrap().first;
  const peer_info &leader = ret.unwrap().second;

  if (r.peers.size() == 0) {
    // this region has not peer
    return Err(Error("this region has no peers"));
  }

  cached_region cr(r, r.peers[0]);
  if (leader.id > 0 && leader.store_id > 0) {
    cr.switch_leader(leader.store_id);
  }
  return Ok(cr);
}

Result<region_cache::cached_region, Error> region_cache::load_region_from_pd_by_id(uint64_t region_id) {
  Result<std::pair<region_info, peer_info>, Error> ret = pd_client_->get_region_by_id(region_id);
  if (!ret.isOk()) {
    return Err(ret.unwrapErr());
  } 
  const region_info &r = ret.unwrap().first;
  const peer_info &leader = ret.unwrap().second;

  if (r.peers.size() == 0) {
    // this region has not peer
    return Err(Error("this region has no peers"));
  }

  cached_region cr(r, r.peers[0]);
  if (leader.id > 0 && leader.store_id > 0) {
    cr.switch_leader(leader.store_id);
  }
  return Ok(cr);
}


} // name tikv