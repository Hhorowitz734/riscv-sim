#ifndef INSTRUCTION_H
#define INSTRUCTION_H

// Standard library string manip
#include <string>
#include <bitset> // For binary printing
#include <iostream>
#include <sstream>
#include <iomanip>

// Extra datatypes
typedef bool Bit;
typedef short Byte;
typedef uint16_t Word;
typedef uint32_t Dword;


// ENUM FOR INSTRUCTION TYPES [BY OPCODE, NOT EXACT]
enum INST_TYPE {
    BLANK = 0x0,
    JAL = 0x6F,
    JALR = 0x67,
    IRR = 0x33, // Integer register register
    STORE = 0x23,
    LOAD = 0x03,
    I_TYPE = 0x13, //Immediate
    BRANCH = 0x63,
    OTHER = 0xFF
};


// ENUM FOR EXACT INSTRUCTIONS
enum EXACT_INSTRUCTION {
    JAL_E,
    J,
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
    NOP,
    AND,
    OR,
    XOR,

    // I-Type Instructions
    ADDI,
    SLTI,

    // Branch Instructions
    BEQ,
    BNE,
    BGE,
    BLT,

    // Error Type
    ERROR_EXACT_INSTRUCTION,
};

// INSTRUCTION STRUCT
struct Instruction {
    Dword value;
    INST_TYPE type;
    EXACT_INSTRUCTION instruction;

    Dword rs1; // Source register 1
    Dword rs2; // Source register 2
    Dword rd; // Destination register
    int32_t imm; // Immediate value

};


// TO STRING FUNCTIONS
std::string exact_instruction_to_string(EXACT_INSTRUCTION instruction);
std::string itype_to_string(INST_TYPE instructionType);


// HANDLING OPCODE
INST_TYPE read_opcode(Dword instruction);


// HELPER FUNCTIONS FOR BREAKING UP INSTRUCTION
Byte get_funct7(Dword instruction);
Byte get_rs2(Dword instruction);
Byte get_rs1(Dword instruction);
Byte get_funct3(Dword instruction);
Byte get_rd(Dword instruction);
Byte get_opcode(Dword instruction);
int32_t get_i_type_imm(Dword instruction);
int32_t get_s_type_imm(Dword instruction);
int32_t get_b_type_imm(Dword instruction);
int32_t get_j_type_imm(Dword instruction);


// GET EXACT INSTRUCTIONS
EXACT_INSTRUCTION decompose_IRR(Dword instruction);
EXACT_INSTRUCTION decompose_JALR(Dword instruction);
EXACT_INSTRUCTION decompose_I_TYPE(Dword instruction);
EXACT_INSTRUCTION decompose_BRANCH(Dword instruction);
EXACT_INSTRUCTION decompose_JAL_J(Dword instruction);
EXACT_INSTRUCTION decompose_types(Dword instruction, INST_TYPE type);


// POPULATE VALUES FOR YOUR INSTRUCTION
Instruction get_populated_instruction(Dword instruction, INST_TYPE type);


// INSTRUCTION -> STRING, AS IN EXAMPLE
std::string register_to_string(Dword reg);
std::string to_binary_string(Dword value, int bits);
std::string instruction_to_string(Instruction inst, int position, bool isBlank);
std::string handle_special_case(Instruction inst, EXACT_INSTRUCTION type, int position);


#endif