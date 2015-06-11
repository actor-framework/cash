/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2015                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#include <set>
#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <functional>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/riac/all.hpp"
#include "caf/cash/shell.hpp"

using namespace caf;
using namespace std;

constexpr char welcome_text[] = R"__(
    _________   _____ __  __
   / ____/   | / ___// / / /  C++
  / /   / /| | \__ \/ /_/ /   Actor
 / /___/ ___ |___/ / __  /    Shell
 \____/_/  |_/____/_/ /_/
)__";

int main(int argc, char** argv) {
  announce<vector<node_id>>("node_id_vector");
  riac::announce_message_types();
  string host;
  uint16_t port = 0;
  auto res = message_builder(argv + 1, argv + argc).extract_opts({
    {"host,H", "IP or hostname of nexus", host},
    {"port,p", "port of published nexus actor", port}
  });
  if (! res.remainder.empty() || host.empty() || port == 0) {
    cout << res.helptext << endl;
    return 1;
  }
  if (res.opts.count("help") > 0) {
    return 0;
  }
  auto nexus = io::typed_remote_actor<riac::nexus_type>(host, port);
  cout << welcome_text << endl;
  { // lifetime scope of shell
    cash::shell sh;
    sh.run(nexus);
  }
  await_all_actors_done();
  shutdown();
}
