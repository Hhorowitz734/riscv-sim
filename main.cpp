#include <iostream>

#include "include/lexer.h"

int main(int argc, char* argv[]) { 
    
    Lexer* lexer = new Lexer();

    lexer->open_file("../test.txt");

    std::cout << "File size: " << static_cast<int>(lexer->getFileSize()) << std::endl;
    std::cout << "Hex test: " << std::hex << lexer->consume() << std::endl;
    return 0;
}	