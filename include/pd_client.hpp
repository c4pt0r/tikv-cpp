#pragma once
#include <string>
#include <unistd.h>

#include "kvproto/pdpb.grpc.pb.h"
#include "client.hpp"
#include "meta.hpp"

namespace tikv {

class pd_client {
 public:
  pd_client(const std::string& addr);

  uint64_t get_cluster_id();
  
  // GetRegion gets a region and its leader Peer from PD by key.
  // The region may expire after split. Caller is responsible for caching and
  // taking care of region change.
  // Also it may return nil if PD finds no Region for the key temporarily,
  // client should retry later.
  resp get_region(const std::string& key, region_info& ret); 

  resp get_region_by_id(uint64_t region_id, region_info& ret);

  resp get_store(uint64_t store_id, store_info& ret);

private:
  std::unique_ptr<pdpb::PD::Stub> stub_;
};

};
