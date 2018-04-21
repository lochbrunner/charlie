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

#ifndef CHARLIE_TOKEN_VARIABLE_DECLARATION_H
#define CHARLIE_TOKEN_VARIABLE_DECLARATION_H

#include <string>

namespace charlie::program {
// Stores the name and the type of a variable declaration.
class VariableDeclaration {
 public:
  // All supported primitive types.
  enum TypeEnum {
    Int,
    Long,
    Float,
    Double,
    Boolean,
    Char,
    ConstCharPointer,
    Void,
    // Used the get the lenght of this enum and to indicate invalid or unknown type.
    // Depending on the context
    Length
  };
  // Use this struct when using a hash function for
  // std::map<VariableDeclaration, T, VariableDeclaration::comparer> or
  // std::set<VariableDeclaration, VariableDeclaration::comparer>
  //
  // Name and type of the variable declaration is important.
  struct comparer {
    bool operator()(const VariableDeclaration& a, const VariableDeclaration& b) const;
  };
  // Only the type is important. Ignores the name.
  struct comparer_only_type {
    bool operator()(const VariableDeclaration& a, const VariableDeclaration& b) const;
  };
  // Only the name is important. Ignores the type.
  struct comparer_only_name {
    bool operator()(const VariableDeclaration& a, const VariableDeclaration& b) const;
  };
  // Creates an object. Needs type of object.
  // The name is optional.
  VariableDeclaration(TypeEnum imageType);
  VariableDeclaration(std::string name, TypeEnum imageType);
  // Converts a type into a string.
  static const char* TypeString(TypeEnum type);
  // The name of the variable
  std::string name;
  // The type of the variable
  TypeEnum image_type;
};
}  // namespace charlie::program

#endif  // ! CHARLIE_TOKEN_VARIABLE_DECLARATION_H
