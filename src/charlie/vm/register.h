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


#include <stack>

#ifndef CHARLIE_VM_REGISTER_H
#define CHARLIE_VM_REGISTER_H

namespace charlie {
namespace vm {

// Manages the register memory used to store variable values.
// Note: Each variable has the size of an integer
class Register {
 public:
  Register();
  ~Register();
  // Increases the memory space when entering a new scope
  // "size": the number of integers
  // Returns false, if an error occured
  bool Increase(int size);
  // Decreases the memory space by the size of the last scope
  // Returns false, if an error occured
  bool Decrease();
  // Removes the scopes from the registry and archives them in a stack
  void StoreFunctionScopes();
  // Loads the last scopes from the stack and append them to the register
  void RestoreFunctionScopes();
  // Gets the value at the spefified index
  // "index": Register index (starting with 0)
  // "value": Pointer to the value where the value should copied to
  // Returns false, if the index exceeds.
  bool GetValue(int index, int *value);
  // Sets the value at the spefified index
  // "index": Register index (starting with 0)
  // "value": The value to which the register should set to
  // Returns false, if the index exceeds.
  bool SetValue(int index, int value);

 private:

  struct FunctionScope {
    FunctionScope();
    ~FunctionScope();
    void clear();
    // Note that they are in reverse order!
    std::stack<int> scope_sizes_;
    int size;
    int *data;
  };

  bool resize(int change, bool updateFunc = true);

  int *data_;
  int size_;

  std::stack<int> scope_sizes_;
  std::stack<FunctionScope> functions_;
  
};

}  // namespace vm
}  // namespace charlie

#endif // !CHARLIE_VM_REGISTER_H