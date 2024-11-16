#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector> 
#include <unordered_map>

#include "instruction.h"
#include "pipelinestage.h"

struct PipelineRegisters {

    // IF
    int npc = 500; // Next program counter   [IF/IS.NPC]

    // IS
    uint32_t instruction_register = 0; //   [IS/ID.IR]

    // RF
    int rf_regs_rs = 0; //RF's rs register  [RF/EX.A]
    int rf_regs_rt = 0; //RF's rt register  [RF/EX.B]

    // EX
    int alu_output = 0; // ALU output       [EX/DF.ALUout]
    int forward_val = 0; // Forwarded value [EX/DF.B]

    // DS
    int result_alu = 0; // ALU result/mem   [DS/WB.ALUout-LMD]


    PipelineRegisters() = default;

};

struct Flags {

    bool isStalled = false; //Means that the instruction in ID stage is stalled

    Flags() = default;

};

struct Stats {

    // Total stalls
    int total_loads = 0;
    int total_branches = 0;
    int other = 0;

    // Total forwardings
    int ex_df_to_rf_ex = 0;
    int df_ds_to_ex_df = 0;
    int df_ds_to_rf_ex = 0;
    int ds_wb_to_ex_df = 0;
    int ds_wb_to_rf_ex = 0;

    Stats() = default;

    std::string toString() const {
        std::ostringstream output;

        // Total Stalls
        output << "Total Stalls:\n";
        output << "* Loads\t\t: " << total_loads << "\n";
        output << "* Branches\t: " << total_branches << "\n";
        output << "* Other\t\t: " << other << "\n";

        // Total Forwardings
        output << "\nTotal Forwardings:\n";
        output << "* EX/DF -> RF/EX : " << ex_df_to_rf_ex << "\n";
        output << "* DF/DS -> EX/DF : " << df_ds_to_ex_df << "\n";
        output << "* DF/DS -> RF/EX : " << df_ds_to_rf_ex << "\n";
        output << "* DS/WB -> EX/DF : " << ds_wb_to_ex_df << "\n";
        output << "* DS/WB -> RF/EX : " << ds_wb_to_rf_ex << "\n";

        return output.str();
    }
};

struct Forwarding {
    /**
     * For keeping track of forwarding paths
     */

    bool detected = false;
    std::unordered_map<std::string, std::string> paths = {
        {"EX/DF -> RF/EX", "(none)"},
        {"DF/DS -> EX/DF", "(none)"},
        {"DF/DS -> RF/EX", "(none)"},
        {"DS/WB -> EX/DF", "(none)"},
        {"DS/WB -> RF/EX", "(none)"}
    };


    // Convert to string for output with fixed order
    std::string toString() const {
        std::ostringstream output;

        output << "Forwarding:\n";
        output << " Detected: " << (detected ? "Yes" : "(none)") << "\n";
        output << " Forwarded:\n";

        // Fixed order of paths
        output << " * EX/DF -> RF/EX : " << paths.at("EX/DF -> RF/EX") << "\n";
        output << " * DF/DS -> EX/DF : " << paths.at("DF/DS -> EX/DF") << "\n";
        output << " * DF/DS -> RF/EX : " << paths.at("DF/DS -> RF/EX") << "\n";
        output << " * DS/WB -> EX/DF : " << paths.at("DS/WB -> EX/DF") << "\n";
        output << " * DS/WB -> RF/EX : " << paths.at("DS/WB -> RF/EX") << "\n";

        return output.str();
    }


    Forwarding() = default;

};

class Pipeline {

public:

    // Constructors
    Pipeline();

    // Pipeline advancing methods
    void sendNextInstruction();

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

    std::unordered_map<std::string, int> integer_registers; // Integer registers

    std::unordered_map<int, int> data_memory;

};




#endif