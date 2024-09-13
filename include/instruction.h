#ifndef INSTRUCTION_H
#define INSTRUCTION_H

// DWORD datatype (4 bytes, instruction size)
typedef uint32_t Dword;

// ENUM FOR INSTRUCTION TYPES
enum I_TYPE {
    JAL,
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
    SLTI
};

struct Instruction {
    Dword value;
    I_TYPE type;
};

#endif