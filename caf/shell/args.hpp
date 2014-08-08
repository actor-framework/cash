#ifndef CAF_SHELL_ARGS_H
#define CAF_SHELL_ARGS_H

struct net_config {
  uint16_t port;
  std::string host;
  inline net_config() : port(0) { }
  inline bool valid() const {
    return port != 0 && !host.empty();
  }
};

const char host_arg[] = "--caf_nexus_host=";
const char port_arg[] = "--caf_nexus_port=";

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

#endif // CAF_SHELL_ARGS_H
