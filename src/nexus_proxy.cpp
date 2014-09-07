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

#include "caf/shell/nexus_proxy.hpp"

namespace caf {
namespace shell {

behavior nexus_proxy::make_behavior() {
  return {
    // nexus communication
    [=](const probe_event::new_message& ) {
      //aout(this) << "new message" << endl;
    },
    [=](const probe_event::new_route& nr) {
      //aout(this) << "new message" << endl;
    },
    [=](probe_event::node_info& ni) {
      m_known_nodes[ni.source_node].node = std::move(ni);
    },
    [=](probe_event::work_load& wl) {
      m_known_nodes[wl.source_node].load = std::move(wl);
    },
    [=](probe_event::ram_usage& ru) {
      m_known_nodes[ru.source_node].ram = std::move(ru);
    },
    on(atom("Nodes")) >> [=]() -> std::vector<node_id> {
      std::vector<node_id> result;
      result.reserve(m_known_nodes.size());
      for (auto& kvp : m_known_nodes) {
        result.push_back(kvp.first);
      }
      return result;
    },
    on(atom("NodeInfo"), arg_match) >> [=](const node_id& nid) -> message {
      auto i = m_known_nodes.find(nid);
      if (i == m_known_nodes.end()) {
        return make_message(atom("NoNodeInfo"));
      }
      return make_message(i->second.node);
    },
    on(atom("WorkLoad"), arg_match) >> [=](const node_id& nid) -> message {
      auto i = m_known_nodes.find(nid);
      if (i == m_known_nodes.end() || !i->second.load) {
        return make_message(atom("NoWorkLoad"));
      }
      return make_message(*(i->second.load));
    },
    on(atom("RamUsage"), arg_match) >> [=](const node_id& nid) -> message {
      auto i = m_known_nodes.find(nid);
      if (i == m_known_nodes.end() || !i->second.ram) {
        return make_message(atom("NoRamUsage"));
      }
      return make_message(*(i->second.ram));
    },
    others() >> [=] {
      aout(this) << "Received from sender: "
                 << to_string(last_sender())
                 << std::endl << "an unexpected message. "
                 << to_string(last_dequeued())
                 << std::endl << std::endl;
    }
  };
}

} // namespace shell
} // namespace caf
