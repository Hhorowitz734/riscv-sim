#include "../include/instruction.h"

// TO STRING FUNCTIONS
std::string exact_instruction_to_string(EXACT_INSTRUCTION instruction) {
    /**
     * Converts exact instruction to string
     * Helper method for writing, also great for testing
     */
    switch (instruction) {
        case JAL_E: return "JAL";
        case J: return "J";
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
        case NOP: return "NOP";
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
*
*
* First I get the relevant bits
* Then, I need to sign preserve, as these are signed values but I'm extracting them to use as standalone
* Therefore, preserving the sign bit is crucial here so that no meaning is lost.
*/
int32_t get_i_type_imm(Dword instruction) {
    /**
     * Immediate that is first 12 bits
     */
    int32_t imm = (instruction >> 20) & 0xFFF; 
    
    // Sign extend
    if (imm & 0x800) {
        imm |= 0xFFFFF000;  
    }

    //std::cout << std::bitset<12>(imm) << std::endl;
    
    return imm;
}

int32_t get_s_type_imm(Dword instruction) {

    /**
     * CHECK THIS
     * THIS IS CAUSING ME A LOT OF TROUBLE
     */
    int32_t imm = 0;

    // First we extract AND SHIFT 31-25
    imm |= (instruction >> 25) & 0x7F;  // Extract 31-25

    // Then you need to make room for the lower part (11-7)
    imm <<= 5;                

    // Extract 11-7
    imm |= (instruction >> 7) & 0x1F;  

    // Sign extend
    if (imm & 0x800) {  
        imm |= 0xFFFFF000;  // Sign extend to 32 bits
    }

    return imm;
}



int32_t get_b_type_imm(Dword instruction) {
    int32_t imm = 0;

    // Extract immediate
    imm |= ((instruction >> 25) & 0x7F) << 5;  // Extract bits 31-25 
    imm |= ((instruction >> 7) & 0x1F) << 0;   // Extract bits 11-7

    // Sign extend 
    if (imm & (1 << 11)) { 
        imm |= 0xFFFFF800; // Fill upper bits based on bit 11
    }

    // For debugging
    //std::cout << std::bitset<16>(imm) << std::endl;

    return imm;
}




int32_t get_jalr_imm(Dword instruction) {
    /**
     * Jalr's immediate is the first 12 bits (31 - 20)
     */
    int32_t imm = 0;

    // Extract bits 31-20
    imm = (instruction >> 20) & 0xFFF;

    // Sign extend
    if (imm & (1 << 11)) { 
        imm |= 0xFFFFF000;
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
        case 0: return ADD; // Need to check ADD or NOP
        case 8: return SUB;
        default: return ERROR_EXACT_INSTRUCTION;
    }

    return ERROR_EXACT_INSTRUCTION;
}

EXACT_INSTRUCTION decompose_JALR(Dword instruction) {
    // RET will be implemented as a specfic case of JALR with  JALR x0, x1, 0

    Byte rd = get_rd(instruction);
    Byte rs1 = get_rs1(instruction);
    Byte immediate = get_jalr_imm(instruction);

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
        case 0: break;
        case 2: return SLTI;
        default: return ERROR_EXACT_INSTRUCTION;
    }

    // We break out here because we need to check the special case of ADDI (NOP)
    Byte rd = get_rd(instruction);
    Byte rs1 = get_rs1(instruction);
    Dword immediate = get_i_type_imm(instruction);

    if (rd == 0 && rs1 == 0 && immediate == 0) { return NOP; }

    return ADDI;

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

EXACT_INSTRUCTION decompose_JAL_J(Dword instruction) {
    /*
    * Decomposes the special case JAL and J
    */

   Byte rd = get_rd(instruction);

   if (rd == 0) { return J; }
   
   return JAL_E;

}

EXACT_INSTRUCTION decompose_types(Dword instruction, INST_TYPE type) {
    /*
    * Takes in an instruction type and the uint32 containing it, decomposes the type given by the opcode
    * into a precise instruction
    */

   switch (type) {
        case IRR: return decompose_IRR(instruction);
        case JALR: return decompose_JALR(instruction);
        case JAL: return decompose_JAL_J(instruction);
        case STORE: return SW;  // For these, we know that the only possible opcode is given here
        case LOAD: return LW;
        case I_TYPE: return decompose_I_TYPE(instruction);
        case BRANCH: return decompose_BRANCH(instruction);
        default: return ERROR_EXACT_INSTRUCTION;
   }
}



// POPULATE THE VALUES (REGISTERS) FOR YOUR INSTRUCTION
Instruction get_populated_instruction(Dword instruction, INST_TYPE type) {
    
    // Create variable to hold instruction
    Instruction dummy;
    
    // Give initialized values
    dummy.value = instruction;
    dummy.rs1 = 0;
    dummy.rs2 = 0;
    dummy.rd = 0;
    dummy.imm = 0;

    // Determine which values are needed based on the instruction type
    switch (type) {
        case JAL:
            dummy.rd = get_rd(instruction);
            dummy.imm = get_i_type_imm(instruction);
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



// PRINTING HELPER FUNCTIONS
std::string register_to_string(Byte reg) {
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

    // Exact Instruction with aligned formatting
    ss << "\t" << exact_instruction_to_string(inst.instruction);

    /**
     * This stuff is really not super useful. I just want to make the "diff" match since that's how grading is done.
     */

    // Spacing adjustment
    std::string mnemonic = exact_instruction_to_string(inst.instruction);
    int padding = 6 - mnemonic.length(); // Padding adjustment
    ss << std::string(padding, ' ');

    // Params
    switch (inst.type) {
        case IRR:  // R-Type
            ss << register_to_string(inst.rd) << ", "
               << register_to_string(inst.rs1) << ", "
               << register_to_string(inst.rs2);
            break;
        case I_TYPE:  // I-Type
            ss << register_to_string(inst.rd) << ", "
               << register_to_string(inst.rs1) << ", "
               << inst.imm;
            break;
        case LOAD:  // Load (I-Type)
            ss << register_to_string(inst.rd) << ", "
               << inst.imm << "(" << register_to_string(inst.rs1) << ")";
            break;
        case STORE:  // Store (S-Type)
            ss << register_to_string(inst.rs2) << ", "
               << inst.imm << "(" << register_to_string(inst.rs1) << ")";
            break;
        case BRANCH:  // B-Type
            ss << register_to_string(inst.rs1) << ", "
               << register_to_string(inst.rs2) << ", "
               //<< std::bitset<30>(inst.imm);
               << inst.imm;
            break;
        case JAL:  // J-Type (JAL)
            ss << register_to_string(inst.rd) << ", "
               << inst.imm;
            break;
        case JALR:  // I-Type (JALR)
            ss << register_to_string(inst.rd) << ", "
               << register_to_string(inst.rs1) << ", "
               << inst.imm;
            break;
        default:
            ss << "UNKNOWN";
            break;
    }

    return ss.str();
}


std::string handle_special_case(Instruction inst, EXACT_INSTRUCTION type, int position) {
    /*
    * We need a helper function to handle the special cases that can arise from our custom instructions
    * These being J, NOP, and RET
    */

    std::ostringstream ss;

    // First, split into parts the same way as normal
    std::string binary_str = to_binary_string(inst.value, 32);

    ss << binary_str.substr(0, 6) << " " << binary_str.substr(6, 6) << " "
       << binary_str.substr(12, 5) << " " << binary_str.substr(17, 3) << " "
       << binary_str.substr(20, 5) << " " << binary_str.substr(25, 7);

    // I realize the above could be its own function, but this is done for simplicity's sake
    // For such an important assignment, I want to avoid using memory allocation as this exposes me to potential segfaults

    // Append the position
    ss << "\t" << position;



    switch (type) {
        case J: 
            ss << "\tJ\t#" << (position + inst.imm); // Address relative to position
            ss << "  //JAL x0, " << inst.imm; // Comment for J
            break;
        case NOP:
            ss << "\tNOP";
            ss << "\t\t//ADDI x0, x0, 0";
            break;
        case RET:
            ss << "\tRET";
            ss << "\t\t//JALR x0, x1, 0";
            break;
        default:
            ss << "ERROR"; // An instruction that isn't of one of these types should never even be passed to this function
    }

    return ss.str();

}