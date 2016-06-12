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

#include "external_function_manager.h"

#include <sstream>
#include <utility>

namespace charlie {
namespace api {

using std::list;
using std::string;
using std::function;
using std::make_pair;

using program::VariableDeclaration;
using program::FunctionDeclaration;


ExternalFunctionManager::ExternalFunctionManager() :
  id_(0),
  pointers_v_v_(), pointers_v_i_(), pointers_v_ccp_(),
  decs_() {
}
void ExternalFunctionManager::AddFunction(string funcName, function<void(void)> funcPointer) {
  list<VariableDeclaration> args = list<VariableDeclaration>();
  auto dec = FunctionDeclaration(funcName, VariableDeclaration::Void, args);

  int id = id_++;
  decs_[dec] = id;
  pointers_v_v_.insert(make_pair(id, FunctionInfo<void(void)>(funcPointer, dec)));
}
void ExternalFunctionManager::AddFunction(string funcName, function<void(int)> funcPointer) {
  list<VariableDeclaration> args = list<VariableDeclaration>();
  args.push_back(VariableDeclaration(VariableDeclaration::Int));
  auto dec = FunctionDeclaration(funcName, VariableDeclaration::Void, args);

  int id = id_++;
  decs_[dec] = id;
  pointers_v_i_.insert(make_pair(id, FunctionInfo<void(int)>(funcPointer, dec)));
}
void ExternalFunctionManager::AddFunction(string funcName, function<void(const char*)> funcPointer) {
  list<VariableDeclaration> args = list<VariableDeclaration>();
  args.push_back(VariableDeclaration(VariableDeclaration::ConstCharPointer));
  auto dec = FunctionDeclaration(funcName, VariableDeclaration::Void, args);

  int id = id_++;
  decs_[dec] = id;
  pointers_v_ccp_.insert(make_pair(id, FunctionInfo<void(const char*)>(funcPointer, dec)));
}

int ExternalFunctionManager::GetId(FunctionDeclaration const& dec) const {
  auto it = decs_.find(dec);
  if (it == decs_.end())
    return -1;
  return it->second;
}

void ExternalFunctionManager::Invoke(int id, std::stack<int> *call_stack) const {
  auto it_v_v = pointers_v_v_.find(id);
  if (it_v_v != pointers_v_v_.end()) {
    it_v_v->second.pointer();
    return;
  }
  auto it_v_i = pointers_v_i_.find(id);
  if (it_v_i != pointers_v_i_.end()) {
    int i = call_stack->top();
    call_stack->pop();
    it_v_i->second.pointer(i);
    return;
  }
  auto it_v_cpp = pointers_v_ccp_.find(id);
  if (it_v_cpp != pointers_v_ccp_.end()) {
    int i = call_stack->top();
    call_stack->pop();
    it_v_cpp->second.pointer(reinterpret_cast<const char*>(i));
    return;
  }
}
}  // namespace api
}  // namespace charlie
