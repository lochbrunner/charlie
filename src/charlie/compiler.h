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

#ifndef CHARLIE_COMPILER_H
#define CHARLIE_COMPILER_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include "common/exportDefs.h"
#include "common/logging_component.h"

#include "program/function_declaration.h"
#include "program/mapping.h"
#include "program/statement.h"
#include "program/unresolved_program.h"

#include "vm/state.h"

#include "api/external_function_manager.h"

namespace charlie {
// This is the central class where as for now all APIs of this library can be called.
// Example:
//    auto compiler = Compiler([](std::string const &message){
//      std::cout << message << std::endl;
//    });
//    if(compiler.Build("main.cc"))
//      compiler.Run();
//
class Compiler : public common::LoggingComponent {
 public:
  // Creates an object without message delegate.
  xprt Compiler();
  // Creates an object with the specified message delegate.
  xprt Compiler(std::function<void(std::string const &message)> messageDelegate);
  // Compiles the speciefed C source file to bytecode. Returns true if succeeded.
  xprt bool Build(std::string const &filename, bool sourcemaps);
  // Saves the current porgram to the file. Optional binary or as readable textfile.
  // Returns true if succeeded.
  xprt bool SaveProgram(std::string const &filename, bool binary = true, bool mapping = false) const;
  // Returns the state containing the current program.
  xprt std::unique_ptr<vm::State> GetProgram();
  // Returns the mapping of available, otherwise return nullptr
  xprt std::shared_ptr<program::Mapping> GetMapping();  // TODO: create a struct containing mapping and bytecode
  // External function manager
  api::ExternalFunctionManager external_function_manager;

 private:
  // Compiles the syntax tree to bytecode.
  bool compile(bool sourcemaps);
  // Enroles a block of the syntax tree to bytecode.
  bool enroleBlock(
      std::map<program::FunctionDeclaration, int, program::FunctionDeclaration::comparer> const &functionDict,
      program::Scope const &block, int *count, bool sourcemaps);
  // Enroles a statement of the syntax tree to bytecode.
  bool enroleStatement(
      std::map<program::FunctionDeclaration, int, program::FunctionDeclaration::comparer> const &functionDict,
      program::Statement const &statement, int *count, bool sourcemaps);
  // The current program data.
  program::UnresolvedProgram program_;
  std::shared_ptr<program::Mapping> mapping_;
};
}  // namespace charlie

#endif  // ! CHARLIE_COMPILER_H
