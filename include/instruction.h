#ifndef INSTRUCTION_H
#define INSTRUCTION_H

// Standard library string manip
#include <string>
#include <bitset> // For binary printing
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <regex>



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

enum DEPENDENCY_TYPE {
    RS1,
    RS2
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

    int32_t result; // Result of the computation (for pipeline)
    int32_t mem_address_store; //For instructions where you need to store an address

    std::unordered_map<DEPENDENCY_TYPE, int32_t> registerValues; // Values of registers, retrieved during RF stage

    std::unordered_map<DEPENDENCY_TYPE, int32_t> getRegisterValues() const { return registerValues; }
    void setRegisterValue(DEPENDENCY_TYPE reg, int32_t newValue) { registerValues[reg] = newValue; }

    /**
     * Flag that the instruction needs forwarding.
     */
    bool needsForward = false;
    bool needsToForward = false;
    //int numCyclesAhead = 0; // How many cycles ahead is the instruction you're forwarding from?
    void setForwardFlag(bool newFlag) {
        needsForward = newFlag;
    }
    bool getForwardFlag() const { return needsForward; }

    void setNeedsToForwardFlag(bool newFlag) { needsToForward = newFlag; }
    bool getNeedsToForwardFlag() const { return needsToForward; }

    std::unordered_map<DEPENDENCY_TYPE, int> forward_from; // Stores how many cycles ahead to forward each dependency from 

    void setNumCyclesAhead(DEPENDENCY_TYPE dep, int newNumCyclesAhead) {
        forward_from[dep] = newNumCyclesAhead;
    }
    
    int getNumCyclesAhead(DEPENDENCY_TYPE dep) const { 
        auto it = forward_from.find(dep); // Look for the key in the map
        if (it != forward_from.end()) {
            return it->second; // Key found, return the associated value
        }
        return -1; // Key not found, return -1
    }



    

        
    std::unordered_map<DEPENDENCY_TYPE, uint32_t> getDependencies() const {

        std::unordered_map<DEPENDENCY_TYPE, uint32_t> dependencies;

        switch (type) {
            case JALR: // JALR uses rs1
                dependencies[RS1] = rs1;
                break;

            case LOAD: // LW uses rs1 for address calculation
                dependencies[RS1] = rs1;
                break;

            case STORE: // SW uses rs1 for address and rs2 for value
                dependencies[RS1] = rs1;
                dependencies[RS2] = rs2;
                break;

            case I_TYPE: // I-Type instructions use rs1
                dependencies[RS1] = rs1;
                break;

            case IRR: // R-Type instructions (e.g., ADD, SUB) use rs1 and rs2
            case BRANCH: // Branch instructions use rs1 and rs2
                dependencies[RS1] = rs1;
                dependencies[RS2] = rs2;
                break;

            default: // Default case handles other instructions with rs1 and rs2
                dependencies[RS1] = rs1;
                dependencies[RS2] = rs2;
                break;
        }

        return dependencies;
    }

    uint32_t getDestination() const {

        if (type == STORE || type == BRANCH || type == BLANK || type == OTHER) {
            std::cerr << "Error: Instruction of type " << type << " should not have a destination." << std::endl;
            return static_cast<uint32_t>(-1); // Return a sentinel value indicating no destination
        }

        return rd;

    }

    int32_t getImmediate() const { return imm; }

    INST_TYPE getInstType() const { return type; }
    EXACT_INSTRUCTION getExactInstruction() const { return instruction; }

    uint32_t getValue() const { return value; }

    int32_t getResult() const { return result; }
    void setResult(int32_t newResult) { result = newResult; }

    uint32_t getMemAddress() const { return mem_address_store; }
    void setMemAddress(uint32_t newAddress) { mem_address_store = newAddress; }

    

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
int32_t get_jal_imm(Dword instruction);
int32_t get_s_type_imm(Dword instruction);
int32_t get_b_type_imm(Dword instruction); // MADE CORRECTIONS -> WRITE TESTS TO CHECK
int32_t get_jalr_imm(Dword instruction);


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
std::string register_to_string(Byte reg);
std::string to_binary_string(Dword value, int bits);
std::string instruction_to_string(Instruction inst, int position, bool isBlank);
std::string handle_special_case(Instruction inst, EXACT_INSTRUCTION type, int position);
std::string instruction_to_new_style_string(Instruction inst);

#endif