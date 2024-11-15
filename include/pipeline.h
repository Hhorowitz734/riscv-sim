#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector> 
#include "instruction.h"

class Pipeline {

public:

    void addInstruction(Instruction instruction) {
        /*
        * Takes in an instruction from Lexer and adds it to pipeline
        */

       instructions.push_back(instruction);

       std::string s = instruction_to_string(instruction, 0, false);
       std::cout << s << std::endl;
    }




private:

    std::vector<Instruction> instructions;

};




#endif