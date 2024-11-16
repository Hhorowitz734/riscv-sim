#include "../include/pipeline.h"

// Constructors
Pipeline::Pipeline() {
    // Initialize the 8 pipeline stages
    stages[StageType::IF] = PipelineStage(StageType::IF);
    stages[StageType::IS] = PipelineStage(StageType::IS);
    stages[StageType::ID] = PipelineStage(StageType::ID);
    stages[StageType::RF] = PipelineStage(StageType::RF);
    stages[StageType::EX] = PipelineStage(StageType::EX);
    stages[StageType::DF] = PipelineStage(StageType::DF);
    stages[StageType::DS] = PipelineStage(StageType::DS);
    stages[StageType::WB] = PipelineStage(StageType::WB);

    // Initialize the 32 integer registers
    for (int i = 0; i < 32; ++i) {
        integer_registers["R" + std::to_string(i)] = 0;
    }

    // Initialize data memory
    for (int i = 0; i <= 36; i+=4) {
        data_memory[600 + i] = 0;
    }

}




/**
 * FOR ADVANCING THE PIPELINE
 */
void Pipeline::sendNextInstruction() {
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




/**
 * TO STRING FUNCTIONS
 */

std::string Pipeline::getCycleOutput() {

    std::ostringstream output;

    // Cycle header
    output << "***** Cycle #" << curr_cycle << "***********************************************\n";

    // Current PC
    output << getPCOutput();

    output << "\n";

    // Pipeline Status
    output << getPipelineStatusOutput();

    // Stall Instruction
    output << "\n" << getStalledInstruction();

    output << "\n" << forwarding.toString() << "\n";

    // Pipeline Registers
    output << "\n" << getPipelineRegistersOutput() << "\n";

    // Integer Registers
    output << getIntegerRegistersOutput() << "\n";

    output << getDataMemoryOutput() << "\n";

    output << stats.toString();

    return output.str();
}

std::string Pipeline::getPipelineStatusOutput() {
    std::string output;
    output += "Pipeline Status: \n";

    // Iterate through the stages and append their state to the output
    output += stages[StageType::IF].getState();
    output += stages[StageType::IS].getState();
    output += stages[StageType::ID].getState();
    output += stages[StageType::RF].getState();
    output += stages[StageType::EX].getState();
    output += stages[StageType::DF].getState();
    output += stages[StageType::DS].getState();
    output += stages[StageType::WB].getState();

    return output;
}

std::string Pipeline::getIntegerRegistersOutput() {
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

std::string Pipeline::getPipelineRegistersOutput() const {

    std::ostringstream output;

    // Format the output string
    output << "Pipeline Registers:\n";
    output << "* IF/IS.NPC\t: " << pipeline_registers.npc << "\n";
    output << "* IS/ID.IR\t: ";
    
    // Check if instruction_register is 0
    if (pipeline_registers.instruction_register == 0) {
        output << "0\n"; // Just print 0 if the value is 0
    } else {
        output << "<";
        // Convert instruction_register to hexadecimal format (byte by byte)
        for (int i = 3; i >= 0; --i) {
            if (i < 3) output << " ";
            output << std::setw(2) << std::setfill('0') 
                << std::hex << ((pipeline_registers.instruction_register >> (i * 8)) & 0xFF);
        }
        output << ">\n";
    }

    output << std::dec; // Switch back to decimal format
    output << "* RF/EX.A\t: " << pipeline_registers.rf_regs_rs << "\n";
    output << "* RF/EX.B\t: " << pipeline_registers.rf_regs_rt << "\n";
    output << "* EX/DF.ALUout\t: " << pipeline_registers.alu_output << "\n";
    output << "* EX/DF.B\t: " << pipeline_registers.forward_val << "\n";
    output << "* DS/WB.ALUout-LMD\t : " << pipeline_registers.result_alu << "\n";

    return output.str();
}

std::string Pipeline::getDataMemoryOutput() const {

    std::ostringstream output;

    output << "Data memory:\n";
    for (int addr = 600; addr <= 636; addr += 4) { // Iterate through addresses
        int value = 0;
        if (data_memory.find(addr) != data_memory.end()) {
            value = data_memory.at(addr); // Get value if present
        }
        output << addr << ": " << value << "\n";
    }

    return output.str();
}

std::string Pipeline::getStalledInstruction() {

    /**
     * 
     * NOTE: THIS FUNCTION NEEDS TO BE TESTED IN FUTURE
     * IT CAN HANDLE NONE CASE JUST FINE
     * BUT BE CAREFUL WITH IT
     */

    std::string output = "Stall Instruction: ";

    // No stalled
    if (!flags.isStalled) { 
        output += "(none)\n";
        return output;
    }

    // Handle a potential error
    if (stages[StageType::ID].isEmpty()) {
        std::cerr << "Stall not possible as ID slot is empty" << std::endl;
        exit(1);
    }

    output += stages[StageType::ID].getInstructionString();
    
    return output;
}

std::string Pipeline::getPCOutput() {
    return "Current PC = " + std::to_string(pipeline_registers.npc - 4) + "\n";
}




void Pipeline::addInstruction(Instruction instruction) {
    /*
    * Takes in an instruction from Lexer and adds it to pipeline
    */

    instructions.push_back(instruction);
}