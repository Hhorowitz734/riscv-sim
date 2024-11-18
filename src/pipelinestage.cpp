#include "../include/pipeline.h"


// CONSTRUCTORS
PipelineStage::PipelineStage() : type(StageType::IF) {}

PipelineStage::PipelineStage(StageType type) : type(type) {
    if (type == IF) { state = "* IF : <unknown>\n"; }
};




// INSTRUCTION UNIQUE POINTER MANAGEMENT
void PipelineStage::setInstruction(std::unique_ptr<Instruction> instr) { 

    // Move new instruction
    curr_instruction = std::move(instr);


    // Result of the function fixes the istring

    std::string input = getInstructionString();

    // Remove trailing newline, if it exists
    if (!input.empty() && input.back() == '\n') {
        input.pop_back();
    }

    input = std::regex_replace(input, std::regex(R"(x(\d+))"), "R$1");

    // Replace immediate values with "#" prefix
    if (getExactInstruction() != LW && getExactInstruction() != SW) {
        input = std::regex_replace(input, std::regex(R"(\b(\d+)\b)"), "#$1");
    }

    // Remove everything before the last tab
    size_t lastTab = input.find_last_of('\t');
    if (lastTab != std::string::npos) {
        input = input.substr(lastTab + 1);
    }

    new_style_istring = input;
    
    // Update status to reflect changes
    updateStatus();

}

std::unique_ptr<Instruction> PipelineStage::clearInstruction() { return std::move(curr_instruction); }

bool PipelineStage::isEmpty() const { return curr_instruction == nullptr; }





// GETTERS

StageType PipelineStage::getStageType() const { return type; }
std::unordered_map<DEPENDENCY_TYPE, uint32_t> PipelineStage::getDependencies() const { return curr_instruction->getDependencies(); } // Protect these from segfaults
uint32_t PipelineStage::PipelineStage::getDestination() const { return curr_instruction->getDestination(); }
int32_t PipelineStage::getImmediate() const { return curr_instruction->getImmediate(); }
EXACT_INSTRUCTION PipelineStage::getExactInstruction() const { return curr_instruction->getExactInstruction(); }
uint32_t PipelineStage::getValue() const { return curr_instruction->getValue(); }
void PipelineStage::deallocateInstruction() { curr_instruction.reset(); }
std::unique_ptr<Instruction>& PipelineStage::getInstruction() { return curr_instruction; }

void PipelineStage::setResult(int32_t newResult) {

    if (type != EX) {
        std::cerr << "Should not be setting a result outside of EX stage." << std::endl;
        return;
    }

    if (isEmpty()) {
        std::cerr << "Cannot set result of empty instruction." << std::endl;
        return;
    }

    curr_instruction->setResult(newResult);
}

int32_t PipelineStage::getResult() const {

    if (isEmpty()) {
        std::cerr << "Cannot get result of empty instruction." << std::endl;
        return -1;
    }

    return curr_instruction->getResult();
}

std::unordered_map<DEPENDENCY_TYPE, int32_t> PipelineStage::getRegisterValues() const { 

    std::unordered_map<DEPENDENCY_TYPE, int32_t> nonelol;

    if (isEmpty()) {
        std::cerr << "Cannot get register value of empty instruction." << std::endl;
        return nonelol;
    }

    return curr_instruction->getRegisterValues();
}

void PipelineStage::setRegisterValue(DEPENDENCY_TYPE reg, int32_t newValue) { 

    if (isEmpty()) {
        std::cerr << "Cannot set register value of empty instruction." << std::endl;
        return;
    }

    curr_instruction->setRegisterValue(reg, newValue);
}
    

void PipelineStage::setMemAddress(uint32_t newAddress) {

    if (type != EX) {
        std::cerr << "Should not be setting a memory address outside of EX stage." << std::endl;
        return;
    }

    if (isEmpty()) { 
        std::cerr << "Cannot set memory address of empty instruction." << std::endl;
        return;
    }

    curr_instruction->setMemAddress(newAddress);

}

uint32_t PipelineStage::getMemAddress() const {

    if (isEmpty()) { 
        std::cerr << "Cannot get memory address of empty instruction." << std::endl;
        return -1;
    }

    return curr_instruction->getMemAddress();

}

INST_TYPE PipelineStage::getInstructionType() {
    if (!isEmpty()) { return curr_instruction->getInstType(); }
    std::cerr << "Trying to get type of empty instruction" << std::endl;
    return BLANK;
}


// Get and set current state
void PipelineStage::setState(std::string updatedState) { state = "* " + getStageName() + " : " + updatedState + "\n"; }
std::string PipelineStage::getState() const { return state; }
void PipelineStage::resetState() { setState("NOP"); }

void PipelineStage::updateStatus() {

    // Handle IF
    if (type == IF && !isEmpty()) {
        setState("<unknown>");
        return;
    }
    if (type == IF && isEmpty()) {
        setState("NOP");
        return;
    }

    // Handle IS
    if (type == IF || type == IS) { return; }

    if (isEmpty()) { setState("NOP"); }
    setState(new_style_istring);

    return;
}




// UTILITY


std::string PipelineStage::getStageName() const {
    switch (type) {
        case IF: return "IF";
        case IS: return "IS";
        case ID: return "ID";
        case RF: return "RF";
        case EX: return "EX";
        case DF: return "DF";
        case DS: return "DS";
        case WB: return "WB";
        default: return "Unknown";
    }
}

// Get instruction string
std::string PipelineStage::getInstructionString() {

    if (curr_instruction) {
        return instruction_to_string(*curr_instruction, 0, false) + "\n";
    } 

    return "NOP\n";
    
}


std::string PipelineStage::getNewStyleIstring() const {

    if (curr_instruction) {
        return new_style_istring;
    } 

    return "NOP\n";

}