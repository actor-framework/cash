#include "include/shell_actor.h"

using namespace cppa;

shell_actor::shell_actor() {

}


shell_actor::make_behavior() override {
  return {
    on(atom("cn")) >> [] () {

    }
  };
}
