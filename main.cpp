#include <iostream>
#include <string>
#include <stdlib.h>

#include "include/lexer.h"

int main(int argc, char* argv[]) { 

    // Good command to run this: ./riscv-sim ../test.txt Hi Hi


    // Not enough params
    if (argc < 3) {
        std::cerr << "Please pass all required parameters: \n      --Inputfilename \n      --Outputfilename \n      --Operation" << std::endl;
        exit(1);
    } 

    // Process params
    std::string inputfile = argv[1];
    std::string outputfile = argv[2];
    std::string operation = argv[3];

    // Force dis flag
    if (std::string(argv[3]) != "dis") {
        std::cerr << "Operation must be 'dis'." << std::endl;
        std::cerr << "Please pass all required parameters: \n      --Inputfilename \n      --Outputfilename \n      --Operation" << std::endl;
        exit(1);
    }


    // Print params for testing
    //std::cout << "Parameters passed: \n" << "Input filename: " << inputfile << "\nOutput filename: " << outputfile << "\nOperation: " << operation << std::endl; 
    
    Lexer* lexer = new Lexer();


    lexer->set_input_file(const_cast<char*>(inputfile.c_str()));
    lexer->set_output_file(const_cast<char*>(outputfile.c_str()));

    //std::cout << "File size: " << static_cast<int>(lexer->getFileSize()) << std::endl;
    
    while (!lexer->isEOF()) {
        lexer->read_next_instruction();
    }

    return 0;
}	