#include <string>
#include "pd_client.hpp"

#define CATCH_CONFIG_MAIN
#include "3rd_party/catch/catch.hpp"


TEST_CASE("simple") {
    tikv::pd_client pd("pd://localhost:2379");
    pd.get_cluster_id();
    REQUIRE( 3 == 3 );
}
