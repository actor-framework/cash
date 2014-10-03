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

#ifndef CAF_SHELL_SHELL_HPP
#define CAF_SHELL_SHELL_HPP

#include <string>

#include "caf/optional.hpp"
#include "caf/scoped_actor.hpp"

#include "caf/riac/all.hpp"

#include "sash/sash.hpp"
#include "sash/libedit_backend.hpp"
#include "sash/variables_engine.hpp"

namespace caf {
namespace cash {

class shell {
 public:
  using char_iter = std::string::const_iterator;
  using cli_type = sash::sash<sash::libedit_backend>::type;

  shell();

  void run(riac::nexus_type nexus);

 private:

  // global commands

  void quit(char_iter first, char_iter last);

  void echo(char_iter first, char_iter last);

  void clear(char_iter, char_iter);

  void help(char_iter first, char_iter last);

  void change_node(char_iter first, char_iter last);

  void test_nodes(char_iter first, char_iter last);

  void list_nodes(char_iter first, char_iter last);

  void sleep(char_iter first, char_iter last);

  void await_msg(char_iter first, char_iter last);

  void mailbox(char_iter first, char_iter last);

  void dequeue(char_iter first, char_iter last);

  void pop_front(char_iter first, char_iter last);

  void all_routes(char_iter first, char_iter last);

  //TODO: remove this command
  void test(char_iter first, char_iter last);

  // Node commands

  void whereami(char_iter first, char_iter last);

  void leave_node(char_iter first, char_iter last);

  void work_load(char_iter first, char_iter last);

  void ram_usage(char_iter first, char_iter last);

  void statistics(char_iter first, char_iter last);

  void interfaces(char_iter first, char_iter last);

  void send(char_iter first, char_iter last);

  void list_actors(char_iter first, char_iter last);

  void direct_conn(char_iter first, char_iter last);

  // no shell commands

  void set_node(node_id id);

  optional<node_id> from_hostname(const std::string& node);

  optional<std::string> to_hostname(const node_id& ni);

  inline std::function<sash::command_result (std::string&, char_iter, char_iter)>
  cb(void (shell::*memfun)(char_iter, char_iter)) {
    return [=](std::string& err, char_iter first, char_iter last) -> sash::command_result {
      (*this.*memfun)(first, last);
      if (!err.empty()) {
        return sash::no_command;
      }
      return sash::executed;
    };
  }

  inline void send_invidually() {
    // end of recursion
  }

  template <class T, class... Ts>
  void send_invidually(T&& arg, Ts&&... args) {
    anon_send(m_nexus_proxy, std::forward<T>(arg));
    send_invidually(std::forward<Ts>(args)...);
  }

  inline bool assert_empty(char_iter first, char_iter last) {
    if (first != last) {
      set_error("to many arguments (none expected)");
      return false;;
    }
    return true;
  }

  template<typename T>
  void set_error(T&& str) {
    m_cli.set_error(std::forward<T>(str));
  }

  bool m_done;
  node_id m_node;
  cli_type m_cli;
  scoped_actor m_self;
  scoped_actor m_user;
  actor m_nexus_proxy;
  std::shared_ptr<sash::variables_engine<>> m_engine;
};

} // namespace cash
} // namespace caf

#endif // CAF_SHELL_SHELL_HPP
