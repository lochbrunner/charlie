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

#include "common/definitions.h"
#include "common/io.h"

#include "program/mapping.h"

#include "vm/instruction.h"

#define ERROR_MESSAGE_MAKE_CODE(message) error_message(message, __FILE__, __LINE__)
#define ERROR_MESSAGE_WITH_POS_MAKE_CODE(message, pos) error_message_to_code(message, pos, __FILE__, __LINE__)

namespace charlie {

using std::function;
using std::list;
using std::make_pair;
using std::map;
using std::string;
using std::stringstream;

using common::io::ascii2string;
using common::io::saveProgramAscii;
using common::io::saveProgramBinary;

using program::FunctionDeclaration;
using program::Mapping;
using program::Statement;
using program::VariableDeclaration;

using vm::InstructionEnums;
using vm::InstructionManager;
using vm::State;

using token::Base;
using token::ControlFlow;
using token::Label;
using token::Operator;

void write_scope_to_mapping(program::Scope const& scope, int begin, int end,
                            std::shared_ptr<program::Mapping> mapping) {
  auto scope_mapping = std::make_unique<program::Mapping::Scope>();

  for (auto& variable : scope.variable_informations) {
    auto var_mapping = std::make_unique<program::Mapping::Variable>();
    var_mapping->name = variable.name;
    var_mapping->position = variable.offset();
    var_mapping->type = program::VariableDeclaration::TypeString(variable.type);
    scope_mapping->variables.push_back(std::move(var_mapping));
  }
  scope_mapping->begin = begin;
  scope_mapping->end = end;
  mapping->Scopes.push_back(std::move(scope_mapping));
}

Compiler::Compiler() : LoggingComponent(), external_function_manager(), program_() {}

Compiler::Compiler(function<void(string const& message)> messageDelegate)
    : LoggingComponent(messageDelegate), external_function_manager(), program_() {}

bool Compiler::Build(string const& filename, bool sourcemaps) {
  string code;
  if (!ascii2string(filename, &code)) {
    std::stringstream str;
    str << "Can not open file \"" << filename << "\"";
    error_message(str);
    return false;
  }

  Scanner scanner(&program_, &external_function_manager, _messageDelegate);

  if (!scanner.Scan(code)) {
    error_message("Scanning failed!");
    return false;
  }
  codeInfo_.set(&code);
  if (!compile(sourcemaps)) {
    error_message("Compiling failed!");
    return false;
  }

  error_message("Building succeded!");
  return true;
}

bool Compiler::SaveProgram(std::string const& filename, bool binary, bool mapping) const {
  if (mapping) mapping_->Save(filename);
  if (binary)
    return saveProgramBinary(filename, program_);
  else
    return saveProgramAscii(filename, program_);
}

bool Compiler::compile(bool sourcemaps) {
  if (sourcemaps) mapping_ = std::make_shared<program::Mapping>();
  auto funcPositions = map<FunctionDeclaration, int, FunctionDeclaration::comparer>();

  program_.instructions.push_back(BYTECODE_VERSION);
  // Junp address will be inserted at the end
  // Global variables
  program_.instructions.push_back(InstructionEnums::IncreaseRegister);
  program_.instructions.push_back(program_.root.num_variable_declarations);
  int count = 2;

  for (auto itI = program_.root.statements.begin(); itI != program_.root.statements.end(); ++itI) {
    if (!enroleStatement(funcPositions, *itI, &count, sourcemaps)) return false;
  }

  program_.instructions.push_back(InstructionEnums::Call);
  auto itMainAddress = --program_.instructions.end();
  program_.instructions.push_back(InstructionEnums::Exit);
  count += 3;
  // Store function definitions
  for (auto itF = program_.function_declarations.cbegin(); itF != program_.function_declarations.cend(); ++itF) {
    int func_begin = count;
    if (!itF->has_definition) {
      stringstream st;
      st << "Missing defintion for function: " << (*itF);
      ERROR_MESSAGE_MAKE_CODE(st);
      return false;
    }
    funcPositions.insert(make_pair((*itF), count));
    if (!enroleBlock(funcPositions, itF->definition, &count, sourcemaps)) return false;

    program_.instructions.push_back(InstructionEnums::Return);
    if (sourcemaps) {
      auto fun_map = std::make_unique<program::Mapping::Function>(itF->label);
      fun_map->scope.begin = func_begin;
      fun_map->scope.end = count;
      mapping_->Functions.push_back(std::move(fun_map));
    }
    ++count;
  }

  program_.instructions.push_back(InstructionEnums::DecreaseRegister);
  if (sourcemaps) {
    write_scope_to_mapping(program_.root, 1, count, mapping_);
  }
  count += 1;

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

bool Compiler::enroleBlock(
    std::map<program::FunctionDeclaration, int, program::FunctionDeclaration::comparer> const& functionDict,
    program::Scope const& block, int* count, bool sourcemaps) {
  int begin = *count;
  program_.instructions.push_back(InstructionEnums::IncreaseRegister);
  program_.instructions.push_back(block.num_variable_declarations);
  *count += 2;

  // Insert variable declaration and defintion of the argument list
  for (auto itI = block.statements.cbegin(); itI != block.statements.cend(); ++itI) {
    if (!enroleStatement(functionDict, *itI, count, sourcemaps)) return false;
  }

  if (sourcemaps) {
    write_scope_to_mapping(block, begin, *count, mapping_);
  }
  program_.instructions.push_back(InstructionEnums::DecreaseRegister);
  *count += 1;
  return true;
}

bool Compiler::enroleStatement(map<FunctionDeclaration, int, FunctionDeclaration::comparer> const& functionDict,
                               Statement const& statement, int* count, bool sourcemaps) {
  auto tokenType = statement.value->token_type;
  if (tokenType == Base::TokenTypeEnum::ConstantInt) {
    program_.instructions.push_back(InstructionEnums::PushConst);
    program_.instructions.push_back(statement.value->ByteCode());
    *count += 2;
  } else if (tokenType == Base::TokenTypeEnum::Label) {
    if (dynamic_cast<Label*>(statement.value)->kind == Label::KindEnum::Function) {
      auto label = dynamic_cast<Label*>(statement.value);

      std::list<VariableDeclaration> argTypes;
      for (auto it = statement.arguments.begin(); it != statement.arguments.end(); ++it) {
        if (!enroleStatement(functionDict, *it, count, sourcemaps)) return false;
        argTypes.push_back(it->value->type);
      }
      FunctionDeclaration dec(label->label_string, VariableDeclaration::Length, argTypes);

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
    } else if (dynamic_cast<Label*>(statement.value)->kind == Label::KindEnum::Variable) {
      auto label = dynamic_cast<Label*>(statement.value);
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
  } else if (tokenType == Base::TokenTypeEnum::Operator) {
    auto op = dynamic_cast<Operator*>(statement.value);
    if (op->assigner) {
      auto itAddress = statement.arguments.begin();
      assert(itAddress->value->token_type == Base::TokenTypeEnum::Label);
      int address = dynamic_cast<Label*>(itAddress->value)->register_address();

      // TODO(lochbrunner): asign operators can also be used to push values: e.g. i = j++;
      if (op->token_chidren_position == Base::TokenChidrenPosEnum::LeftAndRight) {
        if (!enroleStatement(functionDict, *++itAddress, count, sourcemaps)) return false;
      }
      program_.instructions.push_back(op->ByteCode());
      program_.instructions.push_back(address);
      *count += 2;
    } else if (op->kind == Operator::KindEnum::Pop) {
      program_.instructions.push_back(InstructionEnums::IntPop);
      program_.instructions.push_back(dynamic_cast<Label*>(statement.arguments.begin()->value)->register_address());
      *count += 2;
    } else {
      for (auto it = statement.arguments.begin(); it != statement.arguments.end(); ++it) {
        if (!enroleStatement(functionDict, *it, count, sourcemaps)) return false;
      }
      program_.instructions.push_back(statement.value->ByteCode());
      ++*count;
    }
  } else if (tokenType == Base::TokenTypeEnum::ControlFlow) {
    if (dynamic_cast<const ControlFlow*>(statement.value)->kind == ControlFlow::KindEnum::If) {
      // Should have exactly two arguments: First a statement, second a block
      assert(statement.arguments.begin() != statement.arguments.end());
      assert(++++statement.arguments.begin() == statement.arguments.end());
      assert(statement.arguments.begin()->block == nullptr);
      assert(statement.arguments.begin()->value != nullptr);
      assert((++statement.arguments.begin())->value == nullptr);
      assert((++statement.arguments.begin())->block != nullptr);

      enroleStatement(functionDict, *statement.arguments.begin(), count, sourcemaps);

      program_.instructions.push_back(InstructionEnums::PushConst);
      program_.instructions.push_back(-1);

      auto itAlt = --program_.instructions.end();
      program_.instructions.push_back(InstructionEnums::JumpIf);
      *count += 3;
      auto block = (++statement.arguments.begin())->block;
      enroleBlock(functionDict, *block, count, sourcemaps);
      *itAlt = *count;
    } else if (dynamic_cast<const ControlFlow*>(statement.value)->kind == ControlFlow::KindEnum::While) {
      // Should have exactly two arguments: First a statement, second a block
      assert(statement.arguments.begin() != statement.arguments.end());
      assert(++++statement.arguments.begin() == statement.arguments.end());
      assert(statement.arguments.begin()->block == nullptr);
      assert(statement.arguments.begin()->value != nullptr);
      assert((++statement.arguments.begin())->value == nullptr);
      assert((++statement.arguments.begin())->block != nullptr);

      int begin = *count;
      enroleStatement(functionDict, *statement.arguments.begin(), count, sourcemaps);

      program_.instructions.push_back(InstructionEnums::PushConst);
      program_.instructions.push_back(-1);

      auto itAlt = --program_.instructions.end();
      program_.instructions.push_back(InstructionEnums::JumpIf);
      *count += 3;

      auto block = (++statement.arguments.begin())->block;
      enroleBlock(functionDict, *block, count, sourcemaps);

      program_.instructions.push_back(InstructionEnums::Jump);
      program_.instructions.push_back(begin);
      *count += 2;

      *itAlt = *count;
    }
  }
  return true;
}

std::unique_ptr<State> Compiler::GetProgram() {
  auto it = program_.instructions.cbegin();
  if (it == program_.instructions.cend()) return std::unique_ptr<State>(nullptr);

  auto state = std::make_unique<State>();
  state->external_function_manager = &external_function_manager;

  // Skip version byte
  int version = (*it++);

  for (; it != program_.instructions.end(); ++it) {
    state->program.push_back(*it);
  }

  return state;
}

xprt std::shared_ptr<program::Mapping> Compiler::GetMapping() { return mapping_; }

}  // namespace charlie

#undef ERROR_MESSAGE_MAKE_CODE
#undef ERROR_MESSAGE_WITH_POS_MAKE_CODE
