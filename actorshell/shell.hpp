#ifndef SHELL_HPP
#define SHELL_HPP

#include "actorshell.hpp"

class shell {

    std::string     _prompt;

 public:

    shell(std::string prompt) {
        _prompt = prompt;
    }



    bool        auto_complete();
    //std::string get_prompt();
    std::string get_prompt() {
        return _prompt;
    }

};

#endif // SHELL_HPP
