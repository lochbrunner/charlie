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
* SUCH DAMAGE.zeof(int)*size);
   ^~~~~~~~
*/

#ifndef CHARLIE_PROGRAM_UNRESOLVEDPROGRAM_H
#define CHARLIE_PROGRAM_UNRESOLVEDPROGRAM_H

#include <list>
#include <map>

#include "function_declaration.h"
#include "scope.h"
#include "variable_declaration.h"

#include "../common/exportDefs.h"

namespace charlie::program {
// Stores on the one hand the whole syntax tree and function definitions and on the other hand the
// linear bytecode
class UnresolvedProgram {
 public:
  // Creates an object
  xprt UnresolvedProgram();
  // Deletes all the hidden pointers of this and member instances.
  xprt void Dispose();
  // The bytecode
  std::vector<int> instructions;
  // All function declarations
  std::list<FunctionDeclaration> function_declarations;
  // The root scope for the syntax tree
  Scope root;
};
}  // namespace charlie::program

#endif  // !CHARLIE_PROGRAM_UNRESOLVEDPROGRAM_H
