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
#include "caf/shell/test_nodes.hpp"
#include "sash/sash.hpp"
#include "sash/libedit_backend.hpp" // our backend
#include "sash/variables_engine.hpp"

#include "caf/shell/shell_actor.hpp"
//#include "probe-events/caf/probe_event/probe_event.hpp"

using namespace std;
using namespace caf;
using namespace probe_event;
using namespace caf::shell;

using char_iter = string::const_iterator;

inline bool empty(string& err, char_iter first, char_iter last) {
  if (first != last) {
    err = "to many arguments (none expected).";
    return false;;
  }
  return true;
}

/// @param percent of progress.
/// @param filling sign. default is #
/// @param amout of signs. default is 50.
string progressbar(int percent, char sign = '#', int amount = 50) {
  if (percent > 100 || percent < 0) {
    return "[ERROR]: Invalid percent in progressbar";
  }
  stringstream s;
  s << "["
    << left << setw(amount)
    << string(percent, sign)
    << "] " << right << flush;
  return s.str();
}

int main() {
  using sash::command_result;
  using cli_type = sash::sash<sash::libedit_backend>::type;
  map<node_id, node_data>   known_nodes;
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
  engine->set("$NODEHIST", "10");
  vector<cli_type::mode_type::cmd_clause> global_cmds {
      {
        "quit", "terminates the whole thing.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (first != last) {
            err = "quit: to many arguments (none expected).";
            return sash::no_command;
          }
          done = true;
          return sash::executed;
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
        "clear", "clears screen.",
        [](string& err, char_iter, char_iter) {
          err = "Implementation so far to clear screen: 'ctrl + l'.";
          return sash::no_command;
        }
      },
      {
        "help", "prints this text",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (!empty(err, first, last)) {
            return sash::no_command;
          }
          // doin' it complicated!
          string cmd = "echo ";
          cmd += cli.current_mode().help();
          return cli.process(cmd);
        }
      },
      {
        "test-nodes", "loads static dummy-nodes.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (!empty(err,first,last)) {
            return sash::no_command;
          }
          auto nodes = test_nodes();
          for (auto kvp : nodes) {
            known_nodes.emplace(kvp.first, kvp.second);
          }
          return sash::executed;
        }
      },
      {
        "list-nodes", "prints all available nodes.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (!empty(err, first, last)) {
            return sash::no_command;
          }
          if (known_nodes.empty()) {
            cout << "--- no nodes available ---"
                 << endl;
          }
          else {
            for (const auto& kvp : known_nodes) {
              cout << to_string(kvp.first)
                   << endl;
            }
          }
          return sash::executed;
        }
      },
      {
        "change-node", "similar to directorys you can switch between nodes.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (first == last) {
            err = "change-node: no node given";
            return sash::no_command;
          }
          auto input_node = from_string<node_id>(string(first, last));
          if (!input_node) {
            err = "change-node: invalid node-id. ";
            return sash::no_command;
          }
          if (known_nodes.count(*input_node) == 0) {
            err = "change-node: node-id is unknown";
            return sash::no_command;
          }
          auto& in = *input_node;
          if (visited_nodes.back() != in) {
            engine->set("NODE", to_string(in));
            visited_nodes.push_back(in);
            if (cli.current_mode().name() != "node")  {
              cli.mode_push("node");
            }
          }
          return sash::executed;
        }
      },
      {
        "whereami", "prints current node you are located at.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (!empty(err, first, last)) {
            return sash::no_command;
          }
          if (visited_nodes.empty()) {
            err = "You are currently in globalmode. You can select a node"
                  " with 'change-node <node-id>'.";
            return sash::no_command;
          }
          cout << "Node: " << to_string(visited_nodes.back())
               << endl;
          return sash::executed;
        }
      }
    };
  vector<cli_type::mode_type::cmd_clause> node_cmds {
      {
        "leave-node", "returns to global mode",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (!empty(err, first, last)) {
            return sash::no_command;
          }
          visited_nodes = list<node_id>();
          cli.mode_pop();
          cout << "Leaving node-mode..."
               << endl;
          engine->unset("NODE");
          return sash::executed;
        }
      },
      {
        "whereiwas", "prints all node-ids visited starting with least.",
        [&](string&, char_iter, char_iter) -> command_result {
          int i = visited_nodes.size();
          for (const auto& node : visited_nodes) {
            cout << i << ": " << to_string(node)
                 << endl;
            i--;
          }
          return sash::executed;
        }
      },
      {
        "back", "changes location to previous node.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (!empty(err, first, last)) {
            return sash::no_command;
          }
          if (visited_nodes.size() <= 1) {
            cout << "Leaving node-mode..."
                 << endl;
            engine->unset("NODE");
            cli.mode_pop();
          }
          visited_nodes.pop_back();
          return sash::executed;
        }
      },
      {
        "work-load", "prints two bars for CPU and RAM.",
        [&](string& err, char_iter first, char_iter last) -> command_result {
          if (!empty(err, first, last)) {
            return sash::no_command;
          }
          auto search = known_nodes.find(visited_nodes.back());
          cout << "CPU: "
               << progressbar(search->second.work_load.cpu_load / 2, '#')
               << static_cast<int>(search->second.work_load.cpu_load) << "%"
               << endl;
          // ram_usage
          auto used_ram_in_percent =
          static_cast<size_t>((search->second.ram_usage.in_use * 100.0)
                               / search->second.ram_usage.available);
          cout << "RAM: "
               << progressbar(used_ram_in_percent / 2, '#')
               << search->second.ram_usage.in_use
               << "/"
               << search->second.ram_usage.available
               << endl;
          return sash::executed;
        }
      },
      {
        "statistics", "prints statistics of current node.",
        [&](string&, char_iter, char_iter) -> command_result {
          auto search = known_nodes.find(visited_nodes.back());
          if (search != known_nodes.end()) {
            // node_info
            cout << setw(21) << "Node-ID:  "
                 << setw(50) << left
                 << to_string((search->second.node_info.id)) << right
                 << endl
                 << setw(21) << "Hostname:  "
                 << search->second.node_info.hostname
                 << endl
                 << setw(21) << "Operationsystem:  "
                 << search->second.node_info.os
                 << endl
                 << setw(20) << "CPU statistics: "
                 << setw(3)  << "#"
                 << setw(10) << "Core No"
                 << setw(12) << "MHz/Core"
                 << endl;
              int i = 1;
              for (const auto& cpu : search->second.node_info.cpu) {
                cout << setw(23) << i
                     << setw(10) << cpu.num_cores
                     << setw(12) << cpu.mhz_per_core
                     << endl;
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
                 << setw(2)
                 << progressbar(search->second.work_load.cpu_load/2, '#')
                 << " "
                 << static_cast<int>(search->second.work_load.cpu_load) << "%"
                 << endl;
            // ram_usage
            auto used_ram_in_percent =
              static_cast<size_t>((search->second.ram_usage.in_use * 100.0)
                                  / search->second.ram_usage.available);
            cout << setw(20) << "RAM: "
                 << setw(2)
                 << progressbar(used_ram_in_percent / 2, '#')
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
