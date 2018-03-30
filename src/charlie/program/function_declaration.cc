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

#include "function_declaration.h"

#include <list>
#include <cstring>

namespace charlie {

namespace program {

using std::list;

FunctionDeclaration::FunctionDeclaration(std::string const& label, VariableDeclaration::TypeEnum image_type,
                                         std::list<VariableDeclaration> const& argument_type, Scope* parent)
  : label(label), image_type(image_type), argument_types(argument_type), has_definition(false), definition(parent) {
}
FunctionDeclaration::FunctionDeclaration(std::string const& label, VariableDeclaration::TypeEnum image_type, Scope* parent)
  : label(label), image_type(image_type), has_definition(false), definition(parent) {
  argument_types = std::list<VariableDeclaration>();
}

void FunctionDeclaration::Dispose() {
  definition.Dispose();
}
std::ostream& operator<<(std::ostream & stream, const FunctionDeclaration &dec) {
  // See: http://stackoverflow.com/questions/476272/how-to-properly-overload-the-operator-for-an-ostream
  stream << VariableDeclaration::TypeString(dec.image_type) << '@' << dec.label << '(';
  bool first = true;
  for (list<VariableDeclaration>::const_iterator it = dec.argument_types.begin(); it != dec.argument_types.end(); ++it) {
    if (!first)
      stream << '@';
    else
      first = false;
    stream << VariableDeclaration::TypeString(it->image_type);
  }
  stream << ')';
  return stream;
}
// Is a "smaller" than b
bool FunctionDeclaration::comparer::operator()(const FunctionDeclaration & a, const FunctionDeclaration & b) const {
  // See: http://stackoverflow.com/questions/5733254/create-an-own-comparator-for-map
  int name = std::strcmp(a.label.c_str(), b.label.c_str());
  if (name != 0)
    return name < 0;

  list<VariableDeclaration>::const_iterator itA = a.argument_types.begin();
  list<VariableDeclaration>::const_iterator itB = b.argument_types.begin();
  while (itA != a.argument_types.end() && itB != b.argument_types.end()) {
    if (itA->image_type != itB->image_type)
      return itA->image_type - itB->image_type < 0;
    ++itA;
    ++itB;
  }
  if (itB != b.argument_types.end())
    return true;

  return false;
}
}  // namespace program
}  // namespace charlie
