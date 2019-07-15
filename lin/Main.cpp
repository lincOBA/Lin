#include <string.h>
#include <iostream>
#include "Interpreter.h"
#include "Utils.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        panic("Feed your *.lin source file to interpreter!\n");
    }

    Interpreter lin(argv[1]);
    lin.execute();
//  Parser::printLex(argv[1]);
    return 0;
}
