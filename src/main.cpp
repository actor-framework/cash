/******************************************************************************\
 *                                                                            *
 *   ____         __                  ____    __              ___    ___      *
 *  / _  \       /\ \__              /\  _`\ /\ \            /\_ \  /\_ \     *
 * /\ \L\ \    __\ \ ,_\   ___   _ __\ \,\L\_\ \ \___      __\//\ \ \//\ \    *
 * \ \  __ \  /'__\ \ \/  / __`\/\`'__\/_\__ \\ \  _ `\  /'__`\\ \ \  \ \ \   *
 *  \ \ \/\ \/\ __/\ \ \_/\ \L\ \ \ \/  /\ \L\ \ \ \ \ \/\  __/ \_\ \_ \_\ \  *
 *   \ \_\ \_\ \___\\ \__\ \____/\ \_\  \ `\____\ \_\ \_\ \____\/\____\/\___\ *
 *    \/_/\/_/\/___/ \/__/\/___/  \/_/   \/_____/\/_/\/_/\/____/\/____/\/___/ *
 *                                                                            *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 * Alex    Mantel     <alex.mantel        (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the Boost Software License, Version 1.0. See             *
 * accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt  *
\******************************************************************************/


#include <iostream>
#include <string>
#include <memory>

#include "editline/readline.h"

#include "actorshell/actorshell.hpp"

using namespace std;
using namespace actorshell;

bool is_in(string element, const set<string>& Set) {
    return Set.find(element) != Set.end();
}


bool print_set(set<string> Set) {
    cout << "These commands are available: " << endl;
    for(auto value : Set) {
        cout << value << endl;
    }
}

char* current_prompt() {
    return "actor-shell> ";
}

int main(int argc, char** argv) {

    // init()
    set<string> commands    = { "lp", "cp", "la", "lc", "help" };

    //actorshell shell    = actorshell();
    // arguments -argc argv


    for (;;) {

        std::unique_ptr<char, void (*)(void*)> input {
            readline(current_prompt()), free
        };


        add_history(input.get());
        if (!input) break;

        if (is_in(input.get(), commands)) {
            //execute_command(inputd.get());
        } else {
            cerr << "Invalid command: " << input.get() << endl;
            cerr << "Type 'help' for commands" << endl;
        }
        //readline(current_prompt());
        input.get();
    }

}
