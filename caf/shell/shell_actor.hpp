#ifndef CAF_SHELL_SHELL_ACTOR_HPP
#define CAF_SHELL_SHELL_ACTOR_HPP

#include <vector>

#include "caf/all.hpp"
#include "caf/probe_event/all.hpp"

namespace caf {
namespace shell {

struct node_data {
    probe_event::node_info node_info;
    probe_event::work_load work_load;
    probe_event::ram_usage ram_usage;
};

inline bool operator ==(const node_data& lhs, const node_data& rhs) {
  return (lhs.node_info == rhs.node_info);
}

/*
struct get_all_nodes {};
struct get_node{ node_id id; };

using shell_actor_t = probe_event::sink::extend<
                        replies_to<get_all_nodes>::with<std::vector<node_data>>,
                        replies_to<get_node>::with<optional<node_data>>
                      >::type;
using shell_actor = probe_event::sink::extend<reacts_to<get_all_nodes>,
                                              reacts_to<get_node>,
                                              reacts_to<probe_event::
*/
class shell_actor : public event_based_actor {
 public:
  shell_actor();
  bool is_known(const node_id& id);
  bool add(const probe_event::node_info& ni);
  bool add(const node_id& id, const probe_event::work_load& ni);
  bool add(const node_id& id, const probe_event::ram_usage& ni);
  behavior make_behavior() override;

 private:
  std::map<node_id, node_data>  m_known_nodes;
  std::list<node_id>            m_visited_nodes;
};

} // namespace shell
} // namespace caf

#endif // CAF_SHELL_SHELL_ACTOR_HPP
