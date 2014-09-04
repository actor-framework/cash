
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

#ifndef CAF_SHELL_ARGS_H
#define CAF_SHELL_ARGS_H

namespace caf {
namespace args {

struct net_config {
  uint16_t port;
  std::string host;
  inline net_config() : port(0) { }
  inline bool valid() const {
    return port != 0 && !host.empty();
  }
};

constexpr char host_arg[] = "--caf-nexus-host=";
constexpr char port_arg[] = "--caf-nexus-port=";

template<size_t Size>
bool is_substr(const char (&needle)[Size], const char* haystack) {
  // compare without null terminator
  if (strncmp(needle, haystack, Size - 1) == 0) {
    return true;
  }
  return false;
}

template<size_t Size>
size_t cstr_len(const char (&)[Size]) {
  return Size - 1;
}

void from_args(net_config& conf, int argc, char** argv) {
  for (auto i = argv; i != argv + argc; ++i) {
    if (is_substr(host_arg, *i)) {
      conf.host.assign(*i + cstr_len(host_arg));
    } else if (is_substr(port_arg, *i)) {
      int p = std::stoi(*i + cstr_len(port_arg));
      conf.port = static_cast<uint16_t>(p);
    }
  }
}

void print_help() {
  std::cerr << "Invalid arguments. Use:"
       << std::endl << host_arg
       << std::endl << port_arg
       << std::endl;
}

} // namespace args
} // namespace caf

#endif // CAF_SHELL_ARGS_H
