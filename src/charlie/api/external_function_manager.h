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

#ifndef CHARLIE_API_EXTERNAL_FUNCTION_MANAGER_H_
#define CHARLIE_API_EXTERNAL_FUNCTION_MANAGER_H_

#include <map>
#include <set>
#include <string>
#include <stack>
#include <list>
#include <functional>

#include "..\common\exportDefs.h"

#include "..\program\function_declaration.h"

namespace charlie {
namespace api {
// Registrates external functions to compiler and VM.
// If you do not modify this library, you only need to use the Add(...) methods
// Note: registrate the function in the same order when compiling and running the code.
//  Example:
//      auto manager = ExternalFunctionManager();
//      manager.Add("print", [](int message){
//        std::cout << message;
//      });
//
//      std::list<VariableDeclaration> arguments = { VariableDeclaration(VariableDeclaration::TypeEnum::Int) };
//      int id = manager.GetId(FunctionDec("print", VariableDeclaration::TypeEnum::Void, arguments));
//
//      std::list<int> call_stack = {123};
//      manager.Invoke(0, call_stack);          // Prints the integer 123 to the console
class ExternalFunctionManager {
 public:
  // Creates the empty manager
  xprt ExternalFunctionManager();
  // Adds the function pointer to the registry
  xprt void AddFunction(std::string funcName, std::function<void(void)> funcPointer);
  xprt void AddFunction(std::string funcName, std::function<void(int)> funcPointer);
  xprt void AddFunction(std::string funcName, std::function<void(const char*)> funcPointer);
  // Returns the id of the specified function declaration if found. Otherwise returns -1.
  xprt int GetId(program::FunctionDeclaration const& dec) const;
  // Invokes the function with the specified id. The arguments are stored in "call_stack"
  xprt void Invoke(int id, std::stack<int> *call_stack) const;

 private:
  // Stores all relevant informations of a function registration
  template<class _Fty>
  struct FunctionInfo {
    FunctionInfo(std::function<_Fty> const& pointer, program::FunctionDeclaration const& declaration) :
      pointer(pointer), declaration(declaration) {}
    std::function<_Fty> pointer;
    program::FunctionDeclaration declaration;
  };
  // Saves the function pointers to corresponing id for each function signature
  std::map<int, FunctionInfo<void(void)>> pointers_v_v_;
  std::map<int, FunctionInfo<void(int)>> pointers_v_i_;
  std::map<int, FunctionInfo<void(const char*)>> pointers_v_ccp_;
  // This map is used to get the id of an registrated function by its declaration
  std::map<program::FunctionDeclaration, int, program::FunctionDeclaration::comparer> decs_;
  // Counter of the ids assigned to the registrated funtion pointers
  int id_;
};
}  // namespace api
}  // namespace charlie

#endif  // !CHARLIE_API_EXTERNAL_FUNCTION_MANAGER_H_
