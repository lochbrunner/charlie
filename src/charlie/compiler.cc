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

#include "compiler.h"

#include <assert.h>

#include <list>
#include <sstream>

#include "scanner.h"

#include "common\io.h"
#include "common\definitions.h"

#include "vm\instruction.h"

#define ERROR_MESSAGE_MAKE_CODE(message) error_message(message, __FILE__, __LINE__)
#define ERROR_MESSAGE_WITH_POS_MAKE_CODE(message, pos) error_message_to_code(message, pos, __FILE__, __LINE__)


namespace charlie {

using std::map;
using std::list;
using std::string;
using std::stringstream;
using std::function;
using std::make_pair;

using common::io::ascii2string;
using common::io::saveProgramAscii;
using common::io::saveProgramBinary;

using program::VariableDeclaration;
using program::FunctionDeclaration;
using program::Statement;

using vm::InstructionEnums;
using vm::InstructionManager;
using vm::State;

using token::Label;
using token::Base;
using token::ControlFlow;
using token::Operator;

Compiler::Compiler() :
  LoggingComponent(), external_function_manager(), program_() {
}

Compiler::Compiler(function<void(string const& message)> messageDelegate) :
  LoggingComponent(messageDelegate), external_function_manager(), program_() {
}

bool Compiler::Build(string const& filename) {
  string code;
  if (!ascii2string(filename, &code)) {
    std::stringstream str;
    str << "Can not open file \"" << filename << "\"";
    ERROR_MESSAGE_MAKE_CODE(str);
    return false;
  }

  Scanner scanner = Scanner(&program_, &external_function_manager, _messageDelegate);

  if (!scanner.Scan(code)) {
    ERROR_MESSAGE_MAKE_CODE("Scanning failed!");
    return false;
  }
  codeInfo_.set(&code);
  if (!compile()) {
    ERROR_MESSAGE_MAKE_CODE("Compiling failed!");
    return false;
  }

  error_message("Building succeded!");
  return true;
}

bool Compiler::SaveProgram(std::string const &filename, bool binary) const {
  if (binary)
    return saveProgramBinary(filename, program_);
  else
    return saveProgramAscii(filename, program_);
}

bool Compiler::compile() {
  auto funcPositions = map<FunctionDeclaration, int, FunctionDeclaration::comparer>();

  program_.instructions.push_back(BYTECODE_VERSION);
  // Junp address will be inserted at the end
  // Global variables
  program_.instructions.push_back(InstructionEnums::IncreaseRegister);
  program_.instructions.push_back(program_.root.num_variable_declarations);
  int count = 2;

  for (auto itI = program_.root.statements.begin(); itI != program_.root.statements.end(); ++itI) {
    if (!enroleStatement(funcPositions, *itI, &count))
      return false;
  }

  program_.instructions.push_back(InstructionEnums::Call);
  auto itMainAddress = --program_.instructions.end();
  program_.instructions.push_back(InstructionEnums::Exit);
  count += 3;

  for (auto itF = program_.function_declarations.begin(); itF != program_.function_declarations.end(); ++itF) {
    if (!itF->has_definition) {
      stringstream st;
      st << "Missing defintion for function: " << (*itF);
      ERROR_MESSAGE_MAKE_CODE(st);
      return false;
    }
    funcPositions.insert(make_pair((*itF), count));

    enroleBlock(funcPositions, itF->definition, &count);

    program_.instructions.push_back(InstructionEnums::Return);
    ++count;
  }

  program_.instructions.push_back(InstructionEnums::DecreaseRegister);
  program_.instructions.push_back(program_.root.num_variable_declarations);
  count += 2;

  // Find entryPoint
  auto args = list<VariableDeclaration>();
  args.push_back(VariableDeclaration::Int);
  args.push_back(VariableDeclaration::Char);
  auto main = funcPositions.find(FunctionDeclaration(string("main"), VariableDeclaration::Int));
  if (main == funcPositions.end()) {
    ERROR_MESSAGE_MAKE_CODE("Can not find entry point");
    return false;
  }

  program_.instructions.insert(++itMainAddress, main->second);
  program_.Dispose();
  return true;
}

bool Compiler::enroleBlock(std::map<program::FunctionDeclaration, int, program::FunctionDeclaration::comparer> const& functionDict,
  program::Scope const& block, int *count) {

  program_.instructions.push_back(InstructionEnums::IncreaseRegister);
  program_.instructions.push_back(block.num_variable_declarations);
  *count += 2;

  // Insert variable declaration and defintion of the argument list
  for (auto itI = block.statements.begin(); itI != block.statements.end(); ++itI) {
    if (!enroleStatement(functionDict, *itI, count))
      return false;
  }

  program_.instructions.push_back(InstructionEnums::DecreaseRegister);
  program_.instructions.push_back(block.num_variable_declarations);
  *count += 2;
  return true;
}

bool Compiler::enroleStatement(map<FunctionDeclaration, int, FunctionDeclaration::comparer> const& functionDict,
                               Statement const& statement, int *count) {
  auto tokenType = statement.value->token_type;
  if (tokenType == Base::TokenTypeEnum::ConstantInt) {
    program_.instructions.push_back(InstructionEnums::PushConst);
    program_.instructions.push_back(statement.value->ByteCode());
    *count += 2;
  } 
  else if (tokenType == Base::TokenTypeEnum::Label)
  {
    if (dynamic_cast<Label*>(statement.value)->kind == Label::KindEnum::Function) {
      auto label = dynamic_cast<Label*>(statement.value);

      auto argTypes = std::list<VariableDeclaration>();
      for (auto it = statement.arguments.begin(); it != statement.arguments.end(); ++it) {
        if (!enroleStatement(functionDict, *it, count))
          return false;
        argTypes.push_back(it->value->type);
      }
      auto dec = FunctionDeclaration(label->label_string, VariableDeclaration::Length, argTypes);

      int id = external_function_manager.GetId(dec);
      if (id > -1) {
        program_.instructions.push_back(InstructionEnums::CallEx);
        program_.instructions.push_back(id);
        *count += 2;
      } else {
        auto it = functionDict.find(dec);
        if (it == functionDict.end()) {
          stringstream st;
          st << "Can not find function " << dec;
          ERROR_MESSAGE_WITH_POS_MAKE_CODE(st, label->position.character_position);
          return false;
        }
        label->type = it->first.image_type;
        program_.instructions.push_back(InstructionEnums::Call);
        program_.instructions.push_back(it->second);
        *count += 2;
      }
    }
    else if (dynamic_cast<Label*>(statement.value)->kind == Label::KindEnum::Variable)
    {
      Label* label = dynamic_cast<Label*>(statement.value);
      int address = label->register_address();
      if (address > -1) {
        program_.instructions.push_back(InstructionEnums::Push);
        program_.instructions.push_back(address);
        *count += 2;
      } else {
        ERROR_MESSAGE_WITH_POS_MAKE_CODE("Not addressed variable found!", label->position.character_position);
        return false;
      }
    }
  }
  else if (tokenType == Base::TokenTypeEnum::Operator)
  {
    auto op = dynamic_cast<Operator*>(statement.value);
    if (op->kind == Operator::KindEnum::Copy) {
      auto itAddress = statement.arguments.begin();
      assert(itAddress->value->token_type == Base::TokenTypeEnum::Label);

      int address = dynamic_cast<Label*>(itAddress->value)->register_address();
      if (!enroleStatement(functionDict, *++itAddress, count))
        return false;
      program_.instructions.push_back(InstructionEnums::IntCopy);
      program_.instructions.push_back(address);
      *count += 2;
    } else if (op->kind == Operator::KindEnum::Pop) {
      program_.instructions.push_back(InstructionEnums::IntPop);
      program_.instructions.push_back(dynamic_cast<Label*>(statement.arguments.begin()->value)->register_address());
      *count += 2;
    } else {
      for (auto it = statement.arguments.begin(); it != statement.arguments.end(); ++it) {
        if (!enroleStatement(functionDict, *it, count))
          return false;
      }
      program_.instructions.push_back(statement.value->ByteCode());
      ++*count;
    }
  }
  else if (tokenType == Base::TokenTypeEnum::ControlFlow) {
    if (dynamic_cast<const ControlFlow*>(statement.value)->kind == ControlFlow::KindEnum::If) {
      // Should have exactly two arguments: First a statement, second a block
      assert(statement.arguments.begin() != statement.arguments.end());
      assert(++++statement.arguments.begin() == statement.arguments.end());
      assert(statement.arguments.begin()->block == nullptr);
      assert(statement.arguments.begin()->value != nullptr);
      assert((++statement.arguments.begin())->value == nullptr);
      assert((++statement.arguments.begin())->block != nullptr);

      enroleStatement(functionDict, *statement.arguments.begin(), count);

      program_.instructions.push_back(InstructionEnums::PushConst);
      program_.instructions.push_back(-1);
      
      auto itAlt = --program_.instructions.end();
      program_.instructions.push_back(InstructionEnums::JunpIf);
      *count += 3;
      auto block = (++statement.arguments.begin())->block;
      enroleBlock(functionDict, *block, count);
      *itAlt = *count;
    }

  }
  return true;
}

int Compiler::Run(int argn, char** argv) const {
  list<int>::const_iterator it = program_.instructions.begin();
  if (it == program_.instructions.end())
    return false;

  auto state = State();
  state.external_function_manager = &external_function_manager;
  state.alu_stack.push(argn);
  state.alu_stack.push(reinterpret_cast<int>(argv));

  int version = (*it++);

  if (version != BYTECODE_VERSION) {
    ERROR_MESSAGE_MAKE_CODE("Wrong bytecode version");
    return -1;
  }

  for (; it != program_.instructions.end(); ++it) {
    state.program.push_back(*it);
  }

  while (state.pos > -1/* && !state.call_stack.empty()*/) {
    int r = InstructionManager::Instructions[state.program[state.pos]](state);
    if (r < 0)
      break;
  }
  if (state.alu_stack.empty())
    return 0;
  return state.alu_stack.top();
}

int Compiler::Run() const {
  list<int>::const_iterator it = program_.instructions.begin();
  if (it == program_.instructions.end())
    return false;

  auto state = State();
  state.external_function_manager = &external_function_manager;

  int version = (*it++);

  if (version != BYTECODE_VERSION) {
    ERROR_MESSAGE_MAKE_CODE("Wrong bytecode version");
    return -1;
  }

  for (; it != program_.instructions.end(); ++it) {
    state.program.push_back(*it);
  }

  while (state.pos > -1/* && !state.call_stack.empty()*/) {
    int r = InstructionManager::Instructions[state.program[state.pos]](state);
    if (r < 0)
      break;
  }
  if (state.alu_stack.empty())
    return 0;
  return state.alu_stack.top();
}
}  // namespace charlie

#undef ERROR_MESSAGE_MAKE_CODE
#undef ERROR_MESSAGE_WITH_POS_MAKE_CODE

