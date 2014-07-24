
#ifndef TEST_NODES
#define TEST_NODES

#include <set>
#include <iostream>
#include <iomanip>

#include "caf/all.hpp"
#include "sash/sash.hpp"
#include "sash/libedit_backend.hpp" // our backend
#include "sash/variables_engine.hpp"

#include "caf/shell/shell_actor.hpp"
//#include "probe-events/caf/probe_event/probe_event.hpp"

using namespace std;
using namespace caf;
using namespace probe_event;
using namespace caf::shell;


node_data node_d1 {{node_id(42, "afafafafafafafafafafafafafafafafafafafaf"),
                   {{2,2300}},
                   "Sokrates", "MACOS",
                   {{"00:00:FF:FF:92:00", "192.168.1.12",
                   {"IPv6 a", "IPv6 b", "IPv6 c"}},
                   {"AC:10:FF:0F:9A:00", "192.168.1.12",
                   {}}}},
                   {0, 5, 3},
                   {512, 1024}};

node_data node_d2 {{node_id(123, "bfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf"),
                   {{4,1500}, {32,3500}},
                   "Planton", "Linux",
                   {{"00:00:FF:FF:00:00", "192.168.1.32",
                   {"IPv6 a"}}}},
                   {10, 20, 3},
                   {1024, 8096}};

node_data node_d3 {{node_id(1231, "000000000fbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf"),
                   {{4,1500}, {8,2500}, {64,5500}},
                   "hostname123", "Useless-OS",
                   {{"00:00:FF:FF:00:00", "192.168.1.99",
                   {"IPv6 a", "IPv6 b"}}}},
                   {23, 20, 3},
                   {1024, 8096}};

map<node_id, node_data> test_nodes() {
  map<node_id, node_data>   accu;
  accu.emplace(node_id(42, "afafafafafafafafafafafafafafafafafafafaf"),
               node_d1);
  accu.emplace(node_id(123, "bfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf"),
               node_d2);

  accu.emplace(node_id(1231, "000000000fbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf"),
               node_d3);
  return accu;
}

#endif // TEST_NODES
