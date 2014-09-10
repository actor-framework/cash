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
#include "caf/riac/all.hpp"
#include "caf/shell/shell.hpp"
#include "cppa/opt.hpp"

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
  riac::announce_message_types();
  string host;
  uint16_t port = 0;
  options_description desc;
  bool args_valid = match_stream<string> (argv + 1, argv + argc) (
    on_opt1('H', "caf-nexus-host", &desc, "set nexus host") >> rd_arg(host),
    on_opt1('p', "caf-nexus-port", &desc, "set nexus port") >> rd_arg(port),
    on_opt0('h', "help", &desc, "print help") >> print_desc_and_exit(&desc)
  );
  if(!args_valid || port == 0 || host.empty()) {
    auto desc_printer = print_desc(&desc, cerr);
    desc_printer();
    return 42;
  }
  auto nexus = io::typed_remote_actor<riac::nexus_type>(host, port);
  cout << welcome_text << endl;
  { // lifetime scope of shell
    shell::shell sh;
    sh.run(nexus);
  }
  await_all_actors_done();
  shutdown();
}
