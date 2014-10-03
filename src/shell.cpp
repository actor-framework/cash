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

#include "caf/cash/shell.hpp"

#include <thread>
#include <vector>
#include <chrono>
#include <iterator>
#include <iostream>
#include <algorithm>

#include "caf/io/all.hpp"
#include "caf/riac/nexus_proxy.hpp"

using std::cout;
using std::endl;
using std::setw;
using std::left;
using std::right;
using std::flush;

namespace {

std::string progressbar(int percent, char sign = '#', int amount = 50) {
  // make sure percent is in between 0 and 100
  percent = std::min(std::max(percent, 0), 100);
  std::ostringstream s;
  s << "["
    << left << setw(amount)
    << std::string(percent, sign)
    << "] " << right;
  return s.str();
}

// TODO: use cafs split
std::vector<std::string> my_split(const std::string name, const char& lim) {
  std::stringstream         s(name);
  std::vector<std::string>  words;
  std::string               word;
  while (getline(s, word, lim)) {
    words.push_back(word);
  }
 return words;
}

} // namespace <anonymous>

namespace caf {
namespace cash {

shell::shell() : m_done(false), m_engine(sash::variables_engine<>::create()) {
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
    // TODO: remove this command
    {"test",          "tests from_hostname",           cb(&shell::test)},
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
  auto global_mode  = m_cli.mode_add("global", "$ ");
  auto node_mode    = m_cli.mode_add("node"  , "$ ");
  global_mode->add_all(global_cmds);
  node_mode->add_all(global_cmds);
  node_mode->add_all(node_cmds);
  m_cli.add_preprocessor(m_engine->as_functor());
  m_cli.mode_push("global");
  m_nexus_proxy = spawn<riac::nexus_proxy>();
}

void shell::run(riac::nexus_type nexus) {
  cout << "Initiate handshake with Nexus ..." << std::flush;
  // wait until our proxy has finished its handshake
  m_self->sync_send(m_nexus_proxy, atom("Init"), nexus).await(
    on(atom("InitDone")) >> [] {
      cout << " done" << endl;
    }
  );
  std::string line;
  while (!m_done) {
    m_cli.read_line(line);
    switch (m_cli.process(line)) {
      default:
        break;
      case sash::nop:
        break;
      case sash::executed:
        m_cli.append_to_history(line);
        break;
      case sash::no_command:
        m_cli.append_to_history(line);
        cout << m_cli.last_error() << endl;
        break;
    }
  }
  anon_send_exit(m_nexus_proxy, exit_reason::user_shutdown);
}

void shell::quit(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  m_done = true;
}

void shell::echo(char_iter first, char_iter last) {
  std::copy(first, last, std::ostream_iterator<char>(cout));
  cout << endl;
}

void shell::clear(char_iter, char_iter) {
  set_error("Implementation so far to clear screen: 'ctrl + l'");
}

void shell::help(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  cout << m_cli.current_mode().help() << endl;
}

void shell::test_nodes(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  node_id n1(42,   "afafafafafafafafafafafafafafafafafafafaf");
  node_id n2(123,  "bfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf");
  node_id n3(1231, "000000000fbfbfbfbfbfbfbfbfbfbfbfbfbfbfbf");
  send_invidually(
    riac::node_info{
      n1,
      {{n1, 2, 2300}},
      "Sokrates",
      "Mac OS X",
      {{"en0", {{riac::protocol::ethernet,
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
      {{"wlan0", {{riac::protocol::ethernet,
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
      {{"en1", {{riac::protocol::ethernet,
                 {"00:00:FF:FF:00:00"}}}}}
    },
    riac::work_load{n3, 23, 20, 3},
    riac::ram_usage{n3, 1024, 8096}
  );
}

void shell::list_nodes(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  m_self->sync_send(m_nexus_proxy, atom("Nodes")).await(
    [=](std::vector<node_id>& nodes) {
      if(nodes.empty()) {
        cout << " no nodes avaliable" << endl;
      }
      for (auto& node : nodes) {
        m_self->sync_send(m_nexus_proxy, atom("NodeInfo"), node).await(
          [=](const riac::node_info& ni) {
            cout << to_string(ni.source_node) << " on " << ni.hostname
                 << endl;
          },
          on(atom("NoNodeInfo")) >> [=] {
            set_error("Unexpected error.");
          }
        );
      }
    }
  );
}

void shell::sleep(char_iter first, char_iter last) {
  if(first == last) {
    return;
  }
  int time = std::stoi(std::string(first, last));
  std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

void shell::whereami(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  cout << to_string(m_node) << endl;
}

void shell::change_node(char_iter first, char_iter last) {
  if (first == last) {
    return;
  }
  std::string input_str(first, last);
  auto input_node = from_string<node_id>(input_str);
  if (!input_node) {
    std::string unknown_hostname = "change-node: unkown hostname. ";
    std::string invalid_hostname = "change-node: invalid hostname. ";
    auto input_words = my_split(input_str, ':');
    if (2 < input_words.size() || input_words.size() < 1) {
      set_error(invalid_hostname);
      return;
    }
    uint32_t input_pid = 0;
    if (input_words.size() == 2) {
      try {
        input_pid = std::stoi(input_words.back());
      } catch(...) {
        set_error(invalid_hostname);
        return;
      }
    }
    m_self->sync_send(m_nexus_proxy, atom("OnHost"), input_words.front()).await(
      [=](const std::vector<node_id>& nodes_on_host) {
        if (nodes_on_host.size() == 0) {
          set_error(unknown_hostname);
        } else if (input_pid == 0) {
          if (nodes_on_host.size() == 1) {
            set_node(nodes_on_host.front());
          } else {
            std::stringstream es;
            es << "Valid process-ids for " << input_words.front() << ": "
               << endl;
            for(auto node : nodes_on_host) {
              es << node.process_id();
              if (node != nodes_on_host.back()) {
                es << ", " << endl;
              }
            }
            set_error(es.str());
          }
        } else {
          auto n = std::find_if(std::begin(nodes_on_host), std::end(nodes_on_host),
                       [=](const node_id& n) -> bool {
                         return n.process_id() == input_pid;
                       });
          if (n != std::end(nodes_on_host)) {
            set_node(*n);
          } else {
            set_error("change-node: unkown process-id.");
          }
        }
      }
    );
  } else {
    // check if valid node is known
    std::string unkown_id = "change-node: unknown node-id. ";
    m_self->sync_send(m_nexus_proxy, atom("HasNode"), *input_node).await(
      on(atom("Yes")) >> [=] {
        set_node(*input_node);
      },
      on(atom("No")) >> [&] {
        set_error(unkown_id);
      }
    );
  }
}

void shell::all_routes(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  // TODO: refactor with direct_conn
  m_self->sync_send(m_nexus_proxy, atom("Nodes")).await(
    [=](const std::vector<node_id>& nodes) {
      for (auto node : nodes) {
        m_self->sync_send(m_nexus_proxy, atom("Routes"), node).await(
          [=](const std::set<node_id>& dr) {
            cout << to_string(node) << " ->"
                      << endl;
            for (auto route : dr) {
              cout << to_string(route)
                   << endl;
            }
            cout << endl;
          }
        );
      }
    }
  );
}

void shell::leave_node(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  m_cli.mode_pop();
  cout << "Leaving node-mode" << endl;
  m_engine->unset("NODE");
}

void shell::work_load(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  m_self->sync_send(m_nexus_proxy, atom("WorkLoad"), m_node).await(
    [](const riac::work_load& wl) {
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
    on(atom("NoWorkLoad")) >> [] {
      cout << "No work load statistics available for node" << endl;
    }
  );
}

void shell::ram_usage(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  m_self->sync_send(m_nexus_proxy, atom("RamUsage"), m_node).await(
    [](const riac::ram_usage& ru) {
      auto used_ram_in_percent = (ru.in_use * 100.0) / ru.available;
      cout << "RAM: "
           << progressbar(static_cast<size_t>(used_ram_in_percent / 2), '#')
           << ru.in_use << "/" << ru.available
           << endl;
    },
    on(atom("NoRamUsage")) >> [] {
      cout << "No ram usage statistics available for node" << endl;
    }
  );
}

void shell::statistics(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  m_self->sync_send(m_nexus_proxy, atom("NodeInfo"), m_node).await(
    [=](const riac::node_info& ni) {
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
    on(atom("NoNodeInfo")) >> [] {
      cout << "No ram usage statistics available for node" << endl;
    }
  );
}

void shell::direct_conn(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  m_self->sync_send(m_nexus_proxy, atom("Routes"), m_node).await(
    [=](const std::set<node_id>& conn) {
      std::cout << to_string(m_node) << " ->"
                << std::endl;
      for (auto ni : conn) {
        std::cout << to_string(ni) << endl;
      }
      cout << endl;
    }
  );
}

void shell::interfaces(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  m_self->sync_send(m_nexus_proxy, atom("NodeInfo"), m_node).await(
    [=](const riac::node_info& ni) {
      using riac::protocol;
      auto tostr = [](protocol p) -> std::string {
        switch (p) {
          case protocol::ethernet:
            return "ethernet";
          case protocol::ipv4:
            return "ipv4";
          case protocol::ipv6:
            return "ipv6";
        }
        return "-invalid-";
      };
      const char* indent = "    ";
      for (auto& interface : ni.interfaces) {
        cout << interface.first << ":" << endl;
        for (auto& addresses : interface.second) {
          for (auto& address : addresses.second) {
            cout << indent << tostr(addresses.first)
                 << " " << address << endl;
          }
        }
      }
    },
    on(atom("NoNodeInfo")) >> [] {
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
  if (!msg) {
    set_error("cannot deserialize a message from given input");
    return;
  }
  m_self->sync_send(m_nexus_proxy, atom("GetActor"), m_node, aid).await(
    [&](const actor& handle) {
      if (handle == invalid_actor) {
        cout << "send: no actor known with ID " << aid << endl;
        return;
      }
      m_user->send(handle, std::move(*msg));
    }
  );
}

void shell::mailbox(char_iter first, char_iter last) {
  // TODO: implement me
  set_error("mailbox: not implemented yet");
}

void shell::dequeue(char_iter first, char_iter last) {
  // TODO: implement me
  set_error("dequeue: not implemented yet");
}

void shell::pop_front(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  m_user->receive(
    others() >> [&] {
      cout << to_string(m_user->last_dequeued()) << endl;
    },
    after(std::chrono::seconds(0)) >> [] {
      cout << "pop-front: mailbox is empty" << endl;
    }
  );
}

void shell::await_msg(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  m_user->receive(
    others() >> [&] {
      cout << to_string(m_user->last_dequeued()) << endl;
    }
  );
}

void shell::list_actors(char_iter first, char_iter last) {
  if (!assert_empty(first, last)) {
    return;
  }
  auto nid = m_node;
  actor self = m_self;
  auto mm = io::middleman::instance();
  mm->run_later([nid, self, mm] {
    auto bro = mm->get_named_broker<io::basp_broker>(atom("_BASP"));
    auto proxies = bro->get_namespace().get_all(nid);
    std::ostringstream oss;
    for (auto& p : proxies) {
      oss << p->id() << endl;
    }
    anon_send(self, atom("ListActors"), oss.str());
  });
  // wait for result
  m_self->sync_send(m_nexus_proxy, atom("ListActors"), m_node).await(
    [](const std::string& res) {
      if (res.empty()) {
        cout << "list-actors: no actors known on this host" << endl;
      }
      cout << res << flush;
    }
  );
}

void shell::set_node(node_id id) {
  auto node_str = to_string(id);
  m_engine->set("NODE", node_str);
  m_node = id;
  m_cli.mode_push("node");
}

void shell::test(char_iter first, char_iter last) {
  auto dudi = from_string<node_id>(std::string(first, last));
  //auto dudi = from_hostname(std::string(first,last));
  if (dudi) {
    auto blabla = to_hostname(*dudi);
    if (blabla) {
      cout << *blabla << endl;
    } else {
      set_error("NONO");
    }
  } else {
    set_error("sss");
  }
}

optional<node_id> shell::from_hostname(const std::string& input) {
  std::vector<std::string> hostname;
  caf::split(hostname, input, ":");
  // check valid format
  uint16_t input_size = hostname.size();
  if (input_size < 1 || 2 < input_size) {
    return none;
  }
  optional<node_id> ni = none;
  m_self->sync_send(m_nexus_proxy, atom("OnHost"), hostname.front()).await(
    [&](const std::vector<node_id>& nodes_on_host) {
      if (nodes_on_host.size() == 1 && input_size == 1) {
        ni = nodes_on_host.front();
      } else if (input_size == 2) {
        try {
          for (node_id node_on_host : nodes_on_host) {
              uint32_t process_id = std::stoi(hostname.back());
              if (node_on_host.process_id() == process_id) {
                ni = node_on_host;
                // TODO: remove break
                break;
              }
          }
        } catch (...) {
          ni = none;
        }
      } else {
        ni = none;
      }
    }
  );
  return ni;
}

optional<std::string> shell::to_hostname(const node_id& node) {
  if (node == invalid_node_id) {
    return none;
  }
  optional<std::string> hostname = none;
  m_self->sync_send(m_nexus_proxy, atom("Nodes")).await(
    [&](const std::vector<node_id>& nodes) {
        m_self->sync_send(m_nexus_proxy, atom("NodeInfo"), node).await(
          [&](const riac::node_info& ni) {
            std::string tmp_hostname = ni.hostname;
            //hostname = ni.hostname;
            if (nodes.size() > 1) {
              tmp_hostname += ":" + ni.source_node.process_id();
            }
            hostname = tmp_hostname;
          }
        );
    }
  );
  return hostname;
}

} // namespace cash
} // namespace caf
