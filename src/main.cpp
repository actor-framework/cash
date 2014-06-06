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



#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include "actorshell/actorshell.hpp"

using namespace std;
using namespace actorshell;


//extern  int    history_length;

vector<string>  commands = {  "Dog",
                               "Duck",
                        "Cat",
                        "Horse",
                        "Wolf"};

void init_shell();
char **completion(const char* input, int start, int);
char *command_generator(const char* to_complete, int state);






int main(int argc, char** argv) {

    shell        actor_shell(" > ");



    //rl_completion_entry_function    = command_generator; // Autocompletion
    //rl_initialize();

    init_shell();


    for (;;) {

        std::unique_ptr<char, void (*)(void*)> input {
            readline(actor_shell.get_prompt().c_str()), free
        };



        add_history(input.get());
        if (!input) break;

        /*
        if (binary_search(commands.begin, commands.end, string(input.get()))) {
           // execute_command(input.get());
            cout << "Size of History: " << history_length << endl;
        } else {
            cerr << "Invalid command: " << input.get() << endl;
            cerr << "Type 'help' for commands" << endl;
        }
        */
        input.get();
    }
}

char * dupstr (const char* s) {
   char *r;

   r = reinterpret_cast<char*>(malloc (strlen (s) + 1));
   strcpy (r, s);
   return (r);
}

void init_shell() {
    //rl_readline_name = " > ";
    rl_attempted_completion_function = completion;
}

char **completion(const char* input, int start, int) {
    char** matches;

    matches = (char**) NULL;

    if (start == 0) {
       matches = completion_matches(input, command_generator);
    }

    return matches;
}

char *command_generator(const char* to_complete, int state) {
    static int list_index, len;
    const char* name;

    if (state == 0) {
        list_index = 0;
        len = strlen(to_complete);
    }

    // Return matching word(s)
    while (commands.size() > list_index) {
        name = commands[list_index].c_str();
        list_index++;

        if (strncmp(to_complete, name, len) == 0) {
            return dupstr(name);
        }
    }

    // When nothing matches
    return nullptr;
}

