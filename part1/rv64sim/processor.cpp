/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2023

   Class members for processor

**************************************************************** */

#include <iostream>
#include <iomanip>
#include <stdlib.h>

#include "memory.h"
#include "processor.h"

using namespace std;

// Consructor
processor::processor(memory *main_memory, bool verbose, bool stage2)
{
   // TODO
   debugCheck = verbose;
   if (verbose)
   {
      cout << "Processor Debug Check" << endl;
   }
   instruction_count = 0;
   pc = 0;
   breakpoint = 0;
   registers.resize(32);
   this->main_memory = main_memory;

   // Instructions
   opcode = 0;
   rs1 = 0;
   rs2 = 0;
   rd = 0;
}

// TODO
// Display PC value
void processor::show_pc()
{
   cout << setw(16) << setfill('0') << hex << pc << endl;
}

// Set PC to new value
void processor::set_pc(uint64_t new_pc)
{
   pc = new_pc;
}

// Display register value
void processor::show_reg(unsigned int reg_num)
{
   if (reg_num == 0)
   {
      cout << setw(16) << setfill('0') << hex << 0 << endl;
   }
   else
   {
      cout << setw(16) << setfill('0') << hex << registers[reg_num] << endl;
   }
}

// Set register to new value
void processor::set_reg(unsigned int reg_num, uint64_t new_value)
{
   if (reg_num == 0)
   {
      registers[reg_num] = 0;
   }
   else
   {
      registers[reg_num] = new_value;
   }
}

// Execute a number of instructions
void processor::execute(unsigned int num, bool breakpoint_check)
{
   if (pc % 4 != 0)
   {
      cout << "Error: misaligned pc" << endl;
      return;
   }

   for (unsigned int i = 0; i < num; i++)
   {
      uint32_t instruction = main_memory->fetch(pc);
      if (breakpoint_check && breakpoint_bool && (pc == breakpoint))
      {
         cout << "Breakpoint reached at " << setw(16) << setfill('0') << hex << breakpoint << endl;
         break;
      }

      opcode = (instruction & 0x7f);

      switch (opcode)
      {
      case op_I: // Type I
      {
         decode('I', instruction);
         switch (funct3)
         {
         case 0: // Addi
         {
            set_reg(rd, registers[rs1] + imm_extend(imm, 12));
            break;
         }
         case 1: // Slli
         {
            imm = imm & 0x3f;
            set_reg(rd, (registers[rs1] << imm));
            break;
         }
         case 2: // Slti
         {
            if ((int64_t)registers[rs1] < (int64_t)imm_extend(imm, 12))
            {
               set_reg(rd, 1);
            }
            else
            {
               set_reg(rd, 0);
            }
            break;
         }
         case 3: // Sltiu
         {
            if (registers[rs1] < imm_extend(imm, 12))
            {
               set_reg(rd, 1);
            }
            else
            {
               set_reg(rd, 0);
            }
            break;
         }
         case 4: // Xori
         {
            set_reg(rd, (registers[rs1] ^ imm_extend(imm, 12)));
            break;
         }
         case 6: // Ori
         {
            set_reg(rd, (registers[rs1] | imm_extend(imm, 12)));
            break;
         }
         case 7: // Andi
         {
            set_reg(rd, (registers[rs1] & imm_extend(imm, 12)));
            break;
         }
         case 5:
         {
            if ((imm >> 6) == 0) // Srli
            {
               imm = imm & 0x3f;
               set_reg(rd, (registers[rs1] >> imm));
            }
            else // Srai
            {
               imm = imm & 0x3f;
               set_reg(rd, ((int64_t)registers[rs1] >> imm));
            }
            break;
         }
         }
         break;
      }
      case op_U: // Type U (LUI)
      {
         decode('U', instruction);
         set_reg(rd, imm_extend(imm, 20) << 12);
         break;
      }
      case op_B: // Type B
      {
         decode('B', instruction);
         switch (funct3)
         {
         case 1: // Bne
         {
            if (registers[rs1] != registers[rs2])
            {
               pc += imm_extend(imm, 13) - 4;
            }
            break;
         }
         case 0: // Beq
         {
            if (registers[rs1] == registers[rs2])
            {
               pc += imm_extend(imm, 13) - 4;
            }
            break;
         }
         case 4: // Blt
         {
            if ((int32_t)registers[rs1] < (int32_t)registers[rs2])
            {
               pc += imm_extend(imm, 13) - 4;
            }
            break;
         }
         case 5: // Bge
         {
            if ((int32_t)registers[rs1] >= (int32_t)registers[rs2])
            {
               pc += imm_extend(imm, 13) - 4;
            }
            break;
         }
         case 6: // Bltu
         {
            if (registers[rs1] < registers[rs2])
            {
               pc += imm_extend(imm, 13) - 4;
            }
            break;
         }
         case 7: // Bgeu
         {
            if (registers[rs1] >= registers[rs2])
            {
               pc += imm_extend(imm, 13) - 4;
            }
            break;
         }
         }
         break;
      }
      case op_LOAD:
      {
         decode('I', instruction);
         switch (funct3)
         {
            uint32_t load_add;
            uint64_t data;
         case 0: // Lb
         {
            load_add = registers[rs1] + imm_extend(imm, 12);
            data = main_memory->read_doubleword(load_add);
            int remainder = (load_add % 8);
            data = (data >> (remainder * 8)) & 0xff;
            set_reg(rd, imm_extend(data, 8));
            break;
         }
         case 1: // Lh
         {
            load_add = registers[rs1] + imm_extend(imm, 12);
            if (load_add % 2 != 0)
            {
               cout << "Error: misaligned address for lh" << endl;
            }
            data = main_memory->read_doubleword(load_add);
            int remainder = (load_add % 8);
            remainder -= (remainder % 2);
            data = (data >> (remainder * 8)) & 0xffff;
            set_reg(rd, imm_extend(data, 16));
            break;
         }
         case 2: // Lw
         {
            load_add = registers[rs1] + imm_extend(imm, 12);
            if (load_add % 4 != 0)
            {
               cout << "Error: misaligned address for lw" << endl;
            }
            data = main_memory->fetch(load_add);
            set_reg(rd, imm_extend(data, 32));
            break;
         }
         case 3: // Ld
         {
            load_add = registers[rs1] + imm_extend(imm, 12);
            if (load_add % 8 != 0)
            {
               cout << "Error: misaligned address for ld" << endl;
            }
            data = main_memory->read_doubleword(load_add);
            set_reg(rd, data);
            break;
         }
         case 4: // Lbu
         {
            load_add = registers[rs1] + imm_extend(imm, 12);
            data = main_memory->read_doubleword(load_add);
            int remainder = (load_add % 8);
            data = (data >> (remainder * 8)) & 0xff;
            set_reg(rd, data);
            break;
         }
         case 5: // Lhu
         {
            load_add = registers[rs1] + imm_extend(imm, 12);
            if (load_add % 2 != 0)
            {
               cout << "Error: misaligned address for lhu" << endl;
            }
            data = main_memory->read_doubleword(load_add);
            int remainder = (load_add % 8);
            remainder -= (remainder % 2);
            data = (data >> (remainder * 8)) & 0xffff;
            set_reg(rd, data);
            break;
         }
         case 6: // Lwu
         {
            load_add = registers[rs1] + imm_extend(imm, 12);
            data = main_memory->fetch(load_add);
            set_reg(rd, data);
            break;
         }
         }
         break;
      }
      case op_AUIPC: // Auipc
      {
         decode('U', instruction);
         imm = imm << 12;
         set_reg(rd, pc + imm_extend(imm, 32));
         break;
      }
      case op_JAL: // Jal
      {
         decode('J', instruction);
         set_reg(rd, pc + 4);
         pc = pc + imm_extend(imm, 20) - 4;

         if (pc % 2 != 0)
         {
            pc--;
         }
         break;
      }
      case op_JALR: // jalr
      {
         decode('I', instruction);
         uint64_t temp = pc + 4;
         pc = registers[rs1] + imm_extend(imm, 12) - 4;
         set_reg(rd, temp);

         if (pc % 2 != 0)
         {
            pc--;
         }

         // cout << "PC: " << pc << endl;
         break;
      }
      case op_I64: // Type I RV64 ext.
      {
         decode('I', instruction);
         switch (funct3)
         {
         case 0: // Addiw
         {
            uint32_t answer = registers[rs1] + imm_extend(imm, 12);
            set_reg(rd, imm_extend(answer, 32));
            break;
         }
         case 1: // Slliw (XX Question)
         {
            imm = imm & 0x3f;
            set_reg(rd, ((int32_t)registers[rs1] << imm));
            break;
         }
         case 5:
         {
            if ((imm >> 6) == 0) // Srliw (XX Question)
            {
               imm = imm & 0x3f;
               set_reg(rd, (imm_extend((uint32_t)registers[rs1] >> imm, 32)));
               break;
            }
            else // Sraiw
            {
               imm = imm & 0x3f;
               set_reg(rd, ((int32_t)registers[rs1] >> imm));
            }
            break;
         }
         }
         break;
      }
      case op_R: // Type R
      {
         decode('R', instruction);
         switch (funct3)
         {
         case 0:
         {
            if (funct7 == 0) // Add
            {
               set_reg(rd, registers[rs1] + registers[rs2]);
            }
            else // Sub
            {
               set_reg(rd, registers[rs1] - registers[rs2]);
            }
            break;
         }
         case 2: // Slt
         {
            if ((int32_t)registers[rs1] < (int32_t)registers[rs2])
            {
               set_reg(rd, 1);
            }
            else
            {
               set_reg(rd, 0);
            }
            break;
         }
         case 3: // Sltu
         {
            if (registers[rs1] < registers[rs2])
            {
               set_reg(rd, 1);
            }
            else
            {
               set_reg(rd, 0);
            }
            break;
         }
         case 4: // Xor
         {
            set_reg(rd, (registers[rs1] ^ registers[rs2]));
            break;
         }
         case 6: // Or
         {
            set_reg(rd, (registers[rs1] | registers[rs2]));
            break;
         }
         case 7: // And
         {
            set_reg(rd, (registers[rs1] & registers[rs2]));
            break;
         }
         case 1: // Sll
         {
            set_reg(rd, (registers[rs1] << registers[rs2]));
            break;
         }
         case 5:
         {
            if (funct7 == 0) // Srl
            {
               set_reg(rd, (registers[rs1] >> registers[rs2]));
               break;
            }
            else // Sra
            {
               set_reg(rd, ((int64_t)registers[rs1] >> registers[rs2]));
               break;
            }
         }
         }
         break;
      }
      case op_S: // Type S
      {
         decode('S', instruction);
         uint32_t new_address = registers[rs1] + imm_extend(imm, 12);
         uint64_t new_mask;
         switch (funct3)
         {
         case 0: // Sb
         {
            new_mask = 0xff;
            int remainder = new_address % 8;
            uint64_t data = (registers[rs2] & 0xff) << (remainder * 8);
            new_mask = new_mask << (remainder * 8);
            main_memory->write_doubleword(new_address, data, new_mask);
            break;
         }
         case 1: // Sh
         {
            if (new_address % 2 != 0)
            {
               cout << "Error: misaligned address for sh" << endl;
            }
            new_mask = 0xffff;
            int remainder = new_address % 8;
            uint64_t data = (registers[rs2] & 0xffff);
            remainder -= (remainder % 2);
            new_mask = new_mask << (remainder * 8);
            data = data << (remainder * 8);
            main_memory->write_doubleword(new_address, data, new_mask);
            break;
         }
         case 2: // Sw
         {
            if (new_address % 4 != 0)
            {
               cout << "Error: misaligned address for sw" << endl;
            }
            new_mask = 0xffffffff;
            int remainder = new_address % 8;
            remainder -= (remainder % 4);
            new_mask = new_mask << (remainder * 8);
            uint64_t data = registers[rs2] & 0xffffffff;
            data = data << (remainder * 8);
            main_memory->write_doubleword(new_address, data, new_mask);
            break;
         }
         case 3: // Sd
         {
            if (new_address % 8 != 0)
            {
               cout << "Error: misaligned address for sd" << endl;
            }
            main_memory->write_doubleword(new_address, registers[rs2], 0xffffffffffffffff);
            break;
         }
         }
         break;
      }
      case op_FENCE: // Type FENCE
      {
         // Do nothing
         break;
      }
      case op_Exx: // Type ECALL & EBREAK
      {
         decode('I', instruction);
         if (imm == 0) // Ecall
         {
            cout << "ecall: not implemented" << endl;
         }
         else // Ebreak
         {
            cout << "ebreak: not implemented" << endl;
         }
         break;
      }
      case op_R64: // Type R 64 ext.
      {
         decode('R', instruction);
         switch (funct3)
         {
         case 0:
         {
            if (funct7 == 0) // Addw
            {
               set_reg(rd, imm_extend((registers[rs1] + registers[rs2]), 32));
            }
            else // Subw
            {
               set_reg(rd, imm_extend((registers[rs1] - registers[rs2]), 32));
            }
            break;
         }
         case 1: // Sllw
         {
            set_reg(rd, imm_extend(((uint32_t)registers[rs1] << registers[rs2]), 32));
            break;
         }
         case 5:
         {
            if (funct7 == 0) // Srlw
            {
               set_reg(rd, imm_extend(((uint32_t)registers[rs1] >> registers[rs2]), 32));
               break;
            }
            else // Sraw
            {
               set_reg(rd, ((int32_t)registers[rs1] >> registers[rs2]));
               break;
            }
         }
         }
         break;
      }
      }

      instruction_count++;
      pc += 4;
   }
}

// Clear breakpoint
void processor::clear_breakpoint()
{
   breakpoint_bool = false;
   breakpoint = 0;
}

// Set breakpoint at an address
void processor::set_breakpoint(uint64_t address)
{
   breakpoint_bool = true;
   breakpoint = address;
}

// Show privilege level
// Empty implementation for stage 1, required for stage 2
void processor::show_prv()
{
}

// Set privilege level
// Empty implementation for stage 1, required for stage 2
void processor::set_prv(unsigned int prv_num)
{
}

// Display CSR value
// Empty implementation for stage 1, required for stage 2
void processor::show_csr(unsigned int csr_num)
{
}

// Set CSR to new value
// Empty implementation for stage 1, required for stage 2
void processor::set_csr(unsigned int csr_num, uint64_t new_value)
{
}

uint64_t processor::get_instruction_count()
{
   return instruction_count;
}

// Used for Postgraduate assignment. Undergraduate assignment can return 0.
uint64_t processor::get_cycle_count()
{
   return 0;
}

void processor::decode(char type, uint32_t instruct)
{
   if (type == 'I')
   {
      funct3 = (instruct >> 12) & 0x7;
      rd = (instruct >> 7) & 0x1f;
      rs1 = (instruct >> 15) & 0x1f;
      imm = (instruct >> 20) & 0xfff; // [11:0]
      // cout << setw(16) << setfill('0') << hex << rd << "\t" << setw(16) << setfill('0') << hex << funct3 << "\t" << setw(16) << setfill('0') << hex << rs1 << "\t" << setw(16) << setfill('0') << hex << imm << "\t" << endl;
   }
   else if (type == 'U')
   {
      rd = (instruct >> 7) & 0x1f;
      imm = (instruct >> 12); // [31:12]
   }
   else if (type == 'B')
   {
      funct3 = (instruct >> 12) & 0x7;
      rs1 = (instruct >> 15) & 0x1f;
      rs2 = (instruct >> 20) & 0x1f;
      imm = (instruct >> 31);                     //[12]
      imm = (imm << 1) | (instruct >> 7 & 0x1);   //[11]
      imm = (imm << 6) | (instruct >> 25 & 0x3f); // 10:5
      imm = (imm << 4) | (instruct >> 8 & 0xf);   //[4:1]
      imm = imm << 1;                             //[0]
   }
   else if (type == 'R')
   {
      funct3 = (instruct >> 12) & 0x7;
      rd = (instruct >> 7) & 0x1f;
      rs1 = (instruct >> 15) & 0x1f;
      rs2 = (instruct >> 20) & 0x1f;
      funct7 = (instruct >> 25) & 0x7f;
   }
   else if (type == 'S')
   {
      funct3 = (instruct >> 12) & 0x7;
      rs1 = (instruct >> 15) & 0x1f;
      rs2 = (instruct >> 20) & 0x1f;
      imm = (instruct >> 25) & 0x7f;             // [11:5]
      imm = (imm << 5) | (instruct >> 7 & 0x1f); // [4:0]
   }
   else if (type == 'J')
   {
      rd = (instruct >> 7) & 0x1f;

      imm = (instruct >> 31);                       // [20]
      imm = (imm << 8) | (instruct >> 12 & 0xff);   // [19:12]
      imm = (imm << 1) | (instruct >> 20 & 0x1);    // [11]
      imm = (imm << 10) | (instruct >> 21 & 0x3ff); // [10:1]
      imm = imm << 1;
   }
}

uint64_t processor::imm_extend(uint32_t immediate, int length)
{
   uint64_t answer = immediate;
   if (length == 12)
   {
      if ((immediate >> 11 & 0x1) == 1)
      {
         answer = immediate | 0xfffffffffffff000;
      }
   }
   else if (length == 20)
   {
      if ((immediate >> 19 & 0x1) == 1)
      {
         answer = immediate | 0xfffffffffff00000;
      }
   }
   else if (length == 13)
   {
      if ((immediate >> 12 & 0x1) == 1)
      {
         answer = immediate | 0xffffffffffffe000;
      }
   }
   else if (length == 32)
   {
      if ((immediate >> 31 & 0x1) == 1)
      {
         answer = immediate | 0xffffffff00000000;
      }
   }
   else if (length == 8)
   {
      if ((immediate >> 7 & 0x1) == 1)
      {
         answer = immediate | 0xffffffffffffff00;
      }
   }
   else if (length == 16)
   {
      if ((immediate >> 15 & 0x1) == 1)
      {
         answer = immediate | 0xffffffffffff0000;
      }
   }
   return answer;
}