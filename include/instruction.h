#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>

// Extra datatypes
typedef bool Bit;
typedef short Byte;
typedef uint16_t Word;
typedef uint32_t Dword;


// ENUM FOR INSTRUCTION TYPES
enum I_TYPE {
    JAL = 0x6F,
    JALR,
    BEQ,
    BNE,
    BGE,
    BLT,
    ADDI,
    RET,
    SLT,
    SW,
    LW,
    SLL,
    SRL,
    SUB,
    ADD,
    AND,
    OR,
    XOR,
    SLTI,
    ERROR_TYPE
};

struct Instruction {
    Dword value;
    I_TYPE type;
};


std::string itype_to_string(I_TYPE instructionType) {
    /*
    * Converts instruction type to string 
    */
    switch (instructionType) {
        case JAL:   return "JAL";
        case JALR:  return "JALR";
        case BEQ:   return "BEQ";
        case BNE:   return "BNE";
        case BGE:   return "BGE";
        case BLT:   return "BLT";
        case ADDI:  return "ADDI";
        case RET:   return "RET";
        case SLT:   return "SLT";
        case SW:    return "SW";
        case LW:    return "LW";
        case SLL:   return "SLL";
        case SRL:   return "SRL";
        case SUB:   return "SUB";
        case ADD:   return "ADD";
        case AND:   return "AND";
        case OR:    return "OR";
        case XOR:   return "XOR";
        case SLTI:  return "SLTI";
        case ERROR_TYPE: return "ERROR_TYPE";
        default:    return "UNKNOWN";
    }
}

I_TYPE read_opcode(Dword instruction) {
    /*
    * Uses a mask to isolate opcode byte, returns the type
    */
    
    Byte opcode_byte = instruction & 0x7F;

    return static_cast<I_TYPE>(opcode_byte);
}

#endif