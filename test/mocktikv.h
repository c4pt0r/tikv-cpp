#pragma once
#include <boost/thread/shared_mutex.hpp>
#include "meta.h"

namespace tikv {

class Cluster {
 public:
  Cluster(): id_(0) {}
  ~Cluster() {}

  u_int64_t allocate_id();

  void add_peer(uint64_t region_id, uint64_t store_id, uint64_t peer_store_id);
  void add_store(uint64_t store_id, const std::string& addr);
  void remove_peer(uint64_t region_id, uint64_t peer_store_id);
  void remove_store(uint64_t store_id);
  void start_store(uint64_t store_id);
  void shutdown_store(uint64_t store_id);

  void split(uint64_t region_id, 
             uint64_t new_region_id, 
             const std::string& split_key,
             const std::vector<uint64_t>& peer_ids,
             uint64_t leader_peer_id);

 private:
  uint64_t allocate_id_inner();

 private:
  boost::shared_mutex lock_;
  uint64_t id_;
  std::map<uint64_t, Region> regions_;
  std::map<uint64_t, Store> stores_;
};

};

