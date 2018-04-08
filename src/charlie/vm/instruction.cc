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

#include "instruction.h"

namespace charlie {
namespace vm {

using std::array;
using std::queue;

array<functionType, InstructionEnums::Length> InstructionManager::Create() {
  array<functionType, InstructionEnums::Length> types = array<functionType, InstructionEnums::Length>();
  types[InstructionEnums::IncreaseRegister] = [](State& state) {
    int addition = state.program[++state.pos];
    state.reg.Increase(addition);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::DecreaseRegister] = [](State& state) {
    if (!state.reg.Decrease()) return -1;
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::Push] = [](State& state) {
    int address = state.program[++state.pos];
    int value;
    if (!state.reg.GetValue(address, &value)) return -1;
    state.alu_stack.push(value);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::PushConst] = [](State& state) {
    int i = state.program[++state.pos];
    state.alu_stack.push(i);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntPop] = [](State& state) {
    int address = state.program[++state.pos];
    if (state.alu_stack.empty()) return -1;
    int value = state.alu_stack.top();
    state.alu_stack.pop();

    if (!state.reg.SetValue(address, value)) return -1;
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::Call] = [](State& state) {
    state.call_stack.push(state.pos + 2);
    int address = state.program[++state.pos];
    state.pos = address;
    state.reg.StoreFunctionScopes();
    return 0;
  };

  types[InstructionEnums::Jump] = [](State& state) {
    int address = state.program[++state.pos];

    state.pos = address;
    return 0;
  };

  types[InstructionEnums::JumpIf] = [](State& state) {
    int elseAddress = state.alu_stack.top();
    state.alu_stack.pop();
    int condition = state.alu_stack.top();
    state.alu_stack.pop();

    if (condition != 0)
      ++state.pos;
    else
      state.pos = elseAddress;
    return 0;
  };

  types[InstructionEnums::Return] = [](State& state) {
    // Only for testing
    if (state.call_stack.empty()) {
      state.pos = -2;
      return -1;
    } else {
      state.pos = state.call_stack.top();
      state.reg.RestoreFunctionScopes();
    }
    state.call_stack.pop();
    return 0;
  };

  types[InstructionEnums::CallEx] = [](State& state) {
    int id = state.program[++state.pos];
    if (state.external_function_manager != nullptr) state.external_function_manager->Invoke(id, &state.alu_stack);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::Exit] = [](State& state) {
    state.pos = -1;
    return 0;
  };

  types[InstructionEnums::IntCopy] = [](State& state) {
    int value = state.alu_stack.top();
    state.alu_stack.pop();
    int address = state.program[++state.pos];

    if (!state.reg.SetValue(address, value)) return -1;

    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntPop] = [](State& state) {
    int value = state.alu_stack.top();
    state.alu_stack.pop();
    int address = state.program[++state.pos];

    if (!state.reg.SetValue(address, value)) return -1;

    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntAdd] = [](State& state) {
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a + b);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntSubstract] = [](State& state) {
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a - b);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntMultiply] = [](State& state) {
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a * b);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntDivide] = [](State& state) {
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a / b);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntModulo] = [](State& state) {
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a % b);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntIncrease] = [](State& state) {
    int address = state.program[++state.pos];

    int value;
    if (!state.reg.GetValue(address, &value)) return -1;

    ++value;
    state.reg.SetValue(address, value);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntDecrease] = [](State& state) {
    int address = state.program[++state.pos];

    int value;
    if (!state.reg.GetValue(address, &value)) return -1;

    --value;
    state.reg.SetValue(address, value);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntEqual] = [](State& state) {
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a == b ? 1 : 0);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntNotEqual] = [](State& state) {
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a != b ? 1 : 0);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntGreater] = [](State& state) {
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a > b ? 1 : 0);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntGreaterEqual] = [](State& state) {
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a >= b ? 1 : 0);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntLess] = [](State& state) {
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a < b ? 1 : 0);
    ++state.pos;
    return 0;
  };

  types[InstructionEnums::IntLessEqual] = [](State& state) {
    int b = state.alu_stack.top();
    state.alu_stack.pop();
    int a = state.alu_stack.top();
    state.alu_stack.pop();
    state.alu_stack.push(a <= b ? 1 : 0);
    ++state.pos;
    return 0;
  };

  return types;
}

functionType InstructionManager::Get(InstructionEnums bc) { return InstructionManager::Instructions[bc]; }
void InstructionManager::GetLegend(int instruction, queue<const char*>* comments) {
  switch (instruction) {
    case InstructionEnums::IncreaseRegister:
      comments->push("Increases the register space ...");
      comments->push("... size to increase");
      break;
    case InstructionEnums::DecreaseRegister:
      comments->push("Decreases the register space");

      break;
    case InstructionEnums::Push:
      comments->push("Pushs the value ...");
      comments->push("... at address");
      break;
    case InstructionEnums::PushConst:
      comments->push("Pushs a constant ...");
      comments->push("... value to push");
      break;
    case InstructionEnums::IntPop:
      comments->push("Pops an integer from stack and copies to register ...");
      comments->push("... at address");
      break;
    case InstructionEnums::Call:
      comments->push("Calls function ...");
      comments->push("... address of function");
      break;
    case InstructionEnums::CallEx:
      comments->push("Calls external function ...");
      comments->push("... Id of function");
      break;
    case InstructionEnums::Jump:
      comments->push("Jumps ...");
      comments->push("... address to jump");
      break;
    case InstructionEnums::JumpIf:
      comments->push("Jumps if the condition is not zero");
      break;
    case InstructionEnums::Return:
      comments->push("Returns");
      break;
    case InstructionEnums::IntCopy:
      comments->push("Copies integer to register ...");
      comments->push("... at address");
      break;
    case InstructionEnums::IntAdd:
      comments->push("Adds two integers");
      break;
    case InstructionEnums::IntSubstract:
      comments->push("Substracts two integers");
      break;
    case InstructionEnums::IntMultiply:
      comments->push("Muliplies two integers");
      break;
    case InstructionEnums::IntDivide:
      comments->push("Divides two integers");
      break;
    case InstructionEnums::IntModulo:
      comments->push("Modulo of two integers");
      break;
    case InstructionEnums::IntIncrease:
      comments->push("Increases an integer ...");
      comments->push("... at address");
      break;
    case InstructionEnums::IntDecrease:
      comments->push("Decreases an integer ...");
      comments->push("... at address");
      break;
    case InstructionEnums::IntEqual:
      comments->push("Compares to integers on equility");
      break;
    case InstructionEnums::IntNotEqual:
      comments->push("Compares to integers on non-equility");
      break;
    case InstructionEnums::IntGreater:
      comments->push("Compares if the first integer is greater than the second");
      break;
    case InstructionEnums::IntGreaterEqual:
      comments->push("Compares if the first integer is greater or equal than the second");
      break;
    case InstructionEnums::IntLess:
      comments->push("Compares if the first integer is less than the second");
      break;
    case InstructionEnums::IntLessEqual:
      comments->push("Compares if the first integer is less or equal than the second");
      break;
    case InstructionEnums::Exit:
      comments->push("Exit program");
      break;
    default:
      break;
  }
}
const array<functionType, InstructionEnums::Length> InstructionManager::Instructions = InstructionManager::Create();

}  // namespace vm
}  // namespace charlie
