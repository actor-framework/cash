/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENCE_ALTERNATIVE.       *
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
#include "caf/shell/args.hpp"
#include "caf/probe_event/all.hpp"
#include "caf/shell/shell.hpp"

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
  announce<vector<node_id>>();
  probe_event::announce_types();
  args::net_config config;
  args::from_args(config, argc, argv);
  auto nexus = io::typed_remote_actor<probe_event::nexus_type>(config.host,
                                                               config.port);
  if(!config.valid()) {
    args::print_help();
    return 42;
  }
  cout << welcome_text << endl;
  { // lifetime scope of shell
    shell::shell sh;
    sh.run(nexus);
  }
  await_all_actors_done();
  shutdown();
}
