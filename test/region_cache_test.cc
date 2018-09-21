#include <string>
#include <memory>
#include "logging.h"
#include "region_cache.h"
#include "3rd_party/cpp-btree/btree_map.h"

#include "3rd_party/catch/catch.hpp"

TEST_CASE("get region and cache by key") {
  auto pd = std::make_shared<tikv::PDClient>("pd://localhost:2379");
  tikv::RegionCache rc(pd);

  bool hit = false;
  auto r = rc.locate_key("hello", hit);
  REQUIRE(hit == false);
  auto rr = rc.locate_key("hello", hit);
  REQUIRE(rr.isOk());
  REQUIRE(hit == true);
  rc.dump_cache();
}

