
#ifndef CAF_SHELL_TEST_NODES_HPP
#define CAF_SHELL_TEST_NODES_HPP

#include <set>
#include <iomanip>
#include <iostream>

#include "caf/all.hpp"
#include "caf/shell/shell_actor.hpp"

using namespace std;
using namespace caf;
using namespace probe_event;
using namespace caf::shell;

map<node_id, node_data> test_nodes() {
  node_id n1(42,   "afafafafafafafafafafafafafafafafafafafaf");
  node_id n2(123,  "bfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf");
  node_id n3(1231, "000000000fbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf");
  return {
    {
      n1,
      {
        probe_event::node_info{
          n1,
          {{n1, 2, 2300}},
          "Sokrates", "MACOS",
          {
            {
              n1,
              "00:00:FF:FF:92:00",
              "192.168.1.12",
              {"IPv6 a", "IPv6 b", "IPv6 c"}
            },
            {
              n1,
              "AC:10:FF:0F:9A:00",
              "192.168.1.12",
              {}
            }
          }
        },
        probe_event::work_load{n1, 0, 5, 3},
        probe_event::ram_usage{n1, 512, 1024}
      }
    },
    {
      n2,
      {
        probe_event::node_info{
          n2,
          {{n2, 4, 1500}, {n2, 32, 3500}},
          "Planton",  "Linux",
          {
            {
              n2,
              "00:00:FF:FF:00:00",
              "192.168.1.32",
              {"IPv6 a"}
            }
          }
        },
        probe_event::work_load{n2, 10, 20, 3},
        probe_event::ram_usage{n2, 1024, 8096}
      }
    },
    {
      n3,
      {
        {
          n3,
          {{n3, 4, 1500}, {n3, 8, 2500}, {n3, 64, 5500}},
          "hostname123", "Useless-OS",
          {
            {
              n3,
              "00:00:FF:FF:00:00",
              "192.168.1.99",
              {"IPv6 a", "IPv6 b"}
            }
          }
        },
        probe_event::work_load{n3, 23, 20, 3},
        probe_event::ram_usage{n3, 1024, 8096}
      }
    }
  };
}

#endif // CAF_SHELL_TEST_NODES
