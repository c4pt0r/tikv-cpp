#include <string>
#include "logging.h"
#include "mocktikv.h"
#include "3rd_party/catch/catch.hpp"

TEST_CASE("mocktikv cluster") {
  tikv::Cluster cluster;
  cluster.bootstrap(1, {11,22,33}, {1,2,3}, 1);
  cluster.dump();

  REQUIRE(cluster.regions().size() == 1);

  tikv::Region r = cluster.regions()[1];
  REQUIRE(r.peers.size() == 3);

  cluster.split(1, 2, "hello", {1,2,3}, 1);
  cluster.dump();

}
