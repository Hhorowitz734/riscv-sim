#include "../include/lexer.h"


void Lexer::set_input_file(const char* filename) {
    /*
    * Attempts to open useor provided filename
    */

    inputFile.open(filename, std::ios::in);

    // Handle file open failed
    if (!inputFile.is_open()) {
        std::cerr << "Error: File [" << filename << "] could not be opened." << std::endl;
    }

    // Sets static file size variable
    inputFile.seekg(0, std::ios::end); // Set pointer to end
    fileSize = static_cast<std::size_t>(inputFile.tellg());
    inputFile.seekg(0, std::ios::beg); // Skip back to beginning

}

void Lexer::set_output_file(const char* filename) {
    /*
    * Attempts to open filename for writing
    */

    // Open the file in output mode
    outputFile.open(filename, std::ios::out | std::ios::trunc);  // std::ios::trunc overwrites if the file exists

    // Handle file open failed
    if (!outputFile.is_open()) {
        std::cerr << "Error: Output file [" << filename << "] could not be opened." << std::endl;
        return;
    }

    std::cout << "Output file [" << filename << "] successfully opened." << std::endl;
}

void Lexer::write_output(std::string output) {
    /*
    * Writes a string to output file, or throws an error
    */

   if (outputFile.is_open()) {
        outputFile << output << std::endl;
    } else {
        std::cerr << "Could not open output file!" << std::endl;
        exit(1);
    }
}

// Self explanatory, tells us if we've reached end
bool Lexer::isEOF() { return inputFile.eof(); }



Dword Lexer::consume_instruction() {
    /*
    * Reads next byte
    */

    // Even if "instruction" is all 0s, we should consume it
    instructions_consumed++;

    // Skip newline characters
    char nextChar = inputFile.peek();
    while (nextChar == '\n' || nextChar == ' ') {
        inputFile.seekg(1, std::ios::cur);  // Moves the cursor one character forward
        bitsConsumed++;
        nextChar = inputFile.peek();  // Update peek to the next character
    }

    // Handle case that no bytes remain
    if (fileSize - bitsConsumed < 32) {
        std::cerr << "No bytes left to read!" << std::endl; 
        exit(0);
    }

    // Buffer to read in byte
    char buff[32];  // Assumes the buffer contains a binary representation of the instruction

    // Consume and update counter
    inputFile.read(buff, 32);  // Read the next 32 characters
    bitsConsumed += 32;

    // Convert string buffer to 32-bit instruction
    Dword instruction = 0;

    for (int i = 0; i < 32; i++) {
        switch (buff[i]) {
            case '0':
                instruction = instruction << 1;  // Shift and add 0
                break;
            case '1':
                instruction = (instruction << 1) | 0x1;  // Shift and add 1
                break;
            default:
                //std::cerr << "Faulty bit found in text: " << buff[i] << std::endl;
                break;
        }
    }

    return instruction;

}

void Lexer::reset_instruction() {
    /*
    * Resets currently used instruction to read in a new one
    */

    // Create new instruction with default values
    curr_instruction.type = OTHER;
    curr_instruction.value = 0;

}

void Lexer::read_next_instruction() { 

    // Give default value for output
    std::string output = "";

    // Reset
    reset_instruction();

    // Get next instruction value
    Dword instruction_val = consume_instruction();
    curr_instruction.value = instruction_val;

    // Get opcode
    INST_TYPE opcode = read_opcode(instruction_val);
    curr_instruction.type = opcode;

    // Check if instruction is blank, then print it if it is
    if (opcode == BLANK) {
        output = instruction_to_string(curr_instruction, start_position + (instructions_consumed * 4) - 4, true);
        write_output(output);
        return;
    }

    //std::cout << "Instruction type: " << itype_to_string(opcode) << std::endl;

    // Retrieve the exact instruction
    EXACT_INSTRUCTION exact_instruction = decompose_types(instruction_val, opcode);
    curr_instruction.instruction = exact_instruction;

    //std::cout << "Exact instruction: " << exact_instruction_to_string(exact_instruction) << std::endl;

    // Find fields as necessary
    Instruction dummy_populated_instruction = get_populated_instruction(instruction_val, opcode);
    curr_instruction.rs1 = dummy_populated_instruction.rs1;
    curr_instruction.rs2 = dummy_populated_instruction.rs2;
    curr_instruction.rd = dummy_populated_instruction.rd;
    curr_instruction.imm = dummy_populated_instruction.imm;

    // Get instruction as string and write it out
    output = instruction_to_string(curr_instruction, start_position + (instructions_consumed * 4) - 4, false);
    write_output(output);
    

}


std::size_t Lexer::getFileSize() const { return fileSize; }