#ifndef SHELL_HPP
#define SHELL_HPP

#include "actorshell.hpp"

namespace actorshell {

class shell {

    std::string                 m_prompt;
    std::vector<std::string>    m_commands;

 public:

    shell(std::string prompt) {
        shell::m_prompt = prompt;
        init_rl_shell();

        m_commands = {"Cat",
                      "Dog",
                      "Horse",
                      "Dolphin"};
    }

    void        init_rl_shell();
    char**      auto_completion(const char* input, int start, int);
    char*       command_generator(const char* to_complete, int state);
    std::string get_prompt();


};

} // namespace actorshell

#endif // SHELL_HPP
