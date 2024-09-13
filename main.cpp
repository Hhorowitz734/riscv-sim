#include <iostream>
#include <string>

#include "include/lexer.h"

int main(int argc, char* argv[]) { 
    
    Lexer* lexer = new Lexer();

    std::string filename = "../test.txt";

    lexer->open_file(const_cast<char*>(filename.c_str()));


    std::cout << "File size: " << static_cast<int>(lexer->getFileSize()) << std::endl;
    
    lexer->read_next_instruction();
    return 0;
}	