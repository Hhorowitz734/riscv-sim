#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <fstream>
#include <bitset>
#include <string>
#include <instruction.h>


class Lexer {

public:

    void open_file(char* filename) {
        /*
        * Attempts to open useor provided filename
        */

        inputFile.open(filename, std::ios::in);

        // Handle file open failed
        if (!inputFile.is_open()) {
            std::cerr << "Error: File [" << filename << "] could not be opened." << std::endl;
        }

        // Sets static file size variable
        inputFile.seekg(0, std::ios::end); // Set pointer to end
        fileSize = static_cast<std::size_t>(inputFile.tellg());
        inputFile.seekg(0, std::ios::beg); // Skip back to beginning

    }


    Dword consume_instruction() {
        /*
        * Reads next byte
        */

        // Skip newline characters
        char nextChar = inputFile.peek();
        while (nextChar == '\n' || nextChar == ' ') {
            inputFile.seekg(1, std::ios::cur);
            bytesConsumed++;
            nextChar = inputFile.peek();
        }

        // Handle case that no bytes remain
        if (fileSize - bytesConsumed < 32) {
            std::cerr << "No bytes left to read!" << std::endl; 
            return 0;
        }

        // Buffer to read in byte
        char buff[32];

        // Consume and update counter
        inputFile.read(buff, 32);
        bytesConsumed += 32;

        // Convert string buffer to 32-bit instruction
        Dword instruction = 0;

        for (int i = 0; i < 32; i++) {
            switch (buff[i]) {
                case '0':
                    instruction = instruction << 1;
                    break;
                case '1':
                    instruction = (instruction << 1) | 0x1;
                    break;
                default:
                    std::cerr << "Faulty byte found in text: " << buff[i] << std::endl;
            }
        }

        return instruction;
    }

    void reset_instruction() {
        /*
        * Resets currently used instruction to read in a new one
        */

        // Create new instruction with default values
        curr_instruction.type = ERROR_TYPE;
        curr_instruction.value = 0;

    }

    void read_next_instruction() { 

        // Reset
        reset_instruction();

        // Get next instruction value
        Dword instruction_val = consume_instruction();
        curr_instruction.value = instruction_val;

        // Get opcode
        I_TYPE opcode = read_opcode(instruction_val);
        curr_instruction.type = opcode;

        std::cout << itype_to_string(opcode) << std::endl;
    }

    

    std::size_t getFileSize() const { return fileSize; }

private:

    std::ifstream inputFile;

    std::size_t fileSize; //Byte size of file
    std::size_t bytesConsumed = 0;

    Instruction curr_instruction;

};



#endif