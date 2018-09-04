#include <atomic>
#include <iostream>
#include "region_cache.h"


namespace tikv {

Result<region_cache::key_loc, Error>
region_cache::locate_key(const std::string& key) {
  region_lock_.lock_shared();
  auto cached = search_cache(key);
  if (cached != boost::none) {
    LOG("got region info in region cache!");
    key_loc ret;
    ret.start_key = cached->start_key;
    ret.end_key = cached->end_key;
    ret.region_ver_id = cached->ver_id;
    region_lock_.unlock_shared();
    return Ok(ret);
  }
  region_lock_.unlock_shared();

  auto r = load_region_from_pd(key);
  if (r.isOk()) {
    LOG("load region from pd");
    region_info region = r.unwrap();
    insert_region_to_cache(region);
    key_loc ret;
    ret.start_key = region.start_key;
    ret.end_key = region.end_key;
    ret.region_ver_id = region.ver_id;
    return Ok(ret);
  } else {
    return Err(r.unwrapErr());
  }
}

void
region_cache::dump_cache() {
  for (auto it = sorted_.begin(); it != sorted_.end(); it++) {
    cached_region cr = cached_regions_[it->second.ver_id];
    LOG("[" << to_hex(cr.region.start_key) << to_hex(cr.region.end_key) << ") => Leader store:" 
        << cr.region.leader.store_id << std::endl);
  }
}

void
region_cache::insert_region_to_cache(const region_info& r) {
  boost::unique_lock<boost::shared_mutex> l(region_lock_);
  // update btree
  auto it = sorted_.find(r.end_key);
  if (it != sorted_.end()) {
    // remove old item 
    cached_regions_.erase(it->second.ver_id);
  }
  LOG("update region cache");
  sorted_[r.end_key] = r;
  // update cached region info 
  cached_region cr;
  cr.region = r;
  cr.last_access = ::time(NULL);
  cached_regions_[r.ver_id] = cr;
}

boost::optional<region_info>
region_cache::get_cached_region(region_version_id verid) {
  boost::shared_lock<boost::shared_mutex> l(region_lock_);
  auto it = cached_regions_.find(verid);
  if (it != cached_regions_.end() && it->second.is_valid()) {
    // update access time
    uint64_t now = ::time(NULL);
    std::atomic_compare_exchange_strong(&(it->second.last_access), &now, now);
    return it->second.region;
  }
  return boost::none;
}

boost::optional<region_info>
region_cache::search_cache(const std::string& key) {
  boost::shared_lock<boost::shared_mutex> l(region_lock_);
  auto it = sorted_.upper_bound(key);
  // TODO check region cache's TTL
  if (it != sorted_.end() && it->second.contains(key)) {
    return get_cached_region(it->second.ver_id);
  }
  return boost::none;
}

Result<region_info, Error>
region_cache::load_region_from_pd(const std::string& key) {
  Result<std::pair<region_info, peer_info>, Error> ret = pd_client_->get_region(key);
  if (!ret.isOk()) {
    return Err(ret.unwrapErr());
  } 
  region_info r = ret.unwrap().first;
  const peer_info &leader = ret.unwrap().second;

  if (r.peers.size() == 0) {
    // this region has not peer
    return Err(Error("this region has no peers"));
  }

  if (leader.id > 0 && leader.store_id > 0) {
    r.switch_leader(leader.store_id);
  }
  return Ok(r);
}

Result<region_info, Error>
region_cache::load_region_from_pd_by_id(uint64_t region_id) {
  Result<std::pair<region_info, peer_info>, Error> ret = pd_client_->get_region_by_id(region_id);
  if (!ret.isOk()) {
    return Err(ret.unwrapErr());
  } 
  region_info r = ret.unwrap().first;
  const peer_info &leader = ret.unwrap().second;

  if (r.peers.size() == 0) {
    // this region has not peer
    return Err(Error("this region has no peers"));
  }

  if (leader.id > 0 && leader.store_id > 0) {
    r.switch_leader(leader.store_id);
  }
  return Ok(r);
}


} // name tikv
