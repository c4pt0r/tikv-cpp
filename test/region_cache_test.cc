#include <string>
#include <memory>
#include "logging.h"
#include "region_cache.h"
#include "3rd_party/cpp-btree/btree_map.h"

#include "3rd_party/catch/catch.hpp"

TEST_CASE("get region and cache by key") {
  auto pd = std::make_shared<tikv::pd_client>("pd://localhost:2379");
  tikv::region_cache rc(pd);

  auto r = rc.locate_key("hello");

  auto rr = rc.locate_key("hello");
  REQUIRE(rr.isOk());
  rc.dump_cache();
}

