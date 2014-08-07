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
