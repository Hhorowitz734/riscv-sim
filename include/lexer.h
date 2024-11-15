#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <fstream>
#include <bitset>
#include <string>
#include <instruction.h>


class Lexer {

public:

    // Utility functions
    void set_input_file(const char* filename);
    void set_output_file(const char* filename);
    void write_output(std::string output);
    bool isEOF(); 

    // Reading instructions
    Dword consume_instruction(); // Main logic for reading an instruction through (handles reader aspect)
    void reset_instruction(); // Resets the "curr instruction" class variable
    Instruction read_next_instruction(); // Uses the opcode generated by consume_instruction to read over the instruction and create the curr_instruction variable

    

    std::size_t getFileSize() const;

private:

    std::ifstream inputFile;
    std::ofstream outputFile;

    std::size_t fileSize; //Byte size of file
    std::size_t bitsConsumed = 0;
    int instructions_consumed = 0;

    Instruction curr_instruction;

    const int start_position = 496;

};



#endif