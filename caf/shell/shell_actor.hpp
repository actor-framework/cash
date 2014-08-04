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
  behavior make_behavior() override;

 private:
  std::map<node_id, node_data>  m_nodes;
};

} // namespace shell
} // namespace caf

#endif // CAF_SHELL_SHELL_ACTOR_HPP
