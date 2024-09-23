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

    // Print params for testing
    std::cout << "Parameters passed: \n" << "Input filename: " << inputfile << "\nOutput filename: " << outputfile << "\nOperation: " << operation << std::endl; 
    
    Lexer* lexer = new Lexer();


    lexer->open_file(const_cast<char*>(inputfile.c_str()));


    std::cout << "File size: " << static_cast<int>(lexer->getFileSize()) << std::endl;
    
    lexer->read_next_instruction();
    return 0;
}	