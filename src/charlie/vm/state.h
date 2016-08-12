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


#ifndef CHARLIE_VM_STATE_H
#define CHARLIE_VM_STATE_H

#include <vector>
#include <stack>

#include "register.h"

#include "..\api\external_function_manager.h"

namespace charlie {
namespace vm {
// Represents the state of the VM
struct State {
  // When a function gets called one 
  // needs to save the current function register
  // and back it up after the funcion call finished.
  struct FunctionState {
    // Backup of the frozen function register
    std::stack<int> register_backup;
    // position to continue
    int pos;
  };
  // Creates an object
  State();
  // The ALU stack is used for current calculations.
  std::stack<int> alu_stack;
  // Stores each position where a currently running function call was made.
  // Need to jump back, when the function finished.
  std::stack<int> call_stack;
  // Register stores all current variables.
  //std::vector<int> reg;
  Register reg;
  // The bytecode of the program.
  std::vector<int> program;
  // The current position in the programs bytecode.
  int pos;
  // Pointer to the external function manager.
  const api::ExternalFunctionManager *external_function_manager;
};

}  // namespace vm
}  // namespace charlie

#endif // !CHARLIE_VM_STATE_H

