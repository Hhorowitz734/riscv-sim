#ifndef PIPELINE_STAGE_H
#define PIPELINE_STAGE_H

#include "instruction.h"
#include <memory>
#include <vector>
#include <regex>


enum StageType {
    IF, // Instruction Fetch (1/2)
    IS, // Instruction Fetch (2/2)
    ID,
    RF, // Register Fetch
    EX, // Execution
    DF, // Data Fetch (1/2)
    DS, // Data Fetch (2/2)
    WB  // Write Back
};

class PipelineStage {

public: 

    PipelineStage(); // Default to IF or an invalid state
    PipelineStage(StageType type);


    // Instruction management

    void setInstruction(std::unique_ptr<Instruction> instr);
    std::unique_ptr<Instruction> clearInstruction();
    bool isEmpty() const;

    
    // Getters NEED TO UPDATE THESE FOR MEMORY SAFETY
    StageType getStageType() const;
    std::vector<uint32_t> getDependencies() const; // Protect these from segfaults
    uint32_t getDestination() const;
    int32_t getImmediate() const;
    EXACT_INSTRUCTION getExactInstruction() const;
    uint32_t getValue() const;
    void deallocateInstruction();
    INST_TYPE getInstructionType();

    // DO NOT USE THIS EXCEPT TO MOVE !!!
    std::unique_ptr<Instruction>& getInstruction(); // !!!
    // DO NOT USE THIS EXCEPT TO MOVE!! 

    void setResult(int32_t newResult); // for setting the result of a computation in ex stage
    int32_t getResult() const;

    // for seetting memory address for store
    void setMemAddress(uint32_t newAddress);
    uint32_t getMemAddress() const;


    // Get and set current state
    void setState(std::string updatedState);
    std::string getState() const;
    void resetState();

    void updateStatus();


    // Utility functions
    std::string getStageName() const;
    std::string getInstructionString(); // Get instruction string

    

private:

    StageType type;
    std::unique_ptr<Instruction> curr_instruction;
    std::string state = "* " + getStageName() + " : NOP\n";

    std::string new_style_istring;


};

#endif