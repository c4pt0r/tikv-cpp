#include <atomic>
#include <iostream>
#include "region_cache.h"


namespace tikv {

void
RegionCache::dump_cache() {
  for (auto it = sorted_.begin(); it != sorted_.end(); it++) {
    cached_region cr = cached_regions_[it->second.ver_id];
    LOG("[" << to_hex(cr.region.start_key) << "," << to_hex(cr.region.end_key) << ") => Leader store:" 
        << cr.region.leader->store_id << std::endl);
  }
}

Result<RegionCache::key_loc, Error>
RegionCache::locate_key(const std::string& key, bool& cache_hit) {
  region_lock_.lock_shared();
  auto cached = search_cache(key);
  if (cached != boost::none) {
    LOG("got region info in region cache!");
    key_loc ret;
    ret.start_key = cached->start_key;
    ret.end_key = cached->end_key;
    ret.region_ver_id = cached->ver_id;
    region_lock_.unlock_shared();
    cache_hit = true;
    return Ok(ret);
  }
  region_lock_.unlock_shared();

  auto r = load_region_from_pd(key);
  if (r.isOk()) {
    boost::unique_lock<boost::shared_mutex> l(region_lock_);
    LOG("load region from pd");
    Region region = r.unwrap();
    insert_region_to_cache(region);
    key_loc ret;
    ret.start_key = region.start_key;
    ret.end_key = region.end_key;
    ret.region_ver_id = region.ver_id;
    cache_hit = false;
    return Ok(ret);
  } else {
    return Err(r.unwrapErr());
  }
}

Result<RegionCache::rpc_context, Error>
RegionCache::get_rpc_context(RegionVerID ver_id) {
  region_lock_.lock_shared();
  auto r = get_cached_region(ver_id);
  if (r == boost::none) {
    region_lock_.unlock_shared();
    return Err(Error("region info out-of-date"));
  }
  Region ri = *r;
  region_lock_.unlock_shared();

  auto ret = get_store_addr(ri.leader->store_id);
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
    ctx.peer = *ri.leader;
    return Ok(ctx);
  } else {
    return Err(ret.unwrapErr());
  }
}

void
RegionCache::drop_region(RegionVerID ver_id) {
  boost::unique_lock<boost::shared_mutex> l(region_lock_);
  drop_region_from_cache(ver_id);
}

Result<std::string, Error>
RegionCache::get_store_addr(uint64_t store_id) {
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
RegionCache::reload_store_addr(uint64_t store_id) {
  auto ret = PDClient_->get_store_by_id(store_id);
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
RegionCache::update_region_leader(RegionVerID ver_id, uint16_t leader_store_id) {
  boost::unique_lock<boost::shared_mutex> l(region_lock_);
  auto r = get_cached_region(ver_id);
  if (r != boost::none) {
    r->switch_leader(leader_store_id);
  }
}

void
RegionCache::insert_region_to_cache(const Region& r) {
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
RegionCache::drop_region_from_cache(RegionVerID ver_id) {
  auto it = cached_regions_.find(ver_id);
  if (it == cached_regions_.end()) {
    return ;
  }
  sorted_.erase(it->second.region.end_key);
  cached_regions_.erase(it);
}

boost::optional<Region&>
RegionCache::get_cached_region(RegionVerID ver_id) {
  auto it = cached_regions_.find(ver_id);
  if (it != cached_regions_.end() && it->second.is_valid()) {
    // update access time
    uint64_t now = ::time(NULL);
    std::atomic_compare_exchange_strong(&(it->second.last_access), &now, now);
    return it->second.region;
  }
  return boost::none;
}

boost::optional<Region&>
RegionCache::search_cache(const std::string& key) {
  auto it = sorted_.upper_bound(key);
  if (it != sorted_.end() && it->second.contains(key)) {
    return get_cached_region(it->second.ver_id);
  }
  return boost::none;
}

Result<Region, Error>
RegionCache::load_region_from_pd(const std::string& key) {
  auto ret = PDClient_->get_region(key);
  if (!ret.isOk()) {
    return Err(ret.unwrapErr());
  } 
  Region r = ret.unwrap().first;
  const Peer &leader = ret.unwrap().second;

  if (r.peers.size() == 0) {
    // this region has not peer
    return Err(Error("this region has no peers"));
  }

  if (leader.id > 0 && leader.store_id > 0) {
    r.switch_leader(leader.store_id);
  }
  return Ok(r);
}

Result<Region, Error>
RegionCache::load_region_from_pd_by_id(uint64_t region_id) {
  auto ret = PDClient_->get_region_by_id(region_id);
  if (!ret.isOk()) {
    return Err(ret.unwrapErr());
  } 
  Region r = ret.unwrap().first;
  const Peer &leader = ret.unwrap().second;

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
