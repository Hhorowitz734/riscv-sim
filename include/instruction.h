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
    OTHER = 0xFF
};

struct Instruction {
    Dword value;
    INST_TYPE type;
};


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
        default:     return "UNKNOWN_TYPE";
    }
}

INST_TYPE read_opcode(Dword instruction) {
    /*
    * Uses a mask to isolate opcode byte, returns the type
    */
    
    Byte opcode_byte = (instruction & 0x0000007F);

    std::cout << std::bitset<8>(opcode_byte) << std::endl;

    return static_cast<INST_TYPE>(opcode_byte);
}

#endif