#ifndef PROCESSOR_H
#define PROCESSOR_H

/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2023

   Class for processor

**************************************************************** */

#include "memory.h"

using namespace std;

class processor
{

private:
   // TODO: Add private members here
   unsigned long long int instruction_count;
   uint64_t pc;
   vector<uint64_t> registers;
   uint64_t breakpoint;
   bool breakpoint_bool;
   memory *main_memory;
   bool debugCheck;

   // Instructions
   uint16_t opcode;
   uint16_t funct3;
   uint16_t rd;
   uint16_t rs1;
   uint16_t rs2;
   uint16_t funct7;
   int32_t imm;

public:
   // Consructor
   processor(memory *main_memory, bool verbose, bool stage2);

   // Display PC value
   void show_pc();

   // Set PC to new value
   void set_pc(uint64_t new_pc);

   // Display register value
   void show_reg(unsigned int reg_num);

   // Set register to new value
   void set_reg(unsigned int reg_num, uint64_t new_value);

   // Execute a number of instructions
   void execute(unsigned int num, bool breakpoint_check);

   // Clear breakpoint
   void clear_breakpoint();

   // Set breakpoint at an address
   void set_breakpoint(uint64_t address);

   // Decoding the instruction code
   void decode(char type, uint32_t instruct);

   // Sign extend
   uint64_t imm_extend(uint32_t immediate, int length);

   // Show privilege level
   // Empty implementation for stage 1, required for stage 2
   void show_prv();

   // Set privilege level
   // Empty implementation for stage 1, required for stage 2
   void set_prv(unsigned int prv_num);

   // Display CSR value
   // Empty implementation for stage 1, required for stage 2
   void show_csr(unsigned int csr_num);

   // Set CSR to new value
   // Empty implementation for stage 1, required for stage 2
   void set_csr(unsigned int csr_num, uint64_t new_value);

   uint64_t get_instruction_count();

   // Used for Postgraduate assignment. Undergraduate assignment can return 0.
   uint64_t get_cycle_count();
};

enum opcode_check
{
   op_I = 19,
   op_LOAD = 3,
   op_B = 99,
   op_U = 55,
   op_R = 51,
   op_S = 35,
   op_JAL = 111,
   op_JALR = 103,
   op_I64 = 27,
   op_R64 = 59,
   op_AUIPC = 23,
   op_FENCE = 15,
   op_Exx = 115,
};

#endif
