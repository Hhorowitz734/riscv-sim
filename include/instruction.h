#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>
#include <bitset>

// Extra datatypes
typedef bool Bit;
typedef short Byte;
typedef uint16_t Word;
typedef uint32_t Dword;


// ENUM FOR INSTRUCTION TYPES
enum INST_TYPE {
    JAL = 0x6F,
    JALR = 0x67,
    IRR = 0x33, // Integer register register
    STORE = 0x23,
    LOAD = 0x03,
    SLTI = 0x13,
    OTHER = 0xFF
};

enum EXACT_INSTRUCTION {
    JAL_E,
    JALR_E,
    RET,
    SW,
    LW,

    // R-Type Instructions
    SLT,
    SLL,
    SRL,
    SUB,
    ADD,
    AND,
    OR,
    XOR,

    SLTI_E,

    // Error Type
    ERROR_EXACT_INSTRUCTION,
};

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
        case SLTI_E: return "SLTI_E";
        case ERROR_EXACT_INSTRUCTION: return "ERROR_EXACT_INSTRUCTION";
        default: return "UNKNOWN_INSTRUCTION";
    }
}


struct Instruction {
    Dword value;
    INST_TYPE type;
};

// Distinguishing the opcode into 6 types

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
        case SLTI: return "SLTI";
        default:     return "UNKNOWN_TYPE";
    }
}

INST_TYPE read_opcode(Dword instruction) {
    /*
    * Uses a mask to isolate opcode byte, returns the type
    */
    
    Byte opcode_byte = (instruction & 0x0000007F);

    std::cout << "Opcode Bytes: " << std::bitset<7>(opcode_byte) << std::endl;

    return static_cast<INST_TYPE>(opcode_byte);
}



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


#endif