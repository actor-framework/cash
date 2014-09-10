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

#ifndef CAF_SHELL_NEXUS_PROXY_HPP
#define CAF_SHELL_NEXUS_PROXY_HPP

#include <vector>

#include "caf/all.hpp"
#include "caf/riac/all.hpp"

namespace caf {
namespace shell {

class nexus_proxy : public event_based_actor {
 protected:
  behavior make_behavior() override;

 private:
  riac::probe_data_map  m_data;
  std::list<node_id> m_visited_nodes;
};

} // namespace shell
} // namespace caf

#endif // CAF_SHELL_NEXUS_PROXY_HPP
