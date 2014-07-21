/******************************************************************************\
 *                                                                            *
 *   ____         __                  ____    __              ___    ___      *
 *  / _  \       /\ \__              /\  _`\ /\ \            /\_ \  /\_ \     *
 * /\ \L\ \    __\ \ ,_\   ___   _ __\ \,\L\_\ \ \___      __\//\ \ \//\ \    *
 * \ \  __ \  /'__\ \ \/  / __`\/\`'__\/_\__ \\ \  _ `\  /'__`\\ \ \  \ \ \   *
 *  \ \ \/\ \/\ __/\ \ \_/\ \L\ \ \ \/  /\ \L\ \ \ \ \ \/\  __/ \_\ \_ \_\ \  *
 *   \ \_\ \_\ \___\\ \__\ \____/\ \_\  \ `\____\ \_\ \_\ \____\/\____\/\___\ *
 *    \/_/\/_/\/___/ \/__/\/___/  \/_/   \/_____/\/_/\/_/\/____/\/____/\/___/ *
 *                                                                            *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 * Alex    Mantel     <alex.mantel        (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the Boost Software License, Version 1.0. See             *
 * accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt  *
\******************************************************************************/

#include <set>
#include <iostream>

#include "cppa/cppa.hpp"
#include "sash/sash.hpp"
#include "sash/libedit_backend.hpp" // our backend
#include "sash/variables_engine.hpp"

<<<<<<< HEAD

// #include "probe-events/caf/probe_event/probe_event.hpp"

=======
#include "include/shell_actor.h"
//#include "probe-events/caf/probe_event/probe_event.hpp"
>>>>>>> a90828d0ff6a7888b44c6439d9c840804c2beda2

using namespace std;
using namespace caf;

int main() {
  {
  using sash::command_result;
  using char_iter = string::const_iterator;
  using cli_type = sash::sash<sash::libedit_backend>::type;
  scoped_actor              self;
  map<node_id, node_data>   known_nodes;
  known_nodes.emplace(node_id(42, "afafafafafafafafafafafafafafafafafafafaf"), node_data{});
  known_nodes.emplace(node_id(123, "bfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf"), node_data{});
  list<node_id>             visited_nodes;
  cli_type                  cli;
  string                    line;
  //auto                      shellactor   = spawn<shell_actor>();
  auto                      global_mode  = cli.mode_add("global", " > ");
  auto                      node_mode    = cli.mode_add("node"  , " > ");
  bool                      done         = false;
  cli.mode_push("global");
  cli.add_preprocessor(sash::variables_engine<>::create());
  vector<cli_type::mode_type::cmd_clause> global_cmds {
      {
        "quit", "terminates the whole thing.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (first == last) {
            done = true;
            return sash::executed;
          }
          err = "quit: to many arguments (none expected).";
          return sash::no_command;
        }
      },
      {
        "echo", "prints its arguments.",
        [](string&, char_iter first, char_iter last) -> command_result {
          copy(first, last, ostream_iterator<char>(cout));
          cout << endl;
          return sash::executed;
        }
      },
      {
        "help", "prints this text",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (first == last) {
            // doin' it complicated!
            std::string cmd = "echo ";
            cmd += cli.current_mode().help();
            return cli.process(cmd);
          }
          err = "help: to many arguments (none expected).";
          return sash::no_command;
        }
      },
      {
        "list-nodes", "prints all available nodes.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if(first != last) {
            err = "list-nodes: to many arguments (none expected).";
          }
          if(!known_nodes.empty()) {
            for(auto itr = known_nodes.begin(); itr!=known_nodes.end(); itr++) {
              cout << to_string(itr->first) << endl;
            }
          } else {
            cout << "no nodes available."  << endl;
          }
          return sash::executed;
        }
      },
      {
        "change-node", "similar to directorys you can switch between nodes.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          auto input_node = from_string<node_id>(string(first, last));
          if(input_node) {
            bool is_in = false;
            for(auto itr = known_nodes.begin(); itr!=known_nodes.end(); itr++) {
              if(input_node == itr->first) {
                is_in = true;
              }
            }
            if(is_in) {
              // currently not on this node.
              if(!(visited_nodes.back() == *input_node))
                visited_nodes.push_back(*input_node);
            } else {
              err = "Node is not known";
              return sash::no_command;
            }
            // update prompt
            cli.mode_push("node");
          } else {
             err = "Invalid node_id. ";
             return sash::no_command;
          }
          return sash::executed;
        }
      },
      {
        "whereami", "prints current node you are located at.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if(visited_nodes.empty()) {
            err = "You are currently in globalmode. You can select a node"
                  "with 'change-node <node_id>'.";
            return sash::no_command;
            } else {
              cout << "Node: " << to_string(visited_nodes.back()) << endl;
            }
          return sash::executed;
        }
      }
    };
  vector<cli_type::mode_type::cmd_clause> node_cmds {
      {
        "leave-node", "returns to global mode",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if(visited_nodes.back() != invalid_node_id) {
            cli.mode_pop();
            return sash::executed;
          } else {
            err = "[ERROR]: Invalid node_id.";
            return sash::no_command;
          }
        }
      },
      {
        "back", "changes location to previous node.",
        [&](string& err, char_iter, char_iter) -> command_result {
          if(visited_nodes.size() == 1) {
            cout << "Leaving node-mode..." << endl;
            cli.mode_pop();
          } else {
            throw new exception();
          }
          visited_nodes.pop_back();
          return sash::executed;
        }
      },
      {
        "statistics", "prints current RAM and CPU statistics of current node.",
        [&](string& err, char_iter, char_iter) -> command_result {
          err = "Not implemented now.";
          return sash::no_command;
        }
      }
  };
  global_mode->add_all(global_cmds);
  node_mode->add_all(global_cmds);
  node_mode->add_all(node_cmds);
  while (!done) {
    cli.read_line(line);
    switch (cli.process(line)) {
      default:
        break;
      case sash::nop:
        break;
      case sash::executed:
        cli.append_to_history(line);
        break;
      case sash::no_command:
        cli.append_to_history(line);
        cout << cli.last_error()
             << endl;
        break;
    }
  }
  }
  await_all_actors_done();
  shutdown();
}

