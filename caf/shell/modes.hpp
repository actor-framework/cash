#ifndef CAF_SHELL_MODES_H
#define CAF_SHELL_MODES_H

#include "caf/shell/modes.hpp"

#include <vector>
#include <string>

#include "sash/sash.hpp"
#include "sash/libedit_backend.hpp"

using cli_type = sash::sash<sash::libedit_backend>::type;
/*
 * Global
 */
 std::vector<cli_type> global_cmd();

/*
 * Node
 */

#endif // CAF_SHELL_MODES_H
