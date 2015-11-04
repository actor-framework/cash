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

#include "caf/cash/shell.hpp"

#include <set>
#include <map>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <iterator>
#include <iostream>
#include <algorithm>

#include "caf/io/all.hpp"
#include "caf/io/network/protocol.hpp"
#include "caf/riac/nexus_proxy.hpp"

using std::cout;
using std::endl;
using std::setw;
using std::left;
using std::right;
using std::flush;
using caf::io::network::protocol;

namespace {

std::string progressbar(size_t percent, char sign = '#', int amount = 50) {
  // make sure percent is in between 0 and 100
  percent = std::min(std::max(percent, size_t{0}), size_t{100});
  std::ostringstream s;
  s << "["
    << left << setw(amount)
    << std::string(percent, sign)
    << "] " << right;
  return s.str();
}

} // namespace <anonymous>

namespace caf {
namespace cash {

shell::shell() : done_(false), engine_(sash::variables_engine<>::create()) {
  // register global commands
  std::vector<cli_type::mode_type::cmd_clause> global_cmds {
    {"quit",          "terminates the whole thing",    cb(&shell::quit)},
    {"echo",          "prints its arguments",          cb(&shell::echo)},
    {"clear",         "clears screen",                 cb(&shell::clear)},
    {"sleep",         "sleep for n milliseconds",      cb(&shell::sleep)},
    {"help",          "prints this text",              cb(&shell::help)},
    {"all-routes",    "prints all direct routes",      cb(&shell::all_routes)},
    {"list-nodes",    "prints all available nodes",    cb(&shell::list_nodes)},
    {"mailbox",       "prints the shell's mailbox",    cb(&shell::mailbox)},
    {"test-nodes",    "loads static dummy-nodes",      cb(&shell::test_nodes)},
    {"change-node",   "switch between nodes",          cb(&shell::change_node)},
    {"dequeue",       "removes element from mailbox",  cb(&shell::dequeue)},
    {"pop-front",     "removes oldest mailbox element",cb(&shell::pop_front)},
    {"await-msg",     "awaits and prints a message",   cb(&shell::await_msg)}
  };
  std::vector<cli_type::mode_type::cmd_clause> node_cmds {
    {"whereami",      "prints current node",           cb(&shell::whereami)},
    {"leave-node",    "returns to global mode",        cb(&shell::leave_node)},
    {"send",          "sends a message to an actor",   cb(&shell::send)},
    {"work-load",     "prints CPU load",               cb(&shell::work_load)},
    {"ram-usage",     "prints RAM usage",              cb(&shell::ram_usage)},
    {"statistics",    "prints statistics",             cb(&shell::statistics)},
    {"interfaces",    "prints all interfaces",         cb(&shell::interfaces)},
    {"direct-routes", "prints all connected nodes",    cb(&shell::direct_conn)},
    {"list-actors",   "prints all known actors",       cb(&shell::list_actors)}
  };
  auto global_mode  = cli_.mode_add("global", "$ ");
  auto node_mode    = cli_.mode_add("node"  , "$ ");
  global_mode->add_all(global_cmds);
  node_mode->parent(global_mode);
  node_mode->add_all(node_cmds);
  cli_.add_preprocessor(engine_->as_functor());
  cli_.mode_push("global");
  nexus_proxy_ = spawn(riac::nexus_proxy);
}

void shell::run(riac::nexus_type nexus) {
  if (! nexus) {
    cout << "Run in detached mode (no nexus available)" << endl;
  } else {
    cout << "Register proxy at nexus ..." << std::flush;
    self_->send(nexus, riac::add_typed_listener{nexus_proxy_});
  }
  std::string line;
  while (! done_) {
    cli_.read_line(line);
    switch (cli_.process(line)) {
      default:
        break;
      case sash::executed:
        cli_.append_to_history(line);
        break;
      case sash::no_command:
        cli_.append_to_history(line);
        cout << cli_.last_error() << endl;
        break;
    }
  }
  anon_send_exit(nexus_proxy_, exit_reason::user_shutdown);
}

void shell::quit(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  done_ = true;
}

void shell::echo(char_iter first, char_iter last) {
  std::copy(first, last, std::ostream_iterator<char>(cout));
  cout << endl;
}

void shell::clear(char_iter, char_iter) {
  set_error("Implementation so far to clear screen: 'ctrl + l'");
}

using three_strings = std::tuple<std::string, std::string, std::string>;
using help_line = std::vector<three_strings>;

void fill_commands(std::set<std::string>& mode_names,
                   help_line& cmds,
                   size_t& max_mode_name_size,
                   size_t& max_cmd_name_size,
                   const cash::shell::cli_type::mode_type& mptr) {
  max_mode_name_size = std::max(max_mode_name_size, mptr.name().size());
  mode_names.emplace(mptr.name());
  mptr.foreach_command([&](const cash::shell::cli_type::command_ptr& cmd) {
    // do not insert commands that are already defined
    // -> submode commands override global commands
    auto pred = [&](const three_strings& x) {
      return get<1>(x) == cmd->name();
    };
    if (std::none_of(cmds.begin(), cmds.end(), std::move(pred))) {
      cmds.emplace_back(std::forward_as_tuple(mptr.name(),
                                              cmd->name(),
                                              cmd->description()));
      max_cmd_name_size = std::max(max_cmd_name_size, cmd->name().size());
    }
  });
  if (mptr.parent() != nullptr)
    fill_commands(mode_names, cmds, max_mode_name_size,
                  max_cmd_name_size, *mptr.parent());
}

void shell::help(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  std::set<std::string> modes;
  help_line cmds;
  size_t max_mode_name_size = sizeof("[mode]");
  size_t max_cmd_name_size = sizeof("[command]");
  fill_commands(modes, cmds, max_mode_name_size,
                max_cmd_name_size, cli_.current_mode());
  cout << std::left;
  cout << std::setw(static_cast<int>(max_mode_name_size)) << "[mode]" << "  "
       << std::setw(static_cast<int>(max_cmd_name_size)) << "[command]" << "  "
       << "[description]" << endl;
  for (auto& tup : cmds)
    cout << std::setw(static_cast<int>(max_mode_name_size))
         << get<0>(tup) << "  "
         << std::setw(static_cast<int>(max_cmd_name_size))
         << get<1>(tup) << "  "
         << get<2>(tup) << endl;
}

void shell::test_nodes(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  node_id n1(42,   "afafafafafafafafafafafafafafafafafafafaf");
  node_id n2(123,  "bfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf");
  node_id n3(1231, "000000000fbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf");
  send_invidually(
    riac::node_info{
      n1,
      {{n1, 2, 2300}},
      "Sokrates",
      "Mac OS X",
      {{"en0", {{protocol::ethernet,
                 {"00:00:FF:FF:92:00"}}}}}
    },
    riac::work_load{n1, 0, 5, 3},
    riac::ram_usage{n1, 512, 1024}
  );
  send_invidually(
    riac::node_info{
      n2,
      {{n2, 4, 1500}, {n2, 32, 3500}},
      "Platon",
      "Linux",
      {{"wlan0", {{protocol::ethernet,
                   {"00:00:FF:FF:00:00"}}}}}
    },
    riac::work_load{n2, 10, 20, 3},
    riac::ram_usage{n2, 1024, 8096}
  );
  send_invidually(
    riac::node_info{
      n3,
      {{n3, 4, 1500}, {n3, 8, 2500}, {n3, 64, 5500}},
      "hostname123",
      "BSD",
      {{"en1", {{protocol::ethernet,
                 {"00:00:FF:FF:00:00"}}}}}
    },
    riac::work_load{n3, 23, 20, 3},
    riac::ram_usage{n3, 1024, 8096}
  );
}

void shell::list_nodes(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  self_->sync_send(nexus_proxy_, riac::list_nodes::value).await(
    [=](std::vector<node_id>& nodes) {
      if (nodes.empty())
        cout << " no nodes avaliable" << endl;
      for (auto& node : nodes) {
        self_->sync_send(nexus_proxy_, riac::get_node::value, node).await(
          [=](const riac::node_info& ni) {
            auto node_str = to_hostname(ni.source_node);
            if (! node_str) {
              set_error("list-nodes: cannot convert node.");
              return;
            }
            cout << *node_str << endl;
          },
          [=](error_atom) {
            set_error("Unexpected error.");
          }
        );
      }
    }
  );
}

void shell::sleep(char_iter first, char_iter last) {
  if (first == last)
    return;
  int time = std::stoi(std::string(first, last));
  std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

void shell::whereami(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  auto node_host = to_hostname(node_);
  if (! node_host) {
    set_error("whereami: can't convert node-id.");
    return;
  }
  cout << *node_host << endl;
}

void shell::change_node(char_iter first, char_iter last) {
  if (first == last)
    return;
  std::string input_str(first, last);
  auto input_node = from_string<node_id>(input_str);
  if (! input_node) {
    auto host_node = from_hostname(input_str);
    if (! host_node) {
      std::string error = "change-node: invalid host format or ambiguous or"
                          " not known host";
      set_error(error);
    } else {
      set_node(*host_node);
    }
    return;
  }
  // check if valid node is known
  std::string unkown_id = "change-node: unknown node-id. ";
  self_->sync_send(nexus_proxy_, riac::get_node::value, *input_node).await(
    [&](const riac::node_info&) {
      set_node(*input_node);
    },
    [&](error_atom) {
      set_error(unkown_id);
    }
  );
}

void shell::all_routes(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  self_->sync_send(nexus_proxy_, riac::list_nodes::value).await(
    [&](const std::vector<node_id>& nodes) {
      for (auto& node : nodes) {
        cout << get_routes(node) << endl;
      }
    }
  );
}

void shell::leave_node(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  cli_.mode_pop();
  cout << "Leaving node-mode" << endl;
  engine_->unset("NODE");
}

void shell::work_load(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  self_->sync_send(nexus_proxy_, riac::get_sys_load::value, node_).await(
    [&](const riac::work_load& wl) {
      cout << setw(20) << "Processes: "
           << setw(3)  << wl.num_processes
           << endl
           << setw(20) << "Actors: "
           << setw(3)  << wl.num_actors
           << endl
           << "CPU: " << progressbar(wl.cpu_load / 2, '#')
           << static_cast<int>(wl.cpu_load) << "%"
           << endl;
    },
    [&](error_atom) {
      cout << "No work load statistics available for node" << endl;
    }
  );
}

void shell::ram_usage(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  self_->sync_send(nexus_proxy_, riac::get_ram_usage::value, node_).await(
    [&](const riac::ram_usage& ru) {
      auto used_ram_in_percent = (ru.in_use * 100.0) / ru.available;
      cout << "RAM: "
           << progressbar(static_cast<size_t>(used_ram_in_percent / 2.0), '#')
           << ru.in_use << "/" << ru.available
           << endl;
    },
    [&](error_atom) {
      cout << "No ram usage statistics available for node" << endl;
    }
  );
}

void shell::statistics(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  self_->sync_send(nexus_proxy_, riac::get_node::value, node_).await(
    [&](const riac::node_info& ni) {
      cout << setw(21) << "Node-ID:  "
           << setw(50) << left << to_string(ni.source_node) << right << endl
           << setw(21) << "Hostname:  " << ni.hostname << endl
           << setw(21) << "Operating system:  " << ni.os << endl
           << setw(20) << "CPU statistics: "
           << setw(3)  << "#"
           << setw(10) << "Core No"
           << setw(12) << "MHz/Core"
           << endl;
      for (size_t i = 0; i < ni.cpu.size(); ++i) {
        cout << setw(23) << i
             << setw(10) << ni.cpu[i].num_cores
             << setw(12) << ni.cpu[i].mhz_per_core
             << endl;
      }
      work_load(first, last);
      ram_usage(first, last);
    },
    [&](error_atom) {
      cout << "No ram usage statistics available for node" << endl;
    }
  );
}

void shell::direct_conn(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  cout << get_routes(node_) << endl;
}

void shell::interfaces(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  self_->sync_send(nexus_proxy_, riac::get_node::value, node_).await(
    [&](const riac::node_info& ni) {
      const char* indent = "    ";
      for (auto& interface : ni.interfaces) {
        cout << interface.first << ":" << endl;
        for (auto& addresses : interface.second) {
          for (auto& address : addresses.second) {
            cout << indent << to_string(addresses.first)
                 << " " << address << endl;
          }
        }
      }
    },
    [&](error_atom) {
      cout << "No ram usage statistics available for node" << endl;
    }
  );
}

void shell::send(char_iter first, char_iter last) {
  const char* cfirst = &(*first);
  const char* clast = &(*last);
  char* pos;
  auto aid = static_cast<uint32_t>(strtol(cfirst, &pos, 10));
  if (pos == cfirst || pos == clast) {
    set_error("missing actor ID as first argument");
    return;
  }
  if (*pos != ' ') {
    set_error("invalid format: missing whitespace after actor ID");
    return;
  }
  const char* msg_begin = pos + 1;
  auto msg = from_string<message>(std::string(msg_begin, clast));
  if (! msg) {
    set_error("cannot deserialize a message from given input");
    return;
  }
  auto m = *msg;
  self_->sync_send(nexus_proxy_, riac::get_actor::value, node_, aid).await(
    [&](const actor_addr& handle) {
      if (handle != invalid_actor_addr) {
        cout << "send: no actor known with ID " << aid << endl;
        return;
      }
      user_->send(actor_cast<actor>(handle), std::move(*msg));
    }
  );
}

void shell::mailbox(char_iter, char_iter) {
  // TODO: implement me
  set_error("mailbox: not implemented yet");
}

void shell::dequeue(char_iter, char_iter) {
  // TODO: implement me
  set_error("dequeue: not implemented yet");
}

void shell::pop_front(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  user_->receive(
    others >> [&] {
      cout << to_string(user_->current_message()) << endl;
    },
    after(std::chrono::seconds(0)) >> [] {
      cout << "pop-front: mailbox is empty" << endl;
    }
  );
}

void shell::await_msg(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  user_->receive(
    others() >> [&] {
      cout << to_string(user_->current_message()) << endl;
    }
  );
}

void shell::list_actors(char_iter first, char_iter last) {
  if (! assert_empty(first, last))
    return;
  auto nid = node_;
  actor self = self_;
  auto mm = io::middleman::instance();
  mm->run_later([nid, self, mm] {
    auto bro = mm->get_named_broker<io::basp_broker>(atom("BASP"));
    auto proxies = bro->state.get_namespace().get_all(nid);
    std::ostringstream oss;
    for (auto& p : proxies) {
      oss << p->id() << endl;
    }
    anon_send(self, atom("ListActors"), oss.str());
  });
  // wait for result
  self_->sync_send(nexus_proxy_, riac::list_actors::value, node_).await(
    [&](const std::vector<actor_addr>& res) {
      if (res.empty()) {
        cout << "list-actors: no actors known on this host" << endl;
      }
      for (auto& addr : res)
        cout << to_string(addr) << endl;
    }
  );
}

void shell::set_node(const node_id& id) {
  auto node_str = to_string(id);
  engine_->set("NODE", node_str);
  node_ = id;
  cli_.mode_push("node");
}

std::string shell::get_routes(const node_id& id) {
  std::stringstream accu;
  self_->sync_send(nexus_proxy_, riac::list_peers::value, id).await(
    [&](const std::vector<node_id>& conn) {
      auto current_node = to_hostname(id);
      if (! current_node) {
        set_error("direct-routes: ");
        return;
      }
      accu << *current_node << " ->"
           << endl;
      for (auto& ni : conn) {
        auto neighbour = to_hostname(ni);
        if (! neighbour) {
          set_error("direct-routes: can't convert neighbour.");
          return;
        }
        accu << " " << *neighbour
             << endl;
      }
    }
  );
  return accu.str();
}

maybe<node_id> shell::from_hostname(const std::string& input) {
  std::vector<std::string> hostname;
  caf::split(hostname, input, ":");
  // check valid format
  auto input_size = hostname.size();
  if (input_size < 1 || 2 < input_size)
    return none;
  maybe<node_id> ni;
  self_->sync_send(nexus_proxy_, riac::list_nodes::value, hostname.front()).await(
    [&](const std::vector<node_id>& nodes_on_host) {
      if (nodes_on_host.size() == 1 && input_size == 1) {
        ni = nodes_on_host.front();
      } else if (input_size == 2) {
        try {
          auto process_id = static_cast<uint32_t>(std::stoi(hostname.back()));
          auto node_itr = find_if(nodes_on_host.begin(), nodes_on_host.end(),
            [=](const node_id& node_on_host) {
              return node_on_host.process_id() == process_id;
            }
          );
          if (node_itr != nodes_on_host.end())
            ni = *node_itr;
        } catch (...) {
          // nop
        }
      }
    }
  );
  return ni;
}

maybe<std::string> shell::to_hostname(const node_id& node) {
  if (node == invalid_node_id)
    return none;
  std::stringstream ss;
  self_->sync_send(nexus_proxy_, riac::list_nodes::value).await(
    [&](const std::vector<node_id>& nodes) {
      self_->sync_send(nexus_proxy_, riac::get_node::value, node).await(
        [&](const riac::node_info& ni) {
          // explict cast to use +=
          ss << ni.hostname;
          if (nodes.size() > 1)
            ss << ":" + std::to_string(ni.source_node.process_id());
        }
      );
    }
  );
  std::string hostname = ss.str();
  if (hostname.empty())
    return none;
  return hostname;
}

} // namespace cash
} // namespace caf
