#include "../include/instruction.h"

// TO STRING FUNCTIONS
std::string exact_instruction_to_string(EXACT_INSTRUCTION instruction) {
    switch (instruction) {
        case JAL_E: return "JAL";
        case JALR_E: return "JALR";
        case RET: return "RET";
        case SW: return "SW";
        case LW: return "LW";
        case SLT: return "SLT";
        case SLL: return "SLL";
        case SRL: return "SRL";
        case SUB: return "SUB";
        case ADD: return "ADD";
        case AND: return "AND";
        case OR: return "OR";
        case XOR: return "XOR";
        case ADDI: return "ADDI";
        case SLTI: return "SLTI";
        case BEQ: return "BEQ";
        case BNE: return "BNE";
        case BGE: return "BGE";
        case BLT: return "BLT";
        case ERROR_EXACT_INSTRUCTION: return "ERROR_EXACT_INSTRUCTION";
        default: return "UNKNOWN_INSTRUCTION";
    }
}

std::string itype_to_string(INST_TYPE instructionType) {
     /*
    * Converts instruction type (categories) to string
    */
    switch (instructionType) {
        case JAL: return "JAL";
        case JALR: return "JALR";
        case IRR: return "IRR";
        case STORE: return "STORE";
        case LOAD: return "LOAD";
        case I_TYPE: return "I_TYPE";
        case BRANCH: return "BRANCH";
        default:     return "UNKNOWN_TYPE";
    }
}


// HANDLING OPCODE
INST_TYPE read_opcode(Dword instruction) {
    /*
    * Uses a mask to isolate opcode byte, returns the type
    */
    
    Byte opcode_byte = (instruction & 0x0000007F);

    //std::cout << "Opcode Bytes: " << std::bitset<7>(opcode_byte) << std::endl;

    return static_cast<INST_TYPE>(opcode_byte);
}


// GET EXACT INSTRUCTIONS
EXACT_INSTRUCTION decompose_IRR(Dword instruction) {
    
    // First, we can check bits 14-12 to distinguish R-Type instructions
    Dword mask = 0x00007000; // Mask off the relevant bits
    Byte distinguishing_bits = (instruction & mask) >> 12;

    //std::cout << "Bytes 14-12: " << std::bitset<3>(distinguishing_bits) << std::endl;

    switch (distinguishing_bits) {
        case 0: break; //Need to distinguish ADD and SUB
        case 1: return SLL;
        case 2: return SLT;
        case 4: return XOR; 
        case 5: return SRL;
        case 6: return OR;
        case 7: return AND;
        default: return ERROR_EXACT_INSTRUCTION;
    }

    // Now, we need to extract the first 5 bits (to check if add or sub)
    mask = 0xF8000000;
    distinguishing_bits = (instruction & mask) >> 27;

    //std::cout << "Bytes 27-32: " << std::bitset<5>(distinguishing_bits) << std::endl;

    switch (distinguishing_bits) {
        case 0: return ADD;
        case 8: return SUB;
        default: return ERROR_EXACT_INSTRUCTION;
    }
    
    return ERROR_EXACT_INSTRUCTION;
}

EXACT_INSTRUCTION decompose_JALR(Dword instruction) {
    // RET will be implemented as a specfic case of JALR with  JALR x0, x1, 0

    Dword mask = 0x1F; 
    Byte rd = (instruction >> 7) & mask;
    Byte rs1 = (instruction >> 15) & mask;
    Byte immediate = (instruction >> 20) & 0xFFF;

    // For RET we have rd = 0, rs1 = 0x1, immediate = 0

    //std::cout << "Rd: " << std::bitset<3>(rd) << std::endl;
    //std::cout << "Rs1: " << std::bitset<3>(rs1) << std::endl;
    //std::cout << "Immediate: " << std::bitset<5>(immediate) << std::endl;
    
    if (rd == 0 && rs1 == 1 && immediate == 0) { return RET; }

    return JALR_E;

}

EXACT_INSTRUCTION decompose_I_TYPE(Dword instruction) {
    /*
    * Decomposes the immediate type instructions (ADDI, SLTI)
    */

    // We can use the same technique as IRR, as 14-12 bits are the distinguishing bits here
    Dword mask = 0x00007000;
    Byte distinguishing_bits = (instruction & mask) >> 12;

    switch (distinguishing_bits) {
        case 0: return ADDI;
        case 2: return SLTI;
        default: break;
    }

    return ERROR_EXACT_INSTRUCTION;

}

EXACT_INSTRUCTION decompose_BRANCH(Dword instruction) {
    /*
    * Decomposes the branch type instructions (BGE, BNE, BEQ, BLT)
    */

   // We can use the same technique as IRR, as 14-12 bits are the distinguishing bits here
    Dword mask = 0x00007000;
    Byte distinguishing_bits = (instruction & mask) >> 12;

    switch (distinguishing_bits) { 
        case 0: return BEQ;
        case 1: return BNE;
        case 4: return BLT;
        case 5: return BGE;
        default: break;
    }
    
    return ERROR_EXACT_INSTRUCTION;
}

EXACT_INSTRUCTION decompose_types(Dword instruction, INST_TYPE type) {
    /*
    * Takes in an instruction type and the uint32 containing it, decomposes the type given by the opcode
    * into a precise instruction
    */

   switch (type) {
        case IRR: return decompose_IRR(instruction);
        case JALR: return decompose_JALR(instruction);
        case JAL: return JAL_E; // For these, we know that the only possible opcode is given here
        case STORE: return SW;
        case LOAD: return LW;
        case I_TYPE: return decompose_I_TYPE(instruction);
        case BRANCH: return decompose_BRANCH(instruction);
        default: return ERROR_EXACT_INSTRUCTION;
   }
}



