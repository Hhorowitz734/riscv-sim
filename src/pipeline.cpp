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
        stages[StageType::IF].setInstruction(std::make_unique<Instruction>(instruction_map[pc]));
    } else {
        std::cerr << "Error: IF stage is full.\n";
    }

}

void Pipeline::comprehensiveAdvance() {
    /**
     * Main advancing function
     * 
     * 1) Increment PC (if not stalled)
     */

    // Need to check for stalls

    /**
     * Check if instruction can be written back
     * Then, write its result to memory
     */

    pc += 4;
    pipeline_registers.npc = pc + 4;

    advanceInstruction(WB, WB, true);
    advanceInstruction(DS, WB);
    advanceInstruction(DF, DS);
    advanceInstruction(EX, DF);
    advanceInstruction(RF, EX);
    advanceInstruction(ID, RF);
    advanceInstruction(IS, ID);
    advanceInstruction(IF, IS);
    sendNextInstruction();

    // Perform pipeline actions
    ISAction();
    executeInstruction();


   


    curr_cycle++;
    
}

void Pipeline::advanceInstruction(StageType from, StageType to, bool deallocate){
    /**
     * Moves an instruction unique ptr from "stages[from]" to "stages[to]"
     * If deallocate flag is set, simply deallocates the pointer
     */

    // Deallocate if deallocate flag is set
    // Here, to is irrelevant
    if (deallocate) { 
        stages[from].deallocateInstruction(); 
        return;
    }

    if (!stages[to].isEmpty()) {
        std::cerr << "Cannot send to " << stages[to].getStageName() << " because " << stages[to].getStageName() << " is already full." << std::endl;
        return;
    }

    if (stages[from].isEmpty()) {
        std::cerr << "Cannot send from " << stages[from].getStageName() << " because " << stages[from].getStageName() << " is empty." << std::endl;
        return;
    }


    stages[to].setInstruction(std::move(stages[from].getInstruction()));

}

void Pipeline::ISAction() {
    /**
     * Simulates an instruction fetch, in reality, just updates the pipeline register with the correct info
     * 
     * 
     * JUST KEEP IN MIND, THIS IS WRONG AND NEEDS FIXING
     */

    // No need if IS is empty
    if (stages[IS].isEmpty()) { return; }

    uint32_t value = stages[IS].getValue();

    std::ostringstream oss;

    
    // Extract each byte (big-endian order)
    uint8_t byte1 = (value >> 24) & 0xFF; // Most significant byte
    uint8_t byte2 = (value >> 16) & 0xFF;
    uint8_t byte3 = (value >> 8) & 0xFF;
    uint8_t byte4 = value & 0xFF;         // Least significant byte

    // Rearrange bytes into [Byte 3][Byte 2][Byte 4][Byte 1]
    oss << "<Fetched "
        << std::bitset<8>(byte3) << " "  // Byte 3
        << std::bitset<8>(byte2) << " "  // Byte 2
        << std::bitset<8>(byte4) << " "  // Byte 4
        << std::bitset<8>(byte1)         // Byte 1
        << ">";

    // Set the state of IS
    stages[IS].setState(oss.str());

    // Set pipeline register
    pipeline_registers.instruction_register = value;


    return;

}





void Pipeline::executeInstruction() {
    /**
     * Performs the computation currently in EX stage
     * 
     * CURRENTLY JUST DOES ADDI, NEEDS MORE FUNCTIONALITY
     */



    // DUMMY TEST CODE
    //stages[StageType::IF].setInstruction(std::make_unique<Instruction>(std::move(instructions[0])));
    //advanceInstruction(IF, RF);
    //advanceInstruction(RF, EX);
    //

    if (stages[StageType::EX].isEmpty()) {
        std::cerr << "Cannot perform computation when no instruction in EX" << std::endl;
        return;
    }

    INST_TYPE instruction_type = stages[StageType::EX].getInstructionType();

    switch (instruction_type) {

        case JAL:
        case JALR:
            executeJType();
            break;
        case IRR:
            executeRType();
            break;
        case I_TYPE:
            executeIRR(); //trust, this is correct and should probably be renamed
            break;
        case STORE:
            executeStore();
            break;
        case LOAD:
            executeLoad();
            break;
        case BRANCH:
            executeBranch();
            break;
        default: // unhandled -> BLANK, OTHER
            std::cerr << "Trying to execute unhandled type" << std::endl;
            return;
        
    }


    return;

}

void Pipeline::executeIRR() {

    // ADDI, SLTI, NOP

    // Gets dependencies, destination, and immediate of instruction in EX stage
    std::vector<uint32_t> dependencies = stages[StageType::EX].getDependencies();
    uint32_t destination = stages[StageType::EX].getDestination();
    int32_t immediate = stages[StageType::EX].getImmediate();

    // Does string manip to get everything into useable form
    std::string source_register = "R" + std::to_string(dependencies[0]);
    std::string destination_register = "R" + std::to_string(destination);

    // Gets the exact instruction we need to compute
    EXACT_INSTRUCTION inst = stages[StageType::EX].getExactInstruction();

    // Perform necessary computation
    switch(inst) {
        case ADDI:
            integer_registers[destination_register] = integer_registers[source_register] + immediate;
            return;
        case SLTI:
            integer_registers[destination_register] = 
                (static_cast<int32_t>(integer_registers[source_register]) < immediate) ? 1 : 0;
            return;
        default:
            std::cerr << "Could not execute IRR Type Instruction" << std::endl;
            return;
    }

}

void Pipeline::executeRType() {
    // ADD, SUB, SLL, SRL, SLT, AND, OR, XOR

    // Get dependencies and destination
    std::vector<uint32_t> dependencies = stages[StageType::EX].getDependencies();
    uint32_t dep_1 = dependencies[0];
    uint32_t dep_2 = dependencies[1];
    uint32_t destination = stages[StageType::EX].getDestination();

    // Does string manip to get everything into useable form
    std::string destination_register = "R" + std::to_string(destination);
    std::string source_register_1 = "R" + std::to_string(dependencies[0]);
    std::string source_register_2 = "R" + std::to_string(dependencies[1]);

    // Gets the exact instruction we need to compute
    EXACT_INSTRUCTION inst = stages[StageType::EX].getExactInstruction();
    
    //Perform necessary computation
    switch(inst) {
        case ADD:
            integer_registers[destination_register] = integer_registers[source_register_1] + integer_registers[source_register_2];
            return;
        case SUB:
            integer_registers[destination_register] = integer_registers[source_register_1] - integer_registers[source_register_2];
            return;
        case SLL:
            integer_registers[destination_register] = integer_registers[source_register_1] << (integer_registers[source_register_2] & 0x1F);
            return;
        case SRL:
            integer_registers[destination_register] = 
                static_cast<uint32_t>(integer_registers[source_register_1]) >> (integer_registers[source_register_2] & 0x1F);
            return;
        case SLT:
            integer_registers[destination_register] = (static_cast<int32_t>(integer_registers[source_register_1]) < static_cast<int32_t>(integer_registers[source_register_2])) ? 1 : 0;
            return;
        case AND:
            integer_registers[destination_register] = integer_registers[source_register_1] & integer_registers[source_register_2];
            return;
        case OR:
            integer_registers[destination_register] = integer_registers[source_register_1] | integer_registers[source_register_2];
            return;
        case XOR:
            integer_registers[destination_register] = integer_registers[source_register_1] ^ integer_registers[source_register_2];
            return;
        default:
            std::cerr << "Could not execute R Type Instruction" << std::endl;
            return;
            
    }
    

}

void Pipeline::executeLoad() {

    // Get offset, register, destination
    int32_t offset = stages[StageType::EX].getImmediate();
    uint32_t destination = stages[StageType::EX].getDestination();
    std::vector<uint32_t> dependencies = stages[StageType::EX].getDependencies();

    /**
     * std::cout << "Destination: R" << std::to_string(destination) << std::endl;
    std::cout << "Offset: " << std::to_string(offset) << std::endl;
    std::cout << "Dependency: R" << std::to_string(dependencies[0]) << std::endl;
     * 
     */
    

    // Does string manip to get everything into useable form
    std::string destination_register = "R" + std::to_string(destination);
    std::string source_register = "R" + std::to_string(dependencies[0]);


    // Base address from source register
    uint32_t base_address = integer_registers[source_register];

    // Calculate effective memory address
    uint32_t memory_address = base_address + offset;

    // Validate memory address (optional, based on your memory bounds)
    if (memory_address < 600 || memory_address > 1000) {
        std::cerr << "Memory access violation at address: " << memory_address << std::endl;
        return; // Early return or handle error
    }

    int32_t retrieved_data = getDataMemory(memory_address);

    integer_registers[destination_register] = retrieved_data;
    
}

void Pipeline::executeStore() {

    // Get offset, destination (base register), and dependencies (source register rs2)
    int32_t offset = stages[StageType::EX].getImmediate();
    uint32_t base_register_index = stages[StageType::EX].getDestination(); // rs1 (base register)
    std::vector<uint32_t> dependencies = stages[StageType::EX].getDependencies(); // rs2 (source register)

    // Convert base register (rs1) and source register (rs2) to strings
    std::string base_register = "R" + std::to_string(base_register_index); // e.g., "R3"
    std::string source_register = "R" + std::to_string(dependencies[0]);   // e.g., "R5"

    // Calculate the effective memory address
    uint32_t base_address = integer_registers[base_register];
    uint32_t memory_address = base_address + offset;

    // Retrieve the value from the source register (rs2)
    int32_t value_to_store = integer_registers[source_register];

    // Write the value to data memory
    setDataMemory(memory_address, value_to_store);

    // Debugging output
    std::cout << "SW executed: Stored value " << value_to_store
              << " into memory address " << memory_address << std::endl;
}

void Pipeline::executeJType() {
    // JAL, JALR

    int32_t offset = stages[StageType::EX].getImmediate();
    uint32_t pc_place_addr = stages[StageType::EX].getDestination(); // Address to place current PC
    std::vector<uint32_t> dependencies = stages[StageType::EX].getDependencies();

    uint32_t base_address;

    std::string destination_register = "R" + std::to_string(pc_place_addr);
    std::string address_register = "R" + std::to_string(dependencies[0]);

    // Gets the exact instruction we need to compute
    EXACT_INSTRUCTION inst = stages[StageType::EX].getExactInstruction();

    switch(inst) {
        case JAL_E:
            integer_registers[destination_register] = pipeline_registers.npc;
            pc += offset;
            return;
        case JALR_E: //check this
            base_address = integer_registers[address_register];
            integer_registers[destination_register] = pipeline_registers.npc;
            pc = (base_address + offset) & ~1;
            return;
        default:
            return;
    }

    
}

void Pipeline::executeBranch() {
    
    std::vector<uint32_t> dependencies = stages[StageType::EX].getDependencies();
    int32_t offset = stages[StageType::EX].getImmediate();

    // Registers to compare
    std::string term1_register = "R" + std::to_string(dependencies[0]);
    std::string term2_register = "R" + std::to_string(dependencies[1]);

    uint32_t term1 = integer_registers[term1_register];
    uint32_t term2 = integer_registers[term2_register];

    // Gets the exact instruction we need to compute
    EXACT_INSTRUCTION inst = stages[StageType::EX].getExactInstruction();

    // Flag for condition
    bool takeBranch = false;

    switch(inst) {
        case BEQ:
            if (term1 == term2) { takeBranch = true; } 
            break;
        case BNE:
            if (term1 != term2) { takeBranch = true; }
            break;
        case BGE:
            if (term1 >= term2) { takeBranch = true; }
            break; 
        case BLT:
            if (term1 <= term2) { takeBranch = true; }
            break;
        default:
            std::cerr << "Could not determine branch instruction" << std::endl;
            return;
    }

    // If condition is met, take the branch
    if (takeBranch) { pc += offset; }

    return;

    


}







bool Pipeline::setDataMemory(uint32_t address, int32_t data) {
    /**
     * Attempts to place data into address, if this exceeds bounds returns false
     */

    if (address < 600 || address > 1000 || address % 4 != 0) {
        std::cerr << "Memory access violation at address: " << address << std::endl;
        return false;
    }   

    data_memory[address] = data;
    return true;

}

int32_t Pipeline::getDataMemory(uint32_t address) {

    // Check if the address exists in the data_memory map
    if (data_memory.find(address) != data_memory.end()) {
        // Address exists, return the value
        return data_memory[address];
    } else {
        // Address does not exist, throw an error
        throw std::runtime_error("Memory access violation: Address " + std::to_string(address) + " is not valid in data memory.");
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

    output << "\n\n";

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
    return "Current PC = " + std::to_string(pc) + "\n";
}




void Pipeline::addInstruction(Instruction instruction) {
    /*
    * Takes in an instruction from Lexer and adds it to pipeline
    */

    instructions.push_back(instruction);
    instruction_map[492 + (instructions.size() * 4)] = instruction;
}