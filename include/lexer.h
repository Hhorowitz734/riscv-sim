#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <fstream>
#include <instruction.h>


class Lexer {

public:

    void open_file(char* filename) {
        /*
        * Attempts to open useor provided filename
        */

        inputFile.open(filename, std::ios::in | std::ios::binary);

        // Handle file open failed
        if (!inputFile.is_open()) {
            std::cerr << "Error: File [" << filename << "] could not be opened." << std::endl;
        }

        // Sets static file size variable
        inputFile.seekg(0, std::ios::end); // Set pointer to end
        fileSize = static_cast<std::size_t>(inputFile.tellg());
        inputFile.seekg(0, std::ios::beg); // Skip back to beginning

    }
    
    Dword consume() {
        /*
        * Reads next 4 bytes [1 instruction] from input, updates position accordingly
        */

        // Handle case that <4 bytes remain
        if (fileSize - bytesConsumed < 4) {
            std::cerr << "[" << fileSize - bytesConsumed  << "] bytes remain, which is less than 4." << std::endl; 
            return 0;
        }

        // Instruction buffer
        char buff[4];

        // Perform read
        inputFile.read(buff, 4);
        bytesConsumed += 4;

        // Convert buffer to Dword
        Dword instruction;

        for (int i = 0; i < 4; i++) {
            // Converts buffer into 4 byte instruction using |= bitmask
            instruction |= (static_cast<Dword>(buff[i]) << (i * 8));
        }

        return instruction;







        



       
    }

    std::size_t getFileSize() const { return fileSize; }

private:

    std::ifstream inputFile;

    std::size_t fileSize; //Byte size of file
    std::size_t bytesConsumed = 0;

};



#endif