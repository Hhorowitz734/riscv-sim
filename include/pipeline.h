#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector> 
#include <unordered_map>

#include "instruction.h"
#include "pipelinestage.h"

class Pipeline {

public:

    Pipeline() {
        // Initialize the 8 pipeline stages
        stages[StageType::IF] = PipelineStage(StageType::IF);
        stages[StageType::IS] = PipelineStage(StageType::IS);
        stages[StageType::RF] = PipelineStage(StageType::RF);
        stages[StageType::EX] = PipelineStage(StageType::EX);
        stages[StageType::DF] = PipelineStage(StageType::DF);
        stages[StageType::DS] = PipelineStage(StageType::DS);
        stages[StageType::TC] = PipelineStage(StageType::TC);
        stages[StageType::WB] = PipelineStage(StageType::WB);

        // Initialize the 32 integer registers
        for (int i = 0; i < 32; ++i) {
            integer_registers["R" + std::to_string(i)] = 0;
        }
    }

    void sendNextInstruction() {
        /**
         * Takes an instruction from "instructions" vector
         * Sends it to IF pipeline stage
         */

        if (stages[StageType::IF].isEmpty()) {
            stages[StageType::IF].setInstruction(std::make_unique<Instruction>(instructions[instruction_index]));
            instruction_index++;
        } else {
            std::cerr << "Error: IF stage is full. Stall required.\n";
        }

    }

    std::string getPipelineStatusOutput() {
        std::string output;
        output += "Pipeline Status: \n";

        // Iterate through the stages and append their state to the output
        output += stages[StageType::IF].getState();
        output += stages[StageType::IS].getState();
        output += stages[StageType::RF].getState();
        output += stages[StageType::EX].getState();
        output += stages[StageType::DF].getState();
        output += stages[StageType::DS].getState();
        output += stages[StageType::TC].getState();
        output += stages[StageType::WB].getState();

        return output;
    }

    std::string getIntegerRegistersOutput() {
        /**
         * Returns string of integer register results
         */

        std::string output = "Integer registers:\n";

        // Iterate over register indices from 0 to 31
        for (int i = 0; i < 32; ++i) {
            output += "R" + std::to_string(i) + "\t" + std::to_string(integer_registers["R" + std::to_string(i)]) + "\t";

            // Add a newline after every 4 registers
            if ((i + 1) % 4 == 0) {
                output += "\n";
            }
        }

        return output;
    }



    void addInstruction(Instruction instruction) {
        /*
        * Takes in an instruction from Lexer and adds it to pipeline
        */

       instructions.push_back(instruction);
    }



private:

    // instruction_index represents the index of the next instruction to be sent
    int instruction_index = 0;

    std::vector<Instruction> instructions; // Raw instructions recieved from lexer

    std::unordered_map<StageType, PipelineStage> stages;  // The 8 pipeline stages

    std::unordered_map<std::string, int> integer_registers; // Integer registers

};




#endif