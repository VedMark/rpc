#include <iostream>
#include <fstream>
#include "parser.h"

int main(int argc, char **argv) {
    if(argc != 2){
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    try{
        std::string fName(argv[1]);
        auto generator = StubGenerator();
        generator.parse(fName);
    }
    catch (StubGenerator::ParseException){
        std::cerr << "Incorrect syntax. Terminate parsing." << std::endl;
    }

    return 0;
}