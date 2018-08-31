#include <string>
#include "pd_client.hpp"

#define CATCH_CONFIG_MAIN
#include "3rd_party/catch/catch.hpp"

TEST_CASE("get region by key") {
    tikv::pd_client pd("pd://localhost:2379");
    tikv::region_info region;
    tikv::resp r = pd.get_region("hello", &region);
    REQUIRE(r.ok());
}

