#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector> 
#include <unordered_map>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <iomanip>


#include "instruction.h"
#include "pipelinestage.h"

struct PipelineRegisters {

    // IF
    int npc = 496; // Next program counter   [IF/IS.NPC]

    // IS
    uint32_t instruction_register = 0; //   [IS/ID.IR]

    // RF
    int32_t rf_regs_rs = 0; //RF's rs register  [RF/EX.A]
    int32_t rf_regs_rt = 0; //RF's rt register  [RF/EX.B]

    // EX
    int32_t alu_output = 0; // ALU output       [EX/DF.ALUout]
    int32_t forward_val = 0; // Forwarded value [EX/DF.B]

    // DS
    int32_t result_alu = 0; // ALU result/mem   [DS/WB.ALUout-LMD]


    PipelineRegisters() = default;

};

struct Flags {

    bool isRAWStalled = false; //Means that the instruction in ID stage is stalled
    int RAWstallsRemaining = 0;

    bool isBranchStalled = false;
    int branchStallsRemaining = 0;

    StageType stopStage = NONE; // Stage to stop at

    Flags() = default;

};

struct Stats {

    // Total stalls
    int total_loads = 0;
    int total_branches = 0;
    int other = 0;

    std::unordered_map<std::string, int> num_forwards = {
        {"EX/DF -> RF/EX", 0},
        {"DF/DS -> EX/DF", 0},
        {"DF/DS -> RF/EX", 0},
        {"DS/WB -> EX/DF", 0},
        {"DS/WB -> RF/EX", 0}
    };

    Stats() = default;

    std::string toString() const {
        std::ostringstream output;

        // Total Stalls
        output << "Total Stalls:\n";
        output << "* Loads\t\t: " << total_loads << "\n";
        output << "* Branches\t: " << total_branches << "\n";
        output << "* Other\t\t: " << other << "\n";

        output << "\nTotal Forwardings:\n";
        for (const auto& forwarding : num_forwards) {
            output << "* " << forwarding.first << " : " << forwarding.second << "\n";
        }

        return output.str();
    }

    void recordForwarding(std::string forward_type) {

        auto it = num_forwards.find(forward_type);
        if (it != num_forwards.end()) {
            // If the entry exists, increment its value
            it->second++;
        } else {
            // If the entry does not exist, return
            return;
        }

    }


};

struct Forwarding {
    /**
     * For keeping track of forwarding paths
     */

    bool detected = false;
    void setDetected(bool newDetected) {
        detected = newDetected;
    }
    bool getDetected() const {
        return detected;
    }
    std::unordered_map<std::string, std::string> paths_output = {
        {"EX/DF -> RF/EX", "(none)"},
        {"DF/DS -> EX/DF", "(none)"},
        {"DF/DS -> RF/EX", "(none)"},
        {"DS/WB -> EX/DF", "(none)"},
        {"DS/WB -> RF/EX", "(none)"}
    };

    void resetPathsOutput() {
        paths_output = {
            {"EX/DF -> RF/EX", "(none)"},
            {"DF/DS -> EX/DF", "(none)"},
            {"DF/DS -> RF/EX", "(none)"},
            {"DS/WB -> EX/DF", "(none)"},
            {"DS/WB -> RF/EX", "(none)"}
        };
        pending_forwards = {};
        detected = false;
    }

    std::unordered_map<StageType, StageType> paths;

    std::vector<std::pair<Instruction, Instruction>> pending_forwards;

    // and when to actually complete them


    // Convert to string for output with fixed order
    std::string toString() const {
        std::ostringstream output;

        output << "Forwarding:\n";

        if (pending_forwards.empty()) {
            output << " Detected: (none)\n";
        } else {
            output << " Detected:\n";
            for (size_t i = 0; i < pending_forwards.size(); ++i) {
                const auto& [from, to] = pending_forwards[i];
                output << "  [" << i << "] "
                    << "(" << instruction_to_new_style_string(from) << ") to ("
                    << instruction_to_new_style_string(to) << ")\n";
            }
        }

        // Fixed order of paths
        output << " Forwarded:\n";
        output << " * EX/DF -> RF/EX : " << paths_output.at("EX/DF -> RF/EX") << "\n";
        output << " * DF/DS -> EX/DF : " << paths_output.at("DF/DS -> EX/DF") << "\n";
        output << " * DF/DS -> RF/EX : " << paths_output.at("DF/DS -> RF/EX") << "\n";
        output << " * DS/WB -> EX/DF : " << paths_output.at("DS/WB -> EX/DF") << "\n";
        output << " * DS/WB -> RF/EX : " << paths_output.at("DS/WB -> RF/EX") << "\n";

        return output.str();
    }



    void addForward(Instruction from, Instruction to, int cycles_till_execution) {

        pending_forwards.emplace_back(from, to);

        // FIX FIX ?NOTE NOTE
        

    }

    void completeForward(StageType from, StageType to, Instruction from_inst, Instruction to_inst, Stats* stats) {
        paths[from] = to;

        std::string updated_output = "(" + instruction_to_new_style_string(from_inst) + ") to ( " + instruction_to_new_style_string(to_inst) + ")";

        if (from == DF && to == EX) {
            paths_output["EX/DF -> RF/EX"] = updated_output;
            stats->recordForwarding("EX/DF -> RF/EX");
        }

        else if (from == DS && to == DF) {
            paths_output["DF/DS -> EX/DF"] = updated_output;
            stats->recordForwarding("DF/DS -> EX/DF");
        }

        else if (from == DS && to == EX) {
            paths_output["DF/DS -> RF/EX"] = updated_output;
            stats->recordForwarding("DF/DS -> RF/EX");
        }

        else if (from == WB && to == DF) { 
            paths_output["DS/WB -> EX/DF"] = updated_output;
            stats->recordForwarding("DS/WB -> EX/DF");
        }

        else if (from == WB && to == EX) {
            paths_output["DS/WB -> RF/EX"] = updated_output;
            stats->recordForwarding("DS/WB -> RF/EX");
        }

        else {
            std::cerr << "Should not be a forward here. Please check." << std::endl;
            return;
        }




        return;
    }


    Forwarding() = default;

};

class Pipeline {

public:

    // Constructors
    Pipeline();

    // Pipeline advancing methods
    bool sendNextInstruction(); // false if no new instruction to send (ie at end)
    void comprehensiveAdvance();
    void advanceInstruction(StageType from, StageType to, bool deallocate = false);
    bool allPipelineStagesEmpty();



    // Pipeline stage methods
    void ISAction(); //Simulates an instruction fetch
    void instructionDecode(); // Simulates ID stage

    // Stall Checks
    StageType checkDataHazard(DEPENDENCY_TYPE reg, uint32_t register_dependency);
    StageType checkRAWHazard(DEPENDENCY_TYPE reg, uint32_t register_dependency);

    void addDetectedForward(StageType to, StageType from, DEPENDENCY_TYPE dep);

    // Cancelling instructions
    void cancelInstruction(StageType stage);


    void registerFetch(); // Simulates RF stage

    // Helper specific functions for RF stage
    void registerFetchJType();
    void registerFetchLoadStore();
    void registerFetchRType();
    void registerFetchIRR();
    void registerFetchBranch();


    void executeInstruction(); // Performs computation currently in EX stage
    void dataFetch(); // Simulates DF stage
    void dataStore(); // Simulates DS stage
    void writeBack(); // Simulates WB stage

    void handleStalledState(); // Handles stalled state
    void setRAWStall(int numCycles, StageType stopStage);



    // For executing instructions by type
    void executeIRR(); // RENAME THIS
    void executeRType(); // RENAME THIS
    void executeLoad();
    void executeStore();
    void executeJType();
    void executeBranch();

    uint32_t getForwardedValue(StageType stage, DEPENDENCY_TYPE dep);
    bool isValidForward(StageType from, StageType to);

    // Memory access helper functions
    bool setDataMemory(uint32_t address, int32_t data);
    int32_t getDataMemory(uint32_t address);
    // Make some more for integer registers

    // To string methods
    std::string getCycleOutput();
    std::string getPipelineStatusOutput();
    std::string getIntegerRegistersOutput();
    std::string getPipelineRegistersOutput() const;
    std::string getDataMemoryOutput() const;
    std::string getStalledInstruction();
    std::string getPCOutput();


    // Consuming from lexer
    void addInstruction(Instruction instruction);

    // Methods for interacting with integer registers
    void setIntegerRegister(uint32_t register_num, int32_t val);
    int32_t getIntegerRegister(uint32_t register_num);



private:

    int curr_cycle = 0;

    // Pipeline Registers
    PipelineRegisters pipeline_registers;

    // Flags
    Flags flags;

    // Stats
    Stats stats;

    // Forwarding
    Forwarding forwarding;

    // instruction_index represents the index of the next instruction to be sent
    int instruction_index = 0;

    std::vector<Instruction> instructions; // Raw instructions recieved from lexer

    std::unordered_map<StageType, PipelineStage> stages;  // The 8 pipeline stages

    std::unordered_map<std::string, int32_t> integer_registers; // Integer registers

    std::unordered_map<uint32_t, int32_t> data_memory;

    std::unordered_map<int, Instruction> instruction_map; //Maps PC to instruction

    int pc = 492; // Program counter

};




#endif