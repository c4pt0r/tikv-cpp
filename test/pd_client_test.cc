#include <string>
#include "pd_client.h"
#include "logging.hpp"

#define CATCH_CONFIG_MAIN
#include "3rd_party/catch/catch.hpp"

TEST_CASE("get region by key") {
    tikv::pd_client pd("pd://localhost:2379");
    tikv::region_info region;
    tikv::resp r = pd.get_region("hello", &region);
    REQUIRE(r.ok());
}

TEST_CASE("get stores") {
    tikv::pd_client pd("pd://localhost:2379");
    tikv::region_info region;
    std::vector<tikv::store_info> ret;
    tikv::resp r = pd.get_all_stores(&ret);
    LOG("store count:" << ret.size());
    REQUIRE(r.ok());
    REQUIRE(ret.size() > 0);

    tikv::store_info info;
    r = pd.get_store_by_id(ret[0].id, &info);
    LOG("store" << ret[0].id << ":" << info.addr);
    REQUIRE(r.ok());
    REQUIRE(info.addr.size() > 0);
}
