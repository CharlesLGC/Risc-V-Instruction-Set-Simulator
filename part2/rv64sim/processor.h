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
   uint32_t instruction;
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
   uint32_t imm_flag;
   uint16_t csr_num;

   //================================
   // Stage 2

   // Machine privilege level
   int prv;

   // CSR register
   unordered_map<unsigned int, uint64_t> csr_reg;

   // array to store read-only csr
   vector<uint32_t> read_only_csr;

public:
   // Consructor
   processor(memory *main_memory, bool verbose, bool stage2);

   // Exceptions handler
   void check_exceptions(int cause_code, uint64_t num);

   // Interrupt handler
   void check_interrupt(int cause_code, uint64_t &program_c);

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

   // Set MPIE to MIE, MIE to 0, MPP to prv
   void machine_toggle();

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

enum csr_check
{
   csr_mvendroid = 0xF11,
   csr_marchid = 0xF12,
   csr_mimpid = 0xF13,
   csr_mhartid = 0xF14,
   csr_mstatus = 0x300,
   csr_misa = 0x301,
   csr_mie = 0x304,
   csr_mtvec = 0x305,
   csr_mscratch = 0x340,
   csr_mepc = 0x341,
   csr_mcause = 0x342,
   csr_mtval = 0x343,
   csr_mip = 0x344,
};

#endif
