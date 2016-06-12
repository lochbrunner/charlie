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

#ifndef CHARLIE_PROGRAM_STATEMENT_H
#define CHARLIE_PROGRAM_STATEMENT_H

#include <list>

#include "..\token\base.h"

namespace charlie {
namespace program {
// Statements are use als lower nodes in the syntax tree.
class Statement {
 public:
  // Creates an object out of a pointer to the token.
  Statement(token::Base* value);
  // Explicit desctructor is used to delete the value.
  virtual ~Statement();
  // Deletes all the hidden pointers of this and member instances.
  void Dispose();
  // Returns true, if this statement is finished.
  bool Finished() const;
  // Returns the priorty of this statement.
  // Needed to get the order of parsing. E.g. 3+4*5
  int priority() const;
  // Value of this node
  token::Base* value;
  // Children or arguments of this statement. E.g. "a" and "b" when a+b
  std::list<Statement> arguments;
};
}  // namespace program
}  // namespace charlie


#endif  // !CHARLIE_PROGRAM_STATEMENT_H
