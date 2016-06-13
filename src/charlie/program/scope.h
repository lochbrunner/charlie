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

#ifndef CHARLIE_PROGRAM_SCOPE_H
#define CHARLIE_PROGRAM_SCOPE_H

#include <list>
#include <map>
#include <functional>

namespace charlie {
namespace program {
class Scope;
}  // namespace program
}  // namespace charlie

#include "variable_declaration.h"
#include "statement.h"

namespace charlie {
namespace program {
// Stores variable declarations and statements of one scope.
class Scope {
 public:
  // Stores all variable image type and a delegate which can be used at compile time
  // to get the index of the variable in the register.
  struct VariableInfo {
    // Creates the object. All members must be defined.
    VariableInfo(std::function<int()> offset, VariableDeclaration::TypeEnum type);
    // Variable type
    VariableDeclaration::TypeEnum type;
    // Position in the VMs register
    std::function<int()> offset;
  };

  // Creates an object. Needs a pointer to the parent scope.
  // If the parameter is the nullptr this indicates that this should be the root-scope.
  Scope(Scope* parent);
  // Deletes all the hidden pointers of this and member instances.
  void Dispose();
  // Gets all the gathered information of specified variable declaration.
  // Returns invalid result iff nothing was found: VariableInfo(0, VariableDeclaration::TypeEnum::Length);
  VariableInfo GetVariableInfo(VariableDeclaration const& dec) const;
  // Adds a new variable declaration and returns its key in the storing map.
  int AddVariableDec(VariableDeclaration const& dec);
  // Returns the index position of the this scope in the VMs register.
  int ParentOffset() const;
  // Number of the variable stored untill now.
  int num_variable_declarations;
  // List of all statements in the order as they appeared in the source code.
  std::list<Statement> statements;

 private:
  // Stores all the variable declarations of this scope.
  // The types are ignored, because it is not allowed to get multiple variable declarations with differen types in one scope.
  std::map<VariableDeclaration, int, VariableDeclaration::comparer_only_name> variable_declarations_;
  // The parent scope. Scope is root-scope iff this pointer is nullptr.
  Scope* parent_;
};
}  // namespace program
}  // namespace charlie

#endif  // !CHARLIE_PROGRAM_SCOPE_H

