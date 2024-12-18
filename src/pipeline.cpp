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
        setIntegerRegister(i, 0);
    }

    // Initialize data memory
    for (int i = 0; i <= 36; i+=4) {
        data_memory[600 + i] = 0;
    }

}




/**
 * FOR ADVANCING THE PIPELINE
 */
bool Pipeline::sendNextInstruction() {
    /**
     * Takes an instruction from "instructions" vector
     * Sends it to IF pipeline stage
     */

    

    auto it = instruction_map.find(pc); // Check if the key exists in the map
    if (it != instruction_map.end()) { // Key exists
        if (stages[StageType::IF].isEmpty()) {
            stages[StageType::IF].setInstruction(std::make_unique<Instruction>(it->second));
            std::cout << "Sent out instruction: " << stages[StageType::IF].getNewStyleIstring() << std::endl << "Cycle: " << std::to_string(curr_cycle) << std::endl;
            return true; 
        } else {
            std::cerr << "Error: IF stage is full.\n";
            return true;
        }
    } else {
        std::cerr << "Error: Instruction for pc not found.\n";
        return false; // returns false if program is at end
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

    bool endFlag = false; // flag to end program

    if (!flags.isRAWStalled) {
        pc += 4;
        pipeline_registers.npc = pc + 4;
    }

    forwarding.resetPathsOutput();
    handleStalledState();

    advanceInstruction(WB, WB, true);
    advanceInstruction(DS, WB);
    advanceInstruction(DF, DS);
    advanceInstruction(EX, DF);
    if (!flags.isRAWStalled || flags.stopStage == ID) { advanceInstruction(RF, EX); }
    if (!flags.isRAWStalled || flags.stopStage == RF) { advanceInstruction(ID, RF); }
    advanceInstruction(IS, ID);
    advanceInstruction(IF, IS);

    if (sendNextInstruction() == false && allPipelineStagesEmpty()) { // sendNextInstruction is false iff next pc has no instruction to send (not just if IF is full)
        endFlag = true;
    }

    // Perform pipeline actions


    writeBack();
    dataStore();
    dataFetch();
    executeInstruction();
    registerFetch();
    instructionDecode();
    ISAction();

    curr_cycle++;

    if (endFlag || curr_cycle == 127) { 
        std::cout << getCycleOutput();
        std::cout << "Program ended in comprehensiveAdvance()" << std::endl;
        exit(0); 
    }

    std::cout << "Instruction in IF: " << stages[StageType::IF].getNewStyleIstring() << std::endl;
    std::cout << "Instruction in IS: " << stages[StageType::IS].getNewStyleIstring() << std::endl;
    std::cout << "Instruction in ID: " << stages[StageType::ID].getNewStyleIstring() << std::endl;
    
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
        stages[from].resetState();
        if (flags.isRAWStalled || flags.isBranchStalled) { stages[from].setState("**STALL**"); }
        return;
    }

    if (stages[from].isEmpty()) {
        if (flags.isBranchStalled) { stages[to].setState("**STALL**"); }
        std::cerr << "Cannot send from " << stages[from].getStageName() << " because " << stages[from].getStageName() << " is empty." << std::endl;
        return;
    }

    if (!stages[to].isEmpty()) {
        std::cerr << "Cannot send to " << stages[to].getStageName() << " because " << stages[to].getStageName() << " is already full." << std::endl;
        return;
    }

    

    if (flags.isRAWStalled && from != IF && from != IS) { stages[from].setState("**STALL**"); }
    else { stages[from].resetState(); }

    stages[to].setInstruction(std::move(stages[from].getInstruction()));

}

bool Pipeline::allPipelineStagesEmpty() {

    if (stages[IF].isEmpty() &&
        stages[IS].isEmpty() &&
        stages[ID].isEmpty() &&
        stages[RF].isEmpty() &&
        stages[EX].isEmpty() &&
        stages[DF].isEmpty() &&
        stages[DS].isEmpty() &&
        stages[WB].isEmpty()) { return true; }

    return false;

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

void Pipeline::instructionDecode() {
    /**
     * IN THIS STAGE WE ALSO NEED TO DO HAZARD CHECKS
     * I WILL SKIP THIS FOR NOW
     */

    //Check type of instruction in ID state


    if (stages[StageType::ID].isEmpty()) {
        std::cerr << "Cannot decode empty instruction." << std::endl;
        return;
    }

    if (flags.isRAWStalled && curr_cycle != 16 && ((curr_cycle - 1) % 15 != 0)) { return; } // No need to check again
    // This is sketch

    EXACT_INSTRUCTION instruction = stages[StageType::ID].getExactInstruction();

    // Flags for if they can have a dependency in rs1, rs2, or both
    bool flag_dep1 = false;
    bool flag_dep2 = false;
    StageType dep1;
    StageType dep2;

    std::unordered_map<DEPENDENCY_TYPE, uint32_t> dependencies = stages[StageType::ID].getDependencies();

    // Number of cycles to stall if we hit a RAW hazard
    std::unordered_map<StageType, int> cycles_to_stall_raw = {
        {DS, 2},
        {DF, 3},
        {EX, 4},
        {RF, 5}
    };

    switch(instruction) {
        case LW: //Can only have a RAW hazard in rs1
        case ADDI:
        case SLTI:
        case JALR_E:
            flag_dep1 = true;
            dep1 = checkRAWHazard(RS1, dependencies[RS1]);
            break;
        case ADD: // Can have RAW hazards in rs1 and rs2
        case SUB:
        case SLT:
        case SLL:
        case SRL:
        case OR:
        case XOR:
        case BEQ:
        case BGE:
        case BNE:
        case BLT:
        case SW:
            flag_dep1 = true;
            flag_dep2 = true;
            dep1 = checkRAWHazard(RS1, dependencies[RS1]);
            dep2 = checkRAWHazard(RS2, dependencies[RS2]);
            break;
        default: //stuff like NOP which has neither
            return;
       
            
        

    }

    if (flag_dep1 && dep1 != NONE) { addDetectedForward(ID, dep1, RS1); }
    if (flag_dep2 && dep2 != NONE) { addDetectedForward(ID, dep2, RS2); }


    
    return; // temporarily rid of RAW stalls

    bool flag_load_dep = false; //flag to check for load dependency (only 1 cycle)
    int num_cycle_stall_1 = 0;
    int num_cycle_stall_2 = 0;
    int num_cycle_stall = 0;

    if (flag_dep1 && dep1 != NONE) { num_cycle_stall_1 = cycles_to_stall_raw[dep1]; }

    if (flag_dep2 && dep2 != NONE) { num_cycle_stall_2 = cycles_to_stall_raw[dep2]; }

    num_cycle_stall = std::max(num_cycle_stall_1, num_cycle_stall_2);
    
    if (num_cycle_stall > 0) { 

        std::cout << "Will stall for " << num_cycle_stall << " cycles." << std::endl; 

        // Turn on stalled mode
        flags.isRAWStalled = true;
        flags.RAWstallsRemaining = num_cycle_stall;
        
    }
    

    return;
}

StageType Pipeline::checkDataHazard(DEPENDENCY_TYPE reg, uint32_t register_dependency) {
    /**
     * Checks for a data hazard in reg register (RS1, RS2)
     */

    return NONE;

}

StageType Pipeline::checkRAWHazard(DEPENDENCY_TYPE reg, uint32_t register_dependency) {

    uint32_t result_register;

    // So we can iterate front to back to get soonest data hazard
    std::vector<StageType> stageOrder = {
        StageType::RF,
        StageType::EX,
        StageType::DF,
        StageType::DS
    };


    // Iterate in the specified order
    for (StageType stageType : stageOrder) {

        // Find next stage in map
        auto it = stages.find(stageType);
        const PipelineStage& pipelineStage = it->second;

        if (pipelineStage.isEmpty()) { continue; }

        // Depends on instruction
        EXACT_INSTRUCTION stageInstruction = pipelineStage.getExactInstruction();


        switch (stageInstruction) {
            
            // 1. R-Type instructions, IRR Type, LW, 
            case SLT:
            case SLL:
            case SRL:
            case SUB:
            case ADD:
            case NOP:
            case AND:
            case OR:
            case XOR:
            case ADDI:
            case SLTI:
                result_register = pipelineStage.getDestination();
                if (register_dependency == result_register) { return stageType; } // A dependency exists
                break;
            case LW: //Need a stall to manage this
                result_register = pipelineStage.getDestination();  /// aaa
                if (stageType == EX) { 
                    setRAWStall(2, RF);
                    break;
                     }
                if (stageType == RF) { setRAWStall(2, ID);
                break; }
                if (register_dependency != result_register) {break;}
                return stageType;
                break;
            
            // 2. Since JAL and JALR write in EX, they can only cause a hazard in EX
            default: // BRANCH type, RET, NOP, SW
                break;


        }

       
    }

    return NONE;

}



void Pipeline::addDetectedForward(StageType to, StageType from, DEPENDENCY_TYPE dep) {

    forwarding.setDetected(true);

    Instruction from_instruction = stages[from].getInstructionCopy();
    Instruction to_instruction = stages[to].getInstructionCopy();

    stages[to].setNeedsForward(true);
    //stages[from].setNeedsToForward(true);

    // How far ahead is it? FIX FIX edit with stalls LD
    int num_cycles_ahead = static_cast<int>(from) - static_cast<int>(to);
    stages[to].setNumCyclesAhead(dep, num_cycles_ahead);

    forwarding.addForward(from_instruction, to_instruction, num_cycles_ahead); // FIX FIX !!


}




void Pipeline::cancelInstruction(StageType stage) {
    /**
     * Cancels an instruction, for a JAL or BRANCH stall for example
     */
    if (flags.isBranchStalled) { stages[stage].setState("**STALL**"); }
    stages[stage].deallocateInstruction();
    stages[stage].setNeedsForward(false);
    stages[stage].setNumCyclesAhead(RS1, -1);
    stages[stage].setNumCyclesAhead(RS2, -1);
}



void Pipeline::registerFetch() {
    /**
     * Simulates the Register Fetch (RF) stage
     */

    if (stages[StageType::RF].isEmpty()) {
        std::cerr << "Cannot fetch register when no instruction in RF." << std::endl;
        return;
    }

    if (stages[StageType::RF].getAlreadyCompleted()) { return; }

    INST_TYPE instruction_type = stages[StageType::RF].getInstructionType();

    if (flags.isRAWStalled) { stages[StageType::RF].setAlreadyCompleted(true); }

    switch (instruction_type) {
        case JAL:
        case JALR:
            return registerFetchJType();
        case IRR:
            return registerFetchRType();
        case I_TYPE:
            return registerFetchIRR();
        case BRANCH:
            return registerFetchBranch();
        case LOAD:
        case STORE:
            return registerFetchLoadStore();
        default:
            return;

    }

   



}

void Pipeline::registerFetchJType() {

    // Handles JAL, J, JALR, RET

    // Figure out the exact J Type instruction
    EXACT_INSTRUCTION instruction = stages[StageType::RF].getExactInstruction();

    // Fetch dependencies
    std::unordered_map<DEPENDENCY_TYPE, uint32_t> dependencies = stages[StageType::RF].getDependencies();
    int32_t mem_address_value;


    switch(instruction) {

        case JAL_E: // JAL and J do nothing during RF stage
        case J:
            return;
        case JALR_E:
            // Fetches value in RS1 and sets memory address to return to accordingl
            
            mem_address_value = getIntegerRegister(dependencies[RS1]);
            stages[StageType::RF].setRegisterValue(RS1, mem_address_value);

            return;
        case RET:
            // Sets return address to value in R1
            mem_address_value = getIntegerRegister(1);
            stages[StageType::RF].setRegisterValue(RS1, mem_address_value);
            return;
        default:
            std::cerr << "Could not find specific J type instruction." << std::endl;
            return;
    }



}

void Pipeline::registerFetchLoadStore() {

    // Get exact instruction and dependencies
    EXACT_INSTRUCTION instruction = stages[StageType::RF].getExactInstruction();
    std::unordered_map<DEPENDENCY_TYPE, uint32_t> dependencies = stages[StageType::RF].getDependencies();

    //dependencies[RS1] = getForwardedValue(RF, RS1);
    //dependencies[RS2] = getForwardedValue(RF, RS2);

    uint32_t mem_address_value;
    int32_t value_to_store;

    switch(instruction) {

        case LW:
            // Gets just RS1 (address to load from)
            mem_address_value = getIntegerRegister(dependencies[RS1]);
            std::cout << "LW RS1: " << std::to_string(dependencies[RS1]) << std::endl;
            stages[StageType::RF].setRegisterValue(RS1, mem_address_value);
            std::cout << "Fetched: " << std::to_string(mem_address_value) << std::endl;
            return;

        case SW:
            // Gets RS1 (address to store to) and RS2 (value to store)
            mem_address_value = getIntegerRegister(dependencies[RS1]);
            stages[StageType::RF].setRegisterValue(RS1, mem_address_value);

            value_to_store = getIntegerRegister(dependencies[RS2]);
            stages[StageType::RF].setRegisterValue(RS2, value_to_store);
            return;
        

        default:
            std::cerr << "Unhandled type not of LW or SW in RF stage" << std::endl;
            return;

    }

}

void Pipeline::registerFetchRType() {

    // Get exact instruction and dependencies
    EXACT_INSTRUCTION instruction = stages[StageType::RF].getExactInstruction();

    if (instruction != ADD &&
        instruction != SUB &&
        instruction != SLT &&
        instruction != SLL &&
        instruction != AND &&
        instruction != OR &&
        instruction != XOR) {
            std::cerr << "Incorrect type of instruction passed to registerFetchRType()" << std::endl;
            return;
        }
    
    
    std::unordered_map<DEPENDENCY_TYPE, uint32_t> dependencies = stages[StageType::RF].getDependencies();

    // Fetch sources from integer register
    int32_t source_1 = getIntegerRegister(dependencies[RS1]);
    int32_t source_2 = getIntegerRegister(dependencies[RS2]);

    // Save values
    stages[StageType::RF].setRegisterValue(RS1, source_1);
    stages[StageType::RF].setRegisterValue(RS2, source_2);

    return;

}

void Pipeline::registerFetchIRR() {
    /**
     * IMMEDIATE SHOULD ALREADY BE FETCHED IN ID
     */

    // ADDI, SLTI, NOP

    // Get exact instruction and dependencies
    EXACT_INSTRUCTION instruction = stages[StageType::RF].getExactInstruction();

    if (instruction != ADDI && instruction != SLTI && instruction != NOP) { 
        std::cerr << "Improper instruction type passed to registerFetchIRR" << std::endl;
        return; }
    if (instruction == NOP) { return; } //just in case i need to handle this later so i dont forget

    std::unordered_map<DEPENDENCY_TYPE, uint32_t> dependencies = stages[StageType::RF].getDependencies();

    // RS1 is fetched as the source for the operation
    int32_t source_1 = getIntegerRegister(dependencies[RS1]);

    stages[StageType::RF].setRegisterValue(RS1, source_1);

    return;

}

void Pipeline::registerFetchBranch() {
    // BEQ, BNE, BGE, BLT
    /**
     * Offset should be decoded in ID
     */

    EXACT_INSTRUCTION instruction = stages[StageType::RF].getExactInstruction();

    if (instruction != BEQ &&
        instruction != BNE &&
        instruction != BGE &&
        instruction != BLT) {
            std::cerr << "Improper instruction type passed to registerFetchBranch()" << std::endl;
            return;
        }

    std::unordered_map<DEPENDENCY_TYPE, uint32_t> dependencies = stages[StageType::RF].getDependencies();

    // Fetch sources from integer register
    int32_t source_1 = getIntegerRegister(dependencies[RS1]);
    int32_t source_2 =getIntegerRegister(dependencies[RS2]);

    // Save values
    stages[StageType::RF].setRegisterValue(RS1, source_1);
    stages[StageType::RF].setRegisterValue(RS2, source_2);

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

void Pipeline::dataFetch() {

    if (stages[StageType::DF].isEmpty()) {
        std::cerr << "Cannot store data when no instruction in DF." << std::endl;
        return;
    }

    EXACT_INSTRUCTION instruction_type = stages[StageType::DF].getExactInstruction();

    if (instruction_type != SW && instruction_type != LW) { return; } // Only LW and SW

    if (instruction_type == SW) {

    
        int32_t newResult = getForwardedValue(DF, RS2); // sees if there is a new result available from forwarding
        stages[StageType::DF].setRegisterValue(RS2, newResult);

        

        uint32_t newMemAddress = getForwardedValue(DF, RS1);
        stages[StageType::DF].setMemAddress(newMemAddress + stages[StageType::EX].getImmediate());

        //std::cout << "Mem Address (DF): " << std::to_string(stages[StageType::DF].getMemAddress()) << std::endl;
        //std::cout << "Result (DF): " << std::to_string(stages[StageType::DF].getRegisterValues()[RS2]) << std::endl;
        //std::cout << "\n\n";

        return;
    }

    if (instruction_type == LW) {
        return; // FIX FIX FIX
    }

}

void Pipeline::dataStore() {

    if (stages[StageType::DS].isEmpty()) {
        std::cerr << "Cannot store data when no instruction in DS." << std::endl;
        return;
    }

    EXACT_INSTRUCTION instruction_type = stages[StageType::DS].getExactInstruction();

    // Assumption: Only SW instructions use the DS stage
    if (instruction_type != SW && instruction_type != LW) { return; }

    if (instruction_type == SW) {

        //stages[StageType::DS].setResult(getForwardedValue(DS, RS2));

        //std::cout << "Mem Address (DS): " << std::to_string(stages[StageType::DS].getMemAddress()) << std::endl;
        //std::cout << "Result (DS): " << std::to_string(stages[StageType::DS].getResult()) << std::endl;


        setDataMemory(stages[StageType::DS].getMemAddress()
                    ,stages[StageType::DS].getRegisterValues()[RS2]);
        return;
    }

    //LOAD -> set result as retrieved data
    int32_t retrieved_data = getDataMemory(stages[StageType::DS].getMemAddress());
    std::cout << "LD MEM ADDRESS: " <<std::to_string(stages[StageType::DS].getMemAddress()) << std::endl;
    stages[StageType::DS].setResult(retrieved_data);

    return; 



}

void Pipeline::writeBack() {

    if (stages[StageType::WB].isEmpty()) {
        std::cerr << "Cannot write back when no instruction in WB." << std::endl;
        return;
    }

    INST_TYPE instruction_type = stages[StageType::WB].getInstructionType();

    uint32_t destination;
    std::string destination_register;


    switch(instruction_type) {
        case I_TYPE: // ADDI, SLTI
        case IRR:
        case LOAD:
            destination = stages[StageType::WB].getDestination();
            setIntegerRegister(destination, stages[StageType::WB].getResult());
        default:
            return;
    }

}

void Pipeline::handleStalledState() {

    

    // Just return out if not stalled
    if (!flags.isRAWStalled && !flags.isBranchStalled) { return; }

    /** 
    std::cout << std::endl << "Handling stall at cycle: " << std::to_string(curr_cycle) << std::endl;
    std::cout << "Instruction in IF: " << stages[StageType::IF].getNewStyleIstring() << std::endl;
    std::cout << "Instruction in IS: " << stages[StageType::IS].getNewStyleIstring() << std::endl;
    std::cout << "Instruction in ID: " << stages[StageType::ID].getNewStyleIstring() << std::endl;
    std::cout << "\n\n";
    */

    // Cancel stall if no stall remaining
    if (flags.RAWstallsRemaining == 0 && flags.isRAWStalled) {
        flags.isRAWStalled = false;
        flags.stopStage = NONE;
        stages[StageType::RF].setAlreadyCompleted(false);
    } else if (flags.isRAWStalled) {
        flags.RAWstallsRemaining -= 1;
        stats.total_loads++;

        if (stages[flags.stopStage].getNeedsForward()) { // Make sure to preserve the distance with ID's other dependencies

            if (stages[flags.stopStage].getNumCyclesAhead(RS1) != -1) {
                stages[flags.stopStage].setNumCyclesAhead(RS1, stages[StageType::ID].getNumCyclesAhead(RS1) + 1);
            }

            if (stages[flags.stopStage].getNumCyclesAhead(RS2) != -1) {
                stages[flags.stopStage].setNumCyclesAhead(RS2, stages[StageType::ID].getNumCyclesAhead(RS2) + 1);
            }

        }
    }

    if (flags.branchStallsRemaining == 0 && flags.isBranchStalled) {
        flags.isBranchStalled = false;
        stages[StageType::ID].setAlreadyCompleted(false);
        stages[StageType::IF].setAlreadyCompleted(false);
        stages[StageType::IS].setAlreadyCompleted(false);
        stages[StageType::RF].setAlreadyCompleted(false);
        stages[StageType::EX].setAlreadyCompleted(false);
    } else if (flags.isBranchStalled) {
        flags.branchStallsRemaining -= 1;
    }

    return;

    

    return;
}

void Pipeline::setRAWStall(int numCycles, StageType stopStage) {
    /**
     * The stage "stopStage" will not move forward
     */
    flags.RAWstallsRemaining = numCycles;
    flags.isRAWStalled = true;
    flags.stopStage = stopStage;
    pc += 4;

    /*
    std::cout << std::endl << "Executed a raw stall at cycle: " << std::to_string(curr_cycle) << std::endl;
    std::cout << "Instruction in IF: " << stages[StageType::IF].getNewStyleIstring() << std::endl;
    std::cout << "Instruction in IS: " << stages[StageType::IS].getNewStyleIstring() << std::endl;
    std::cout << "Instruction in ID: " << stages[StageType::ID].getNewStyleIstring() << std::endl;
    std::cout << "\n\n";
    */
}



void Pipeline::executeIRR() {

    // ADDI, SLTI, NOP

    // Gets dependencies, destination, and immediate of instruction in EX stage
    std::unordered_map<DEPENDENCY_TYPE, int32_t> register_values = stages[StageType::EX].getRegisterValues();

    register_values[RS1] = getForwardedValue(EX, RS1);

    int32_t immediate = stages[StageType::EX].getImmediate();

    int32_t source_value = register_values[RS1];


    // Gets the exact instruction we need to compute
    EXACT_INSTRUCTION inst = stages[StageType::EX].getExactInstruction();

    // Perform necessary computation
    switch(inst) {
        case ADDI:
            stages[EX].setResult(source_value + immediate);
            return;
        case SLTI:
            stages[EX].setResult((static_cast<int32_t>(source_value) < immediate) ? 1 : 0);
            return;
        default:
            std::cerr << "Could not execute IRR Type Instruction" << std::endl;
            return;
    }

}

void Pipeline::executeRType() {
    // ADD, SUB, SLL, SRL, SLT, AND, OR, XOR

    // Get dependencies and destination
    std::unordered_map<DEPENDENCY_TYPE, int32_t> register_values = stages[StageType::EX].getRegisterValues();
    std::cout << "RS1: " << std::to_string(register_values[RS1]) << std::endl << "RS2: " << std::to_string(register_values[RS2]) << std::endl;
    register_values[RS1] = getForwardedValue(EX, RS1);
    register_values[RS2] = getForwardedValue(EX, RS2);
    std::cout << "RS1: " << std::to_string(register_values[RS1]) << std::endl << "RS2: " << std::to_string(register_values[RS2]) << std::endl;
    // Gets the exact instruction we need to compute
    EXACT_INSTRUCTION inst = stages[StageType::EX].getExactInstruction();

    int32_t source_register_1 = register_values[RS1];
    int32_t source_register_2 = register_values[RS2];

    //Perform necessary computation
    switch(inst) {
        case ADD:
            stages[EX].setResult(source_register_1 + source_register_2);
            return;
        case SUB:
            stages[EX].setResult(source_register_1 - source_register_2);
            return;
        case SLL:
            stages[EX].setResult((source_register_1 << (source_register_2 & 0x1F)));
            return;
        case SRL:
            stages[EX].setResult((static_cast<uint32_t>(source_register_1) >> (source_register_2 & 0x1F)));
            return;
        case SLT:
            stages[EX].setResult((static_cast<int32_t>(source_register_1) < static_cast<int32_t>(source_register_2)) ? 1 : 0);
            return;
        case AND:
            stages[EX].setResult(source_register_1 & source_register_2);
            return;
        case OR:
            stages[EX].setResult(source_register_1 | source_register_2);
            return;
        case XOR:
            stages[EX].setResult(source_register_1 ^ source_register_2);
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
    std::unordered_map<DEPENDENCY_TYPE, int32_t> register_values = stages[StageType::EX].getRegisterValues();
    register_values[RS1] = getForwardedValue(EX, RS1);

    std::cout << "Forwarded value: " << std::to_string(register_values[RS1]) << std::endl;
    
    // Does string manip to get everything into useable form
    std::string destination_register = "R" + std::to_string(destination);

    // Base address from source register
    uint32_t base_address = register_values[RS1];

    // Calculate effective memory address
    uint32_t memory_address = base_address + offset;

    // Validate memory address (optional, based on your memory bounds)
    if (memory_address < 600 || memory_address > 1000) {
        std::cerr << "Memory access violation at address: " << memory_address << std::endl;
        return; // Early return or handle error
    }

    stages[StageType::EX].setMemAddress(memory_address);

    return;

    //int32_t retrieved_data = getDataMemory(memory_address);

    //integer_registers[destination_register] = retrieved_data;
    
}

void Pipeline::executeStore() {

    // Get offset, destination (base register), and dependencies (source register rs2)
    int32_t offset = stages[StageType::EX].getImmediate();
    std::unordered_map<DEPENDENCY_TYPE, int32_t> register_values = stages[StageType::EX].getRegisterValues();
    
    uint32_t base_address = register_values[RS1];
    base_address = getForwardedValue(EX, RS1);

    uint32_t memory_address = base_address + offset;
    stages[StageType::EX].setMemAddress(memory_address);

    if (stages[StageType::EX].getNumCyclesAhead(RS2) == 3) {
        int32_t result = getForwardedValue(EX, RS2);
        stages[StageType::EX].setRegisterValue(RS2, result);
    }//Need to get WB if that value is needed


}

void Pipeline::executeJType() {
    // JAL, JALR

    int32_t offset = stages[StageType::EX].getImmediate();
    uint32_t pc_place_addr = stages[StageType::EX].getDestination(); // Address to place current PC

    uint32_t base_address;


    std::unordered_map<DEPENDENCY_TYPE, int32_t> register_values = stages[StageType::EX].getRegisterValues();
    register_values[RS1] = getForwardedValue(EX, RS1);

    // Gets the exact instruction we need to compute
    EXACT_INSTRUCTION inst = stages[StageType::EX].getExactInstruction();

    switch(inst) {
        case J:
            pc = 520;
            flags.branchStallsRemaining = 8;
            flags.isBranchStalled = true;

            // Cancel all instructions prior to jump
            cancelInstruction(IF);
            cancelInstruction(IS);
            cancelInstruction(ID);
            cancelInstruction(RF);
            sendNextInstruction();

            return;


        case JAL_E:
            setIntegerRegister(pc_place_addr, pipeline_registers.npc);
            pc += offset;
            pc -= 4; // to account for advancing at beginning of each cycle

            flags.branchStallsRemaining = 8;
            flags.isBranchStalled = true;

            // Cancel all instructions prior to jump
            cancelInstruction(IF);
            cancelInstruction(IS);
            cancelInstruction(ID);
            cancelInstruction(RF);
            

            return;
        case JALR_E: //check this
            base_address = register_values[RS1];
            setIntegerRegister(pc_place_addr, pipeline_registers.npc);
            pc = (base_address + offset) & ~1;
            pc -= 4; // to account for advancing at beginning of each cycle

            flags.branchStallsRemaining = 8;
            flags.isBranchStalled = true;

            // Cancel all instructions prior to jump
            cancelInstruction(IF);
            cancelInstruction(IS);
            cancelInstruction(ID);
            cancelInstruction(RF);

            return;


        default:
            return;
    }

    
}

void Pipeline::executeBranch() {
    
    std::unordered_map<DEPENDENCY_TYPE, int32_t> register_values = stages[StageType::EX].getRegisterValues();
    register_values[RS1] = getForwardedValue(EX, RS1);
    register_values[RS2] = getForwardedValue(EX, RS2);

    int32_t offset = stages[StageType::EX].getImmediate();

    uint32_t term1 = register_values[RS1];
    uint32_t term2 = register_values[RS2];

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

    // If condition is not met, don't take the branch
    if (!takeBranch) { return; }
    
    stats.total_branches++;
    pc += offset;
    pc -= 4; //to account for auto advancing


    flags.branchStallsRemaining = 8;
    flags.isBranchStalled = true;

    // Cancel all instructions prior to jump
    cancelInstruction(IF);
    cancelInstruction(IS);
    cancelInstruction(ID);
    cancelInstruction(RF);

    return;

    


}



uint32_t Pipeline::getForwardedValue(StageType stage, DEPENDENCY_TYPE dep) {


    /// UPDATE THIS SO IT DOES GETREGISTERVALUES FOR SW
    // FIX FIX FIX

    bool needsForward = stages[stage].getNeedsForward();
    EXACT_INSTRUCTION instruction_to = stages[stage].getExactInstruction();

    if (!needsForward) { 
        if (false) { return stages[stage].getDependencies()[dep]; }
        //if (instruction_to != SW && instruction_to != LW)
        else { return stages[stage].getRegisterValues()[dep]; }
    } //just returns the proper value if it doesnt need forwarding

    int num_cycles_ahead = stages[stage].getNumCyclesAhead(dep);

    if (num_cycles_ahead == -1) { 
        if (false) { return stages[stage].getDependencies()[dep]; }
        else { return stages[stage].getRegisterValues()[dep]; }
    } // if this dependency is fine just return proper value

    StageType from = static_cast<StageType>(static_cast<int>(stage) + num_cycles_ahead);
    uint32_t value = stages[from].getResult();


    if (!isValidForward(from, stage)) {
        if (false) { return stages[stage].getDependencies()[dep]; }
        else { return stages[stage].getRegisterValues()[dep]; }
    }


    std::cout << "Forwarded " << value << " from " << stages[from].getStageName() << " to " << stages[stage].getStageName() << std::endl;
    

    
    forwarding.completeForward(from, stage, stages[from].getInstructionCopy(), stages[stage].getInstructionCopy(), &stats);
    stages[stage].setNumCyclesAhead(dep, -1); //make it so you cant forward again

    return value;

}

bool Pipeline::isValidForward(StageType from, StageType to) {
    if ((from == StageType::DF && to == StageType::EX) || 
        (from == StageType::DS && to == StageType::DF) || 
        (from == StageType::DS && to == StageType::EX) || 
        (from == StageType::WB && to == StageType::DF) || 
        (from == StageType::WB && to == StageType::EX)) {
        return true;
    }
    return false;
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
        throw std::runtime_error(("Memory access violation (Cycle) " + std::to_string((curr_cycle - 1)) + ": Address " + std::to_string(address) + " is not valid in data memory."));
    }
}




/**
 * TO STRING FUNCTIONS
 */

std::string Pipeline::getCycleOutput() {

    std::ostringstream output;

    // Cycle header
    output << "***** Cycle #" << (curr_cycle - 1) << "***********************************************\n";

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
        output += "R" + std::to_string(i) + "\t" + std::to_string(getIntegerRegister(i)) + "\t";

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
    if (!flags.isRAWStalled) { 
        output += "(none)\n";
        return output;
    }

    // Handle a potential error
    if (stages[StageType::ID].isEmpty()) {
        std::cerr << "Stall not possible as ID slot is empty" << std::endl;
        exit(1);
    }

    output += stages[StageType::ID].getNewStyleIstring();
    
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

void Pipeline::setIntegerRegister(uint32_t register_num, int32_t val) {

    if (register_num > 31) { // Register number too large
        std::cerr << "Cannot write to invalid register " << register_num << ".";
        return;
    }

    //std::cerr << "Writing to integer_registers[" << ("R" + std::to_string(register_num)) << "]" << std::endl;

    integer_registers["R" + std::to_string(register_num)] = val;

    return;
}

int32_t Pipeline::getIntegerRegister(uint32_t register_num) {

    if (register_num > 31) { // Register number too large
        std::cerr << "Cannot read from invalid register " << register_num << ".";
        exit(-1);
    }

    //std::cerr << "Reading from integer_registers[" << ("R" + std::to_string(register_num)) << "]" << std::endl;

    return integer_registers.at(("R" + std::to_string(register_num)));

}