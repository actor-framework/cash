#include "caf/shell/shell_actor.hpp"

namespace caf {
namespace shell {

shell_actor::shell_actor() {
  // nop
}

behavior shell_actor::make_behavior() {
  return {
    on(atom("cn")) >> []() {

    }
  };
}

} // namespace shell
} // namespace caf
