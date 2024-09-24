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
    
    Byte opcode_byte = get_opcode(instruction);

    //std::cout << "Opcode Bytes: " << std::bitset<7>(opcode_byte) << std::endl;

    return static_cast<INST_TYPE>(opcode_byte);
}



// HELPER FUNCTIONS FOR BREAKING UP INSTRUCTION
/*
* The implementation details of these functions are identical, so I will detail them once here
* Get instruction, apply a bitmask to relevant bits, then extract the n-bit string
* that describes each instruction part
*/
Byte get_funct7(Dword instruction) { return ((instruction & 0xFE000000) >> 25); }
Byte get_rs2(Dword instruction) { return ((instruction & 0x1F00000) >> 20); }
Byte get_rs1(Dword instruction) { return ((instruction & 0xF8000) >> 15); }
Byte get_funct3(Dword instruction) { return ((instruction & 0x7000) >> 12); }
Byte get_rd(Dword instruction) { return ((instruction & 0xF80) >> 7); }
Byte get_opcode(Dword instruction) { return (instruction & 0x7F); }

/*
* Different instruction types have different immediate formats
* These helper functions extract the immediate correctly by type
*/
Dword get_i_type_imm(Dword instruction) {
    Dword imm = (instruction >> 20) & 0xFFF; 
    
    // Sign extend
    if (imm & 0x800) {
        imm |= 0xFFFFF000;  
    }
    
    return imm;
}

Dword get_s_type_imm(Dword instruction) {
    Dword imm = 0;
    
    imm |= (instruction >> 25) & 0x7F;  // Extract bits 31-25
    imm |= (instruction >> 7) & 0x1F;   // Extract bits 11-7
    
    // Sign extend
    if (imm & 0x800) {
        imm |= 0xFFFFF000;  
    }
    
    return imm;
}

Dword get_b_type_imm(Dword instruction) {
    Dword imm = 0;

    imm |= (instruction >> 31) & 0x1 << 12;     // Extract sign bit
    imm |= (instruction >> 25) & 0x3F << 5;     // Extract bits 10-5
    imm |= (instruction >> 8) & 0xF << 1;       // Extract bits 4-1
    imm |= (instruction >> 7) & 0x1 << 11;      // Bit 11
    
    // Sign extend
    if (imm & 0x1000) {
        imm |= 0xFFFFE000;  
    }
    
    return imm;
}

Dword get_j_type_imm(Dword instruction) {
    Dword imm = 0;

    imm |= (instruction >> 31) & 0x1 << 20;     // Bit 20 
    imm |= (instruction >> 12) & 0xFF << 12;    // Bits 19-12
    imm |= (instruction >> 20) & 0x1 << 11;   // Bit 11
    imm |= (instruction >> 21) & 0x3FF << 1;    // Bits 10-1
    
    // Sign-extend the immediate
    if (imm & 0x100000) {
        imm |= 0xFFE00000;  // Sign-extend to 32 bits
    }
    
    return imm;
}



// GET EXACT INSTRUCTIONS
EXACT_INSTRUCTION decompose_IRR(Dword instruction) {
    
    // First, we can check bits funct3 to distinguish R-Type instructions
    Byte funct3 = get_funct3(instruction);

    //std::cout << "Bytes 14-12: " << std::bitset<3>(distinguishing_bits) << std::endl;

    switch (funct3) {
        case 0: break; //Need to distinguish ADD and SUB
        case 1: return SLL;
        case 2: return SLT;
        case 4: return XOR; 
        case 5: return SRL;
        case 6: return OR;
        case 7: return AND;
        default: return ERROR_EXACT_INSTRUCTION;
    }

    // Now, we need to extract funct7 (to check if add or sub)
    Byte funct7 = get_funct7(instruction);

    //std::cout << "Bytes 27-32: " << std::bitset<5>(distinguishing_bits) << std::endl;

    switch (funct7) {
        case 0: return ADD;
        case 8: return SUB;
        default: return ERROR_EXACT_INSTRUCTION;
    }
    
    return ERROR_EXACT_INSTRUCTION;
}

EXACT_INSTRUCTION decompose_JALR(Dword instruction) {
    // RET will be implemented as a specfic case of JALR with  JALR x0, x1, 0

    Byte rd = get_rd(instruction);
    Byte rs1 = get_rs1(instruction);
    Byte immediate = get_i_type_imm(instruction);

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

    // We can use the same technique as IRR, as funct3 bits are the distinguishing bits here
    Byte funct3 = get_funct3(instruction);

    switch (funct3) {
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

   // We can use the same technique as IRR, as funct3 bits are the distinguishing bits here
    Dword funct3 = get_funct3(instruction);

    switch (funct3) { 
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



Instruction get_populated_instruction(Dword instruction, INST_TYPE type) {
    
    // Create variable to hold instruction
    Instruction dummy;
    
    // Store the instruction's raw value
    dummy.value = instruction;

    // Determine which values are needed based on the instruction type
    switch (type) {
        case JAL:
            dummy.rd = get_rd(instruction);
            dummy.imm = get_j_type_imm(instruction);
            break;
        case JALR:
            dummy.rd = get_rd(instruction);
            dummy.rs1 = get_rs1(instruction);
            dummy.imm = get_i_type_imm(instruction);
            break;
        case IRR:  // Integer Register-Register (R-type)
            dummy.rd = get_rd(instruction);
            dummy.rs1 = get_rs1(instruction);
            dummy.rs2 = get_rs2(instruction);
            break;
        case STORE:  // S-Type
            dummy.rs1 = get_rs1(instruction);
            dummy.rs2 = get_rs2(instruction);
            dummy.imm = get_s_type_imm(instruction);
            break;
        case LOAD:  // I-Type (Load)
            dummy.rd = get_rd(instruction);
            dummy.rs1 = get_rs1(instruction);
            dummy.imm = get_i_type_imm(instruction);
            break;
        case I_TYPE:  // Immediate (ADDI, etc.)
            dummy.rd = get_rd(instruction);
            dummy.rs1 = get_rs1(instruction);
            dummy.imm = get_i_type_imm(instruction);
            break;
        case BRANCH:  // B-Type (BEQ, BNE, etc.)
            dummy.rs1 = get_rs1(instruction);
            dummy.rs2 = get_rs2(instruction);
            dummy.imm = get_b_type_imm(instruction);
            break;
        default:
            break;
    }
    
    return dummy;
}


// PRINTING FUNCTIONS
std::string register_to_string(Dword reg) {
    /*
    * Register to string (prepend x)
    */
    std::ostringstream ss;
    ss << "x" << reg;
    return ss.str();
}

std::string to_binary_string(Dword value, int bits) {
    /*
    * Binary to string
    */
    std::bitset<32> b(value);
    return b.to_string().substr(32 - bits, bits);
}

std::string instruction_to_string(Instruction inst, int position, bool isBlank) {
    /*
    * Takes an instruction and our position in a file
    * Pretty prints it as a decompiled version, as shown in example
    */

   std::ostringstream ss;

    // Handle blank instruction
    if (isBlank) {
        std::ostringstream ss;
    
        // Output 32-bit zeros
        ss << "00000000000000000000000000000000";
        
        // Append the position and "0"
        ss << "\t" << position << "\t0";
        
        return ss.str();
    }
    
    // Stringstream to pretty print the binary
    std::string binary_str = to_binary_string(inst.value, 32);
    ss << binary_str.substr(0, 6) << " " << binary_str.substr(6, 6) << " "
       << binary_str.substr(12, 5) << " " << binary_str.substr(17, 3) << " "
       << binary_str.substr(20, 5) << " " << binary_str.substr(25, 7);
    
    // Position
    ss << "\t" << position;

    // Exact Instruction
    ss << "\t" << exact_instruction_to_string(inst.instruction);
    
    // Params
    switch (inst.type) {
        case IRR:  // R-Type
            ss << "\t" << register_to_string(inst.rd) << ", "
               << register_to_string(inst.rs1) << ", "
               << register_to_string(inst.rs2);
            break;
        case I_TYPE:  // I-Type
            ss << "\t" << register_to_string(inst.rd) << ", "
               << register_to_string(inst.rs1) << ", "
               << inst.imm;
            break;
        case LOAD:  // Load
            ss << "\t" << register_to_string(inst.rd) << ", "
               << inst.imm << "(" << register_to_string(inst.rs1) << ")";
            break;
        case STORE:  // Store
            ss << "\t" << register_to_string(inst.rs2) << ", "
               << inst.imm << "(" << register_to_string(inst.rs1) << ")";
            break;
        case BRANCH:  // B-Type
            ss << "\t" << register_to_string(inst.rs1) << ", "
               << register_to_string(inst.rs2) << ", "
               << inst.imm;
            break;
        case JAL:  // J-Type (JAL)
            ss << "\t" << register_to_string(inst.rd) << ", "
               << inst.imm;
            break;
        case JALR:  // I-Type (JALR)
            ss << "\t" << register_to_string(inst.rd) << ", "
               << register_to_string(inst.rs1) << ", "
               << inst.imm;
            break;
        default:
            ss << "\tUNKNOWN";
            break;
    }

    return ss.str();
}