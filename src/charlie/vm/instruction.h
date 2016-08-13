/*
* Copyright (c) 2016, Matthias Lochbrunner <matthias_lochbrunner@live.de>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/

#ifndef CHARLIE_VM_INSTRUCTION_H
#define CHARLIE_VM_INSTRUCTION_H

#include <functional>

#include <array>
#include <queue>

#include "state.h"

#include "..\api\external_function_manager.h"

namespace charlie {
namespace vm {

// Enum of all kind of bytecodes
enum InstructionEnums {
  IncreaseRegister,
  DecreaseRegister,
  Push,
  PushConst,
  IntPop,
  Call,
  CallEx,
  Jump,
  JumpIf,
  Return,
  IntCopy,
  IntAdd,
  IntSubstract,
  IntMultiply,
  IntDivide,
  IntModulo,
  IntIncrease,
  IntDecrease,
  IntEqual,
  IntNotEqual,
  IntGreater,
  IntGreaterEqual,
  IntLess,
  IntLessEqual,
  Exit,
  Length
};
// Type of callback function of each instruction
typedef std::function<int(State&)> functionType;
// The instruction manager does the mass of work
// of the VM: It manages all the instructions which
// get "called" by the bytecode.
class InstructionManager {
 public:
  // creates the instruction array.
  static std::array<functionType, InstructionEnums::Length> Create();
  // Returns the instruction to the specified bytecode.
  static functionType Get(InstructionEnums bc);
  // Returns the legend to the specified bytecode.
  // Used when saving the program as a textfile.
  static void GetLegend(int instruction, std::queue<const char*> *comments);
  // Stores all the instructions.
  static const std::array<functionType, InstructionEnums::Length> Instructions;
};
}  // namespace vm
}  // namespace charlie


#endif  // !CHARLIE_VM_INSTRUCTION_H
