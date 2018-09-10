#include <atomic>
#include <iostream>
#include "region_cache.h"


namespace tikv {

void
region_cache::dump_cache() {
  for (auto it = sorted_.begin(); it != sorted_.end(); it++) {
    cached_region cr = cached_regions_[it->second.ver_id];
    LOG("[" << to_hex(cr.region.start_key) << "," << to_hex(cr.region.end_key) << ") => Leader store:" 
        << cr.region.leader.store_id << std::endl);
  }
}

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
    boost::unique_lock<boost::shared_mutex> l(region_lock_);
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

Result<region_cache::rpc_context, Error>
region_cache::get_rpc_context(region_version_id ver_id) {
  region_lock_.lock_shared();
  auto r = get_cached_region(ver_id);
  if (r == boost::none) {
    region_lock_.unlock_shared();
    return Err(Error("region info out-of-date"));
  }
  region_info ri = *r;
  region_lock_.unlock_shared();

  auto ret = get_store_addr(ri.leader.store_id);
  if (ret.isOk()) {
    // something wrong with getting store addr
    if (ret.unwrap().size() == 0) {
      drop_region(ver_id);
      return Err(Error("region info out-of-date"));
    }

    std::string addr = ret.unwrap();
    rpc_context ctx;
    ctx.addr = addr; 
    ctx.meta = ri;
    ctx.peer = ri.leader;
    return Ok(ctx);
  } else {
    return Err(ret.unwrapErr());
  }
}

void
region_cache::drop_region(region_version_id ver_id) {
  boost::unique_lock<boost::shared_mutex> l(region_lock_);
  drop_region_from_cache(ver_id);
}

Result<std::string, Error>
region_cache::get_store_addr(uint64_t store_id) {
  // try to find it in cache
  store_lock_.lock_shared();
  auto it = stores_.find(store_id);
  if (it != stores_.end()) {
    std::string addr = it->second.addr; 
    store_lock_.unlock_shared();
    return Ok(addr);
  }
  // if not found in cache
  return reload_store_addr(store_id);
}

Result<std::string, Error>
region_cache::reload_store_addr(uint64_t store_id) {
  auto ret = pd_client_->get_store_by_id(store_id);
  if (ret.isOk()) {
    // update store cache  
    boost::unique_lock<boost::shared_mutex> l(store_lock_);
    stores_[store_id] = ret.unwrap();
    return Ok(ret.unwrap().addr);
  } else {
    return Err(ret.unwrapErr());
  }
}

void
region_cache::update_region_leader(region_version_id ver_id, uint16_t leader_store_id) {
  boost::unique_lock<boost::shared_mutex> l(region_lock_);
  auto r = get_cached_region(ver_id);
  if (r != boost::none) {
    r->switch_leader(leader_store_id);
  }
}

void
region_cache::insert_region_to_cache(const region_info& r) {
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

void 
region_cache::drop_region_from_cache(region_version_id ver_id) {
  auto it = cached_regions_.find(ver_id);
  if (it == cached_regions_.end()) {
    return ;
  }
  sorted_.erase(it->second.region.end_key);
  cached_regions_.erase(it);
}

boost::optional<region_info&>
region_cache::get_cached_region(region_version_id ver_id) {
  auto it = cached_regions_.find(ver_id);
  if (it != cached_regions_.end() && it->second.is_valid()) {
    // update access time
    uint64_t now = ::time(NULL);
    std::atomic_compare_exchange_strong(&(it->second.last_access), &now, now);
    return it->second.region;
  }
  return boost::none;
}

boost::optional<region_info&>
region_cache::search_cache(const std::string& key) {
  auto it = sorted_.upper_bound(key);
  if (it != sorted_.end() && it->second.contains(key)) {
    return get_cached_region(it->second.ver_id);
  }
  return boost::none;
}

Result<region_info, Error>
region_cache::load_region_from_pd(const std::string& key) {
  auto ret = pd_client_->get_region(key);
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
  auto ret = pd_client_->get_region_by_id(region_id);
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
