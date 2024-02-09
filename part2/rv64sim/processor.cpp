/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2023

   Class members for processor

**************************************************************** */

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <exception>

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
   prv = 3;

   csr_reg[csr_mvendroid] = uint64_t(0);
   csr_reg[csr_marchid] = uint64_t(0);
   csr_reg[csr_mhartid] = uint64_t(0);
   csr_reg[csr_mie] = uint64_t(0);
   csr_reg[csr_mtvec] = uint64_t(0);
   csr_reg[csr_mscratch] = uint64_t(0);
   csr_reg[csr_mepc] = uint64_t(0);
   csr_reg[csr_mcause] = uint64_t(0);
   csr_reg[csr_mtval] = uint64_t(0);
   csr_reg[csr_mip] = uint64_t(0);
   csr_reg[csr_mimpid] = uint64_t(0x2018020000000000);
   csr_reg[csr_mstatus] = uint64_t(0x0000000200000000);
   csr_reg[csr_misa] = uint64_t(0x8000000000100100);

   read_only_csr.push_back(csr_mvendroid);
   read_only_csr.push_back(csr_marchid);
   read_only_csr.push_back(csr_mhartid);
   read_only_csr.push_back(csr_mimpid);

   // Instructions
   opcode = 0;
   rs1 = 0;
   rs2 = 0;
   rd = 0;
   csr_num = 0;
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

   for (unsigned int i = 0; i < num; i++)
   {
      uint32_t check_mstatus = csr_reg[csr_mstatus] & 0x8;

      if (check_mstatus || prv == 0)
      {
         // MEI Machine external interrupt
         if (csr_reg[csr_mie] & 0x800 && csr_reg[csr_mip] & 0x800)
            check_interrupt(11, pc);

         // MSI Machine software interrupt
         else if ((csr_reg[csr_mie] & 0x8) && (csr_reg[csr_mip] & 0x8))
            check_interrupt(3, pc);

         // MTI Machine timer interrupt
         else if (csr_reg[csr_mie] & 0x80 && csr_reg[csr_mip] & 0x80)
            check_interrupt(7, pc);

         // UEI User external interrupt
         else if (csr_reg[csr_mie] & 0x100 && csr_reg[csr_mip] & 0x100)
            check_interrupt(8, pc);

         // USI User software interrupt
         else if (csr_reg[csr_mie] & 0x1 && csr_reg[csr_mip] & 0x1)
            check_interrupt(0, pc);

         // UTI User timer interrupt
         else if (csr_reg[csr_mie] & 0x10 && csr_reg[csr_mip] & 0x10)
            check_interrupt(4, pc);
      }

      if (pc % 4 != 0)
      {
         check_exceptions(0, pc); // Instruction MisAddr ex
         continue;
      }
      instruction = main_memory->fetch(pc);
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
         default:
         {
            check_exceptions(2, 0); // Illegal instruction
            continue;
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
         default:
         {
            check_exceptions(2, 0); // Illegal instruction
            continue;
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
               check_exceptions(4, load_add); // Load MisAddr ex
               continue;
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
               check_exceptions(4, load_add); // Load MisAddr ex
               continue;
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
               check_exceptions(4, load_add); // Load MisAddr ex
               continue;
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
               check_exceptions(4, load_add); // Load MisAddr ex
               continue;
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
            if (load_add % 4 != 0)
            {
               check_exceptions(4, load_add); // Load MisAddr ex
               continue;
            }
            data = main_memory->fetch(load_add);
            set_reg(rd, data);
            break;
         }
         default:
         {
            check_exceptions(2, 0); // Illegal instruction
            continue;
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
         default:
         {
            check_exceptions(2, 0); // Illegal instruction
            continue;
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
            else if (funct7 == 32) // Sub
            {
               set_reg(rd, registers[rs1] - registers[rs2]);
            }
            else // Mul
            {
               check_exceptions(2, 0); // Illegal instruction
               continue;
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
         default:
         {
            check_exceptions(2, 0); // Illegal instruction
            continue;
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
               check_exceptions(6, new_address); // Store MisAddr ex
               continue;
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
               check_exceptions(6, new_address); // Store MisAddr ex
               continue;
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
               check_exceptions(6, new_address); // Store MisAddr ex
               continue;
            }
            main_memory->write_doubleword(new_address, registers[rs2], 0xffffffffffffffff);
            break;
         }
         default:
         {
            check_exceptions(2, 0); // Illegal instruction
            continue;
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
         decode('C', instruction);
         if (funct3 == 0)
         {
            switch (csr_num)
            {
            case 0: // Ecall
            {
               if (prv == 0)
                  check_exceptions(8, 0); // User mode ex
               else if (prv == 3)
                  check_exceptions(11, 0); // Machine mode ex
               continue;
            }
            case 1: // Ebreak
            {
               check_exceptions(3, 0); // Breakpoint ex
               continue;
            }
            case 0x302: // Mret
            {
               if (prv == 0)
               {
                  check_exceptions(2, 0); // Illegal instruction ex
                  continue;
               }
               pc = csr_reg[csr_mepc] - 4;
               uint16_t mpie = csr_reg[csr_mstatus] & 0x80;
               uint16_t mpp = (csr_reg[csr_mstatus] >> 11) & 0x3;
               if (mpp == 0)
                  prv = 0;
               else
                  prv = 3;
               set_csr(csr_mstatus, csr_reg[csr_mstatus] & 0xE777);
               set_csr(csr_mstatus, csr_reg[csr_mstatus] | (mpie >> 4));
               set_csr(csr_mstatus, csr_reg[csr_mstatus] | 0x80);
               break;
            }
            default:
            {
               check_exceptions(2, 0); // Illegal instruction ex
               continue;
            }
            }
         }
         else
         {
            if (prv == 0)
            {
               check_exceptions(2, 0); // Illegal instruction ex
               continue;
            }
            if (csr_reg.find(csr_num) == csr_reg.end())
            {
               check_exceptions(2, 0); // Illegal instruction ex
               continue;
            }
            switch (funct3)
            {
            case 1: // CSRRW
            {
               if (rd == 0)
               {
                  set_csr(csr_num, registers[rs1]);
               }
               else
               {
                  set_reg(rd, csr_reg[csr_num]);
                  set_csr(csr_num, registers[rs1]);
               }
               break;
            }
            case 2: // CSRRS
            {
               if (rs1 == 0)
               {
                  set_reg(rd, csr_reg[csr_num]);
               }
               else
               {
                  if (find(read_only_csr.begin(), read_only_csr.end(), csr_num) != read_only_csr.end())
                  {
                     check_exceptions(2, 0); // Illegal instruction
                     continue;
                  }
                  if (!(csr_num == csr_mip && registers[rs1] & 0x888))
                  {
                     set_reg(rd, csr_reg[csr_num]);
                     set_csr(csr_num, csr_reg[csr_num] | registers[rs1]);
                     if (registers[rs1] == 0)
                        check_exceptions(2, 0); // Illegal instruction;
                  }
               }
               break;
            }
            case 3: // CSRRC
            {
               if (rs1 == 0)
               {
                  set_reg(rd, csr_reg[csr_num]);
               }
               else
               {
                  if (find(read_only_csr.begin(), read_only_csr.end(), csr_num) != read_only_csr.end())
                  {
                     check_exceptions(2, 0); // Illegal instruction
                     continue;
                  }
                  if (!(csr_num == csr_mip && registers[rs1] & 0x888))
                  {
                     set_reg(rd, csr_reg[csr_num]);
                     set_csr(csr_num, csr_reg[csr_num] & ~registers[rs1]);
                     if (registers[rs1] == 0)
                        check_exceptions(2, 0); // Illegal instruction;
                  }
               }
               break;
            }
            case 5: // CSRRWI
            {
               if (rd == 0)
               {
                  set_reg(rd, csr_reg[csr_num]);
               }
               else
               {
                  set_reg(rd, csr_reg[csr_num]);
                  set_csr(csr_num, imm);
               }
               break;
            }
            case 6: // CSRRSI
            {
               if (imm == 0)
               {
                  set_reg(rd, csr_reg[csr_num]);
               }
               else
               {
                  set_reg(rd, csr_reg[csr_num]);
                  set_csr(csr_num, imm | csr_reg[csr_num]);
               }
               break;
            }
            case 7: // CSRRCI
            {
               if (imm == 0)
               {
                  set_reg(rd, csr_reg[csr_num]);
               }
               else
               {
                  set_reg(rd, csr_reg[csr_num]);
                  set_csr(csr_num, ~(uint64_t)imm & csr_reg[csr_num]);
               }
               break;
            }
            default:
            {
               check_exceptions(2, 0); // Illegal instruction
               continue;
            }
            }
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
            break;
         }
         default:
         {
            check_exceptions(2, 0); // Illegal instruction
            continue;
         }
         }
         break;
      }
      default:
      {
         check_exceptions(2, 0); // Illegal instruction
         continue;
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
   else if (type == 'C')
   {
      funct3 = (instruct >> 12) & 0x7;
      rd = (instruct >> 7) & 0x1f;
      rs1 = (instruct >> 15) & 0x1f;
      imm = (instruct >> 15) & 0x1f;
      csr_num = (instruct >> 20) & 0xfff; // [11:0]
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

// ========================================================
// Part 2

// Show privilege level
// Empty implementation for stage 1, required for stage 2
void processor::show_prv()
{
   switch (prv)
   {
   case 0:
      cout << prv << " (user)" << endl;
      break;
   case 3:
      cout << prv << " (machine)" << endl;
      break;
   }
}

// Set privilege level
// Empty implementation for stage 1, required for stage 2
void processor::set_prv(unsigned int prv_num)
{
   prv = prv_num;
}

// Display CSR value
// Empty implementation for stage 1, required for stage 2
void processor::show_csr(unsigned int csr_num)
{
   if (csr_reg.find(csr_num) != csr_reg.end())
      cout << setw(16) << setfill('0') << hex << csr_reg[csr_num] << endl;
   else
      cout << "Illegal CSR number" << endl;
}

// Set CSR to new value
// Empty implementation for stage 1, required for stage 2
void processor::set_csr(unsigned int csr_num, uint64_t new_value)
{
   switch (csr_check(csr_num))
   {
   case csr_marchid:
   case csr_mimpid:
   case csr_mhartid:
   case csr_mvendroid:
   {
      cout << "Illegal write to read-only CSR" << endl;
      return;
   }
   case csr_mscratch:
   case csr_mtval:
   {
      csr_reg[csr_num] = new_value;
      break;
   }
   case csr_mstatus:
   {
      csr_reg[csr_num] = (new_value & 0x0000000000001888) | 0x0000000200000000;
      break;
   }
   case csr_mie:
   case csr_mip:
   {
      csr_reg[csr_num] = new_value & 0x999;
      break;
   }
   case csr_mtvec:
   {
      csr_reg[csr_num] = new_value % 2 ? new_value & 0xFFFFFFFFFFFFFF01 : new_value & 0xFFFFFFFFFFFFFFFC;
      break;
   }
   case csr_mepc:
   {
      csr_reg[csr_num] = new_value & 0xFFFFFFFFFFFFFFFC;
      break;
   }
   case csr_mcause:
   {
      csr_reg[csr_num] = new_value & 0x800000000000000F;
      break;
   }
   case csr_misa:
   {
      break;
   }
   }
}

void processor::check_exceptions(int cause_code, uint64_t num)
{
   switch (cause_code)
   {
   case 2: // Illegal instruction
   {
      set_csr(csr_mtval, instruction);
      break;
   }
   case 0: // Instruction Address Misaligned exceptions
   case 4: // Load Address Misaligned exceptions
   case 6: // Store Address Misaligned exceptions
   {
      set_csr(csr_mtval, num);
      break;
   }
   case 3:  // Breakpoint exceptions
   case 8:  // Ecall user mode exceptions
   case 11: // Ecall machine mode exceptions
   {
      break;
   }
   }
   set_csr(csr_mcause, cause_code);
   set_csr(csr_mepc, pc);
   pc = csr_reg[csr_mtvec] & 0xFFFFFFFFFFFFFFFC;
   machine_toggle();
}

void processor::check_interrupt(int cause_code, uint64_t &program_c)
{
   set_csr(csr_mepc, program_c);
   set_csr(csr_mcause, 0x8000000000000000 | cause_code);
   uint32_t mode = csr_reg[csr_mtvec] & 0x3;

   if (mode == 0)
      program_c = csr_reg[csr_mtvec] & 0xFFFFFFFFFFFFFFFC;
   else if (mode == 1)
      program_c = (csr_reg[csr_mtvec] & 0xFFFFFFFFFFFFFFFC) + (cause_code * 4);

   machine_toggle();
}

void processor::machine_toggle()
{
   uint16_t mie = csr_reg[csr_mstatus] & 0x8;
   set_csr(csr_mstatus, (csr_reg[csr_mstatus] >> 17) & 0x1);
   set_csr(csr_mstatus, csr_reg[csr_mstatus] | (mie << 4));
   set_csr(csr_mstatus, csr_reg[csr_mstatus] | (prv << 11));
   prv = 3;
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