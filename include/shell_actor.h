#ifndef SHELL_ACTOR_H
#define SHELL_ACTOR_H

#include <vector>

#include "shell_actor.h"
#include "probe_event.hpp"

#include "cppa/cppa.hpp"

struct node_data {
    caf::probe_event::node_info node_info;
    caf::probe_event::cpu_info  cpu_info;
    caf::probe_event::work_load work_load;
    caf::probe_event::ram_usage ram_usage;
};

class shell_actor : public cppa::event_based_actor {

public:
  shell_actor();
  cppa::behavior make_behavior() override;

private:
  std::map<cppa::node_id, node_data>  m_nodes;

};

#endif // SHELL_ACTOR_H
