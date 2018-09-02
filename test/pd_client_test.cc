#include <string>
#include "pd_client.h"
#include "logging.h"
#include "3rd_party/cpp-btree/btree_map.h"

#define CATCH_CONFIG_MAIN
#include "3rd_party/catch/catch.hpp"

TEST_CASE("get region by key") {
    tikv::pd_client pd("pd://localhost:2379");
    tikv::region_info region;
    tikv::peer_info leader;
    auto r = pd.get_region("hello");
    region = r.unwrap().first;
    leader = r.unwrap().second;
    REQUIRE(r.isOk());
}

TEST_CASE("get stores") {
    tikv::pd_client pd("pd://localhost:2379");
    tikv::region_info region;
    auto r = pd.get_all_stores();
    LOG("store count:" << r.unwrap().size());
    REQUIRE(r.isOk());
    REQUIRE(r.unwrap().size() > 0);

    auto rr = pd.get_store_by_id(r.unwrap()[0].id);
    LOG("store" << r.unwrap()[0].id << ":" << rr.unwrap().addr);
    REQUIRE(rr.isOk());
    REQUIRE(rr.unwrap().addr.size() > 0);
}

TEST_CASE("btree") {
  btree::btree_map<int, std::string> m;
  m[1] = "1";
  m[3] = "3";
  m[5] = "5";

  auto it = m.upper_bound(4);
  REQUIRE(it.key() == 5);
  it = m.lower_bound(4);
  REQUIRE(it.key() == 5);

  it = m.upper_bound(3);
  REQUIRE(it.key() == 5);
  it = m.lower_bound(3);
  REQUIRE(it.key() == 3);
}