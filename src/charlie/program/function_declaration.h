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

#ifndef CHARLIE_TOKEN_FUNCTIONDEC_H
#define CHARLIE_TOKEN_FUNCTIONDEC_H

#include <list>
#include <string>
#include <sstream>

#include "variable_declaration.h"
#include "scope.h"

namespace charlie {

namespace program {
// Stores the signature and definition of a function.
class FunctionDeclaration {
 public:
  // Use this struct when using a hash function for
  // std::map<FunctionDeclaration, T, FunctionDeclaration::comparer> or
  // std::set<FunctionDeclaration, FunctionDeclaration::comparer>
  struct comparer {
    bool operator()(const FunctionDeclaration &a, const FunctionDeclaration &b) const;
  };
  // Creates an object. You have to specify the label of the function, the image type,
  // optional the argument type list and the scope where this function gets declared.
  FunctionDeclaration(std::string const& label, VariableDeclaration::TypeEnum image_type,
    std::list<VariableDeclaration> const& argument_type, Scope* parent = nullptr);
  FunctionDeclaration(std::string const& label, VariableDeclaration::TypeEnum image_type, Scope* parent = nullptr);
  // Disposes all elements and child elments of this instance.
  void Dispose();
  // Prints a the signature of the specified function into the specified declaration.
  friend std::ostream& operator<<(std::ostream &stream, const FunctionDeclaration &dec);
  // The image type of the function
  VariableDeclaration::TypeEnum image_type;
  // The argument type list of the function
  std::list<program::VariableDeclaration> argument_types;
  // The label of the function.
  std::string label;
  // Stores if the functions definition is already found and parsed.
  bool has_definition;
  // Stores the definition of the function. Empty if not found yet.
  Scope definition;
};
}  // namespace program
}  // namespace charlie


#endif  // !CHARLIE_TOKEN_FUNCTIONDEC_H
