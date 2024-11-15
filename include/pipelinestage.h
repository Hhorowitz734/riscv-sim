#ifndef PIPELINE_STAGE_H
#define PIPELINE_STAGE_H

#include "instruction.h"
#include <memory>

enum StageType {
    IF, // Instruction Fetch (1/2)
    IS, // Instruction Fetch (2/2)
    RF, // Register Fetch
    EX, // Execution
    DF, // Data Fetch (1/2)
    DS, // Data Fetch (2/2)
    TC, // Tag Check
    WB  // Write Back
};

class PipelineStage {

public: 

    PipelineStage() : type(StageType::IF) {} // Default to IF or an invalid state
    PipelineStage(StageType type) : type(type) {};


    // Instruction management

    void setInstruction(std::unique_ptr<Instruction> instr) { curr_instruction = std::move(instr); }
    std::unique_ptr<Instruction> clearInstruction() { return std::move(curr_instruction); }
    bool isEmpty() const { return curr_instruction == nullptr; }

    
    // Getter
    StageType getStageType() const { return type; }

    // Utility functions
    std::string getStageName() const {
        switch (type) {
            case IF: return "IF";
            case IS: return "IS";
            case RF: return "RF";
            case EX: return "EX";
            case DF: return "DF";
            case DS: return "DS";
            case TC: return "TC";
            case WB: return "WB";
            default: return "Unknown";
        }
    }

    // Print stage state
    std::string getState() const {
        std::string output;

        // Append stage name
        output += getStageName();
        output += ": ";

        // If IF stage, print <unknown>
        if (type == IF) {
            output += "<unknown>\n";
            return output;
        }

        // Append instruction or NOP
        if (curr_instruction) {
            output += instruction_to_string(*curr_instruction, 0, false) + "\n";
        } else {
            output += "NOP\n";
        }

        return output;
    }



private:

    StageType type;
    std::unique_ptr<Instruction> curr_instruction;


};

#endif