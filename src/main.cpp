/******************************************************************************\
 *                                                                            *
 *                 _
               | |
   ___ __ _ ___| |__
  / __/ _` / __| '_ \
 | (_| (_| \__ \ | | |
  \___\__,_|___/_| |_|


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
#include "caf/io/all.hpp"
#include "caf/shell/test_nodes.hpp"
#include "sash/sash.hpp"
#include "sash/libedit_backend.hpp" // our backend
#include "sash/variables_engine.hpp"

#include "caf/shell/shell_actor.hpp"
#include "caf/probe_event/all.hpp"

using namespace std;
using namespace caf;
using namespace probe_event;
using namespace caf::shell;

using char_iter = string::const_iterator;

// arguments
struct net_config {
  uint16_t port;
  std::string host;
  inline net_config() : port(0) { }
  inline bool valid() const {
    return port != 0 && !host.empty();
  }
};

const char host_arg[] = "--caf_nexus_host=";
const char port_arg[] = "--caf_nexus_port=";

template<size_t Size>
bool is_substr(const char (&needle)[Size], const char* haystack) {
  // compare without null terminator
  if (strncmp(needle, haystack, Size - 1) == 0) {
    return true;
  }
  return false;
}

template<size_t Size>
size_t cstr_len(const char (&)[Size]) {
  return Size - 1;
}

void from_args(net_config& conf, int argc, char** argv) {
  for (auto i = argv; i != argv + argc; ++i) {
    if (is_substr(host_arg, *i)) {
      conf.host.assign(*i + cstr_len(host_arg));
    } else if (is_substr(port_arg, *i)) {
      int p = std::stoi(*i + cstr_len(port_arg));
      conf.port = static_cast<uint16_t>(p);
    }
  }
}

inline bool empty(string& err, char_iter first, char_iter last) {
  if (first != last) {
    err = "to many arguments (none expected).";
    return false;;
  }
  return true;
}


/**
 * @param percent of progress.
 * @param filling sign. default is #
 * @param amout of signs. default is 50
 **/
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

/**
 * @brief get_node_info
 * @param self - scoped actor
 * @param sa - shellactor
 * @param err
 * @returns node_data when sa has
 */
optional<node_data> get_node_data(const scoped_actor& self,
                                  const actor& sa,
                                  string& err) {
  node_data nd;
  bool valid = true;
  self->sync_send(sa, atom("nodedata")).await(
    on(atom("ndcorrect"), arg_match) >> [&](node_info ni_,
                                            work_load wl_,
                                            ram_usage ru_) {
      nd.node_info = ni_;
      nd.ram_usage = ru_;
      nd.work_load = wl_;
    },
    on(atom("ndfail"), arg_match) >> [&](const string& msg) {
      err = msg;
      valid = false;
    }
  );
  if(valid){
    return nd;
  } else {
    return none;
  }
}

int main(int argc, char** argv) {
  announce_types(); // probe_event types
  announce<string>();
  net_config config;
  from_args(config, argc, argv);
  if(!config.valid()) {
    cerr << "Invalid arguments. Supported arguments are: "
         << endl << host_arg
         << endl << port_arg
         << endl;
    return 42;
  }
  { // scope of self
    using sash::command_result;
    using cli_type = sash::sash<sash::libedit_backend>::type;
    cli_type                  cli;
    string                    line;
    scoped_actor              self;
    auto                      shellactor   = spawn<shell_actor>();
    auto nex = io::typed_remote_actor<probe_event::nexus_type>(config.host,
                                                               config.port);
    anon_send(nex, probe_event::add_listener{shellactor});
    auto                      global_mode  = cli.mode_add("global", " $ ");
    auto                      node_mode    = cli.mode_add("node"  , " $ ");
    bool                      done         = false;
    cli.mode_push("global");
    auto engine = sash::variables_engine<>::create();
    cli.add_preprocessor(engine->as_functor());
    //TODO add sleep(N)
    vector<cli_type::mode_type::cmd_clause> global_cmds {
        {
          "quit", "terminates the whole thing.",
          [&](string& err, char_iter first, char_iter last) -> command_result {
            if (first != last) {
              err = "quit: to many arguments (none expected).";
              return sash::no_command;
            }
            self->sync_send(shellactor, atom("quit")).await(
              on(atom("done")) >> [] { }
            );
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
              self->send(shellactor, atom("add-test"), kvp.first, kvp.second);
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
            self->sync_send(shellactor, atom("list-nodes")).await(
              on(atom("done")) >> [] {
              }
            );
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
            bool valid = true;
            self->sync_send(shellactor, atom("changenode"), *input_node).await(
              on(atom("cnfail"), arg_match) >> [&](const string& msg) {
                err = msg;
                valid = false;
              },
              on(atom("cncorrect"), arg_match) >> [&](node_id id) {
                engine->set("NODE", to_string(id));
                if (cli.current_mode().name() != "node")  {
                  cli.mode_push("node");
                }
              }
            );
            return valid ? sash::executed : sash::no_command;
          }
        },
        {
          "whereami", "prints current node you are located at.",
          [&](string& err, char_iter first, char_iter last) -> command_result {
            if (!empty(err, first, last)) {
              return sash::no_command;
            }
            auto res = sash::executed;
            self->sync_send(shellactor, atom("whereami")).await(
              on(atom("waifail"), arg_match) >> [&](const string& msg) {
                err = msg;
                res = sash::no_command;
              },
              on(atom("waicorrect"), arg_match) >> [&](const string& id) {
                cout << "Current node: " << id << endl;
              }
            );
            return res;
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
            self->sync_send(shellactor, atom("leave-node"));
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
            list<node_id> visited_nodes;
            self->sync_send(shellactor, atom("visited")).await(
                  on(atom("done"), arg_match) >> [&] (const list<node_id>& ids){
                    visited_nodes = ids;
                  }
            );
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
            self->sync_send(shellactor, atom("back")).await(
              on(atom("leave")) >> [&]() {
                cli.mode_pop();
                engine->unset("NODE");
              },
              on(atom("done"), arg_match) >> [&](const node_id& new_id) {
                engine->set("NODE", to_string(new_id));
              }
            );
            return sash::executed;
          }
        },
        {
          "work-load", "prints two bars for CPU and RAM.",
          [&](string& err, char_iter first, char_iter last) -> command_result {
            if (!empty(err, first, last)) {
              return sash::no_command;
            }
            auto nd = get_node_data(self, shellactor, err);
            if(nd) {
              cout << "CPU: "
                   << progressbar(nd->work_load.cpu_load / 2, '#')
                   << static_cast<int>(nd->work_load.cpu_load) << "%"
                   << endl;
              // ram_usage
              auto used_ram_in_percent =
              static_cast<size_t>((nd->ram_usage.in_use * 100.0)
                                   / nd->ram_usage.available);
              cout << "RAM: "
                   << progressbar(used_ram_in_percent / 2, '#')
                   << nd->ram_usage.in_use
                   << "/"
                   << nd->ram_usage.available
                   << endl;
              return sash::executed;
            }
            return sash::no_command;
          }
        }/*, TODO: statistics doesn't print!
        {
          "statistics", "prints statistics of current node.",
          [&](string& err, char_iter first, char_iter last) -> command_result {
            if(empty(err, first, last)) {
              return sash::no_command;
            }
            auto nd = get_node_data(self, shellactor, err);
            if (nd) {
              cout << "here" << endl;
              // node_info
              cout << setw(21) << "Node-ID:  "
                   << setw(50) << left
                   << to_string((nd->node_info.id)) << right
                   << endl
                   << setw(21) << "Hostname:  "
                   << nd->node_info.hostname
                   << endl
                   << setw(21) << "Operationsystem:  "
                   << nd->node_info.os
                   << endl
                   << setw(20) << "CPU statistics: "
                   << setw(3)  << "#"
                   << setw(10) << "Core No"
                   << setw(12) << "MHz/Core"
                   << endl;
                int i = 1;
                for (const auto& cpu : nd->node_info.cpu) {
                  cout << setw(23) << i
                       << setw(10) << cpu.num_cores
                       << setw(12) << cpu.mhz_per_core
                       << endl;
                  i++;
                }
              // work_load
              cout << setw(20) << "Processes: "
                   << setw(3)  << nd->work_load.num_processes
                   << endl
                   << setw(20) << "Actors: "
                   << setw(3)  << nd->work_load.num_actors
                   << endl
                   << setw(20) << "CPU: "
                   << setw(2)
                   << progressbar(nd->work_load.cpu_load/2, '#')
                   << " "
                   << static_cast<int>(nd->work_load.cpu_load) << "%"
                   << endl;
              // ram_usage
              auto used_ram_in_percent =
                static_cast<size_t>((nd->ram_usage.in_use * 100.0)
                                    / nd->ram_usage.available);
              cout << setw(20) << "RAM: "
                   << setw(2)
                   << progressbar(used_ram_in_percent / 2, '#')
                   << nd->ram_usage.in_use
                   << "/"
                   << nd->ram_usage.available
                   << endl;
              return sash::executed;
            }
            return sash::no_command;
          }
        }*/
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
  } // scope of self
  await_all_actors_done();
  shutdown();
}
