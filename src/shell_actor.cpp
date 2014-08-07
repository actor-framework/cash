#include "caf/shell/shell_actor.hpp"


namespace caf {

using namespace std;
using namespace probe_event;

namespace shell {

shell_actor::shell_actor() {
  // nop
}

bool shell_actor::is_known(const node_id& id) {
  return m_known_nodes.count(id) != 0;
}

/**
 * @brief shell_actor::add
 * @param ni - node_id
 * @return true when node_id wasn't known and has been added.
 */
bool shell_actor::add(const node_info& ni) {
  if (is_known(ni.id)) {
    return false;
  }
  node_data nd;
  nd.node_info = ni;
  m_known_nodes.emplace(ni.id, nd);
  return true;
}

/**
 * @brief shell_actor::add
 * @param wl - work_load
 * @return true when node_id wasn't known and false when work_load drops.
 */
bool shell_actor::add(const node_id& id, const work_load& wl) {
  auto kvp = m_known_nodes.find(id);
  if (kvp == m_known_nodes.end()) {
    return false;
  }
  auto nd = kvp->second;
  nd.work_load = wl;
  m_known_nodes.emplace(id, nd);
  return true;
}

/**
 * @brief shell_actor::add
 * @param ru - ram_usage
 * @return true when node_id wasn't known and has been added.
 */
bool shell_actor::add(const node_id& id, const ram_usage& ru) {
  auto kvp = m_known_nodes.find(id);
  if (kvp == m_known_nodes.end()) {
    return false;
  }
  auto nd = kvp->second;
  nd.ram_usage = ru;
  m_known_nodes.emplace(id, nd);
  return true;
}

behavior shell_actor::make_behavior() {
  return {
    [=](const node_id& from, const probe_event::new_message& ) {
      aout(this) << "new message" << endl;
    },
    [=](const node_id& from, const probe_event::new_route& nr) {
      aout(this) << "new message" << endl;
    },
    [=](const node_id& from, const probe_event::node_info& ni) {
      add(ni);
    },
    [=](const node_id& from, const probe_event::work_load& wl) {
      add(from, wl);
    },
    [=](const node_id& from, const probe_event::ram_usage& ru) {
      add(from, ru);
    },
    // communication with shell
    on(atom("quit")) >> [=] {
      quit();
      return make_message(atom("done"));
    },
    on(atom("add-test"), arg_match) >> [&](node_id id, node_data data) {
      m_known_nodes.emplace(id, data);
    },
    on(atom("list-nodes")) >> [&]() {
      if (m_known_nodes.empty()) {
        aout(this) << "--- no nodes available ---"
                   << endl;
      } else {
        for (const auto& kvp : m_known_nodes) {
          cout << setw(55)  << to_string(kvp.first)
                     << " - "          << kvp.second.node_info.hostname
                     << endl;
        }
      }
      return make_message(atom("done"));
    },
    on(atom("changenode"), arg_match) >> [&](node_id input_node) {
      if (m_known_nodes.empty()) {
        return make_message(atom("cnfail"), string("No nodes known."));
      } else if (m_known_nodes.count(input_node) == 0) {
        return make_message(atom("cnfail"), string("Given node is unkown."));
      } else {
        if (m_visited_nodes.back() != input_node) {
          m_visited_nodes.push_back(input_node);
        }
        return make_message(atom("cncorrect"), input_node);
      }
    },
    on(atom("whereami")) >> [=] {
      if (m_visited_nodes.empty()) {
        return make_message(atom("waifail"), "You are currently in globalmode. "
                                             "You can select a node "
                                             "with 'change-node <node_id>'.");
      } else {
        return make_message(atom("waicorrect"),
                            to_string(m_visited_nodes.back()));
      }
    },
    on(atom("nodedata")) >> [&] {
      auto search = m_known_nodes.find(m_visited_nodes.back());
      if (search != m_known_nodes.end()) {
        auto nd = search->second;
        auto ni = nd.node_info;
        auto wl = nd.work_load;
        auto ru = nd.ram_usage;
        return make_message(atom("ndcorrect"), ni, wl, ru);
      } else {
        return make_message(atom("ndfail"), "Node not found");
      }
    },
    on(atom("leave-node")) >> [&] {
      m_visited_nodes = list<node_id>();
      return make_message(atom("done"));
    },
    on(atom("back")) >> [&]() {
      if (m_visited_nodes.size() <= 1) {
        m_visited_nodes = list<node_id>();
        return make_message(atom("leave"));
      } else {
        m_visited_nodes.pop_back();
        return make_message(atom("done"), m_visited_nodes.back());
      }
    },
    others() >> [=] {
      aout(this) << "Received from sender: "
                 << to_string(last_sender())
                 << endl << "an unexpected message. "
                 << to_string(last_dequeued())
                 << endl << endl;
    }
  };
}

} // namespace shell
} // namespace caf
