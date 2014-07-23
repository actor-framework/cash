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

// Test purporses:
/*
struct node_data {
    probe_event::node_info node_info;
    probe_event::work_load work_load;
    probe_event::ram_usage ram_usage;
};
*/
node_data node_d1 {{node_id(42, "afafafafafafafafafafafafafafafafafafafaf"),
                   {{2,2300}}},
                   {0, 5, 3},
                   {512, 1024}};

node_data node_d2 {{node_id(123, "bfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf"),
                   {{4,1500}, {32,3500}}},
                   {10, 20, 3},
                   {1024, 8096}};

node_data node_d3 {{node_id(1231, "000000000fbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf"),
                   {{4,1500}, {8,2500}, {64,5500}}},
                   {23, 20, 3},
                   {1024, 8096}};

int main() {
  using sash::command_result;
  using char_iter = string::const_iterator;
  using cli_type = sash::sash<sash::libedit_backend>::type;
  map<node_id, node_data>   known_nodes;
  known_nodes.emplace(node_id(42, "afafafafafafafafafafafafafafafafafafafaf"),
                      node_d1);
  known_nodes.emplace(node_id(123, "bfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf"),
                      node_d2);

  known_nodes.emplace(node_id(1231, "000000000fbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf"),
                      node_d3);
  list<node_id>             visited_nodes;
  cli_type                  cli;
  string                    line;
  //auto                      shellactor   = spawn<shell_actor>();
  auto                      global_mode  = cli.mode_add("global", " > ");
  auto                      node_mode    = cli.mode_add("node"  , " > ");
  bool                      done         = false;
  cli.mode_push("global");
  auto engine = sash::variables_engine<>::create();
  cli.add_preprocessor(engine->as_functor());
  //cli.add_preprocessor(sash::variables_engine<>::create());
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
              if(!(visited_nodes.back() == *input_node)) {
                engine->set("node", to_string(*input_node));
                visited_nodes.push_back(*input_node);
              }
            } else {
              err = "Node-id is unknown";
              return sash::no_command;
            }
            // update prompt
            cli.mode_push("node");
          } else {
             err = "Invalid node-id. ";
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
                  " with 'change-node <node-id>'.";
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
          visited_nodes = list<node_id>();
          cli.mode_pop();
          cout << "Leaving node-mode..." << endl;
        }
      },
      {
        "whereiwas", "prints all node-ids visited starting with least.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          int i = visited_nodes.size();
          for(auto node : visited_nodes) {
            cout << i << ": " << to_string(node) << endl;
            i--;
          }
          return sash::executed;
        }
      },
      {
        "back", "changes location to previous node.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if(first == last) {
            if(visited_nodes.size() <= 1) {
              cout << "Leaving node-mode..." << endl;
              cli.mode_pop();
            }
            visited_nodes.pop_back();
            return sash::executed;
          } else {
            err = "list-nodes: to many arguments (none expected).";
            return sash::no_command;
          }
        }
      },
      {
        "statistics", "prints current RAM and CPU statistics of current node.",
        [&](string& err, char_iter, char_iter) -> command_result {
          auto search = known_nodes.find(visited_nodes.back());
          if(search != known_nodes.end()) {
            // node_info
            cout << setw(20) << "Node-ID: "
                 << setw(46) << to_string((search->second.node_info.id))
                 << endl ;
            cout << setw(20) << "CPU statistics: "
                 << setw(3)  << "#"
                 << setw(10) << "Core No"
                 << setw(12) << "MHz/Core" << endl;
              int i = 1;
              for(cpu_info cpu : search->second.node_info.cpu) {
                cout << setw(23) << i
                     << setw(10) << cpu.num_cores
                     << setw(12) << cpu.mhz_per_core << endl;
                i++;
              }
            // work_load
            cout << setw(20) << "Processes: "
                 << setw(3)  << search->second.work_load.num_processes
                 << endl
                 << setw(20) << "Actors: "
                 << setw(3)  << search->second.work_load.num_actors
                 << endl
                 << setw(20) << "CPU: "
                 << setw(2)  << "[";
            for(float i = 0; i < 50; i++) {
              if(i < search->second.work_load.cpu_load / 2) {
                cout << "#";
              } else {
                cout << " ";
              }
            }
            cout << "] "
                 << int(search->second.work_load.cpu_load) << "%" << endl;
            // ram_usage
            cout << setw(20) << "RAM: "
                 << setw(2)  << "[";
            float used_ram_in_percent= float(search->second.ram_usage.in_use)
                                     / float(search->second.ram_usage.available)
                                     * 100;
            for(float i = 0; i < 50; i++) {
             if(i < used_ram_in_percent / 2) {
               cout << "#";
             } else {
               cout << " ";
             }
            }
            cout << "] "
                << search->second.ram_usage.in_use
                << "/"
                << search->second.ram_usage.available
                << endl;
          }
          return sash::executed;
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
  await_all_actors_done();
  shutdown();
}
