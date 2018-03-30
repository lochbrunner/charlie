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

#include "base.h"
#include "../vm/instruction.h"

namespace charlie {
namespace token {

using std::string;
using program::VariableDeclaration;

CodePostion::CodePostion(int character_position) : character_position(character_position) {}

Base::Base(TokenTypeEnum tokentype, CodePostion const& position, int priorty, bool finished, VariableDeclaration::TypeEnum type) :
  token_type(tokentype), position(position), priority(priorty), finished(finished),
  type(type), token_chidren_position(TokenChidrenPosEnum::None) {
}

Bracket::Bracket(KindEnum kind, DirectionEnum direction, CodePostion const& position) :
  Base(TokenTypeEnum::Bracket, position, 9), kind(kind), direction(direction) {
}

std::string Bracket::ToString() const {
  return string();
}

int Bracket::ByteCode() const {
  return -1;
}

Comma::Comma(CodePostion const& position) : Base(TokenTypeEnum::Comma, position, 1) {}
std::string Comma::ToString() const {
  return string();
}

int Comma::ByteCode() const {
  return -1;
}

List::List(CodePostion const& position) : Base(TokenTypeEnum::List, position, 1) {}

std::string List::ToString() const {
  return string();
}

int List::ByteCode() const {
  return -1;
}

Constant::Constant(KindEnum kind, void* pointer, CodePostion const& position) :
  Base(TokenTypeEnum::Constant, position, 1, true), kind(kind), pointer(pointer) {}

Constant::~Constant() {
  if (pointer != nullptr) {
    delete pointer;
  }
}

std::string Constant::ToString() const {
  return string();
}

int Constant::ByteCode() const {
  return -1;
}

ConstantInt::ConstantInt(int value, CodePostion const& position) :
  Base(TokenTypeEnum::ConstantInt, position, 1, true, VariableDeclaration::Int), value(value) {}
std::string ConstantInt::ToString() const {
  return string();
}

int ConstantInt::ByteCode() const {
  return value;
}

Operator::Operator(KindEnum kind, CodePostion const& postion) : Base(TokenTypeEnum::Operator, postion), kind(kind), assigner(false){
  switch (kind) {
  case Operator::KindEnum::Add:
  case Operator::KindEnum::Substract:
    priority = 5;
    token_chidren_position = TokenChidrenPosEnum::LeftAndRight;
    break;
  case Operator::KindEnum::Multipply:
  case Operator::KindEnum::Divide:
  case Operator::KindEnum::Modulo:
    priority = 6;
    token_chidren_position = TokenChidrenPosEnum::LeftAndRight;
    break;
  case Operator::KindEnum::Copy:
    priority = 2;
    assigner = true;
    token_chidren_position = TokenChidrenPosEnum::LeftAndRight;
    break;
  case Operator::KindEnum::Equal:
  case Operator::KindEnum::NotEqual:
  case Operator::KindEnum::Greater:
  case Operator::KindEnum::GreaterEqual:
  case Operator::KindEnum::Less:
  case Operator::KindEnum::LessEqual:
    priority = 4;
    token_chidren_position = TokenChidrenPosEnum::LeftAndRight;
    break;
  case Operator::KindEnum::LogicAnd:
  case Operator::KindEnum::LogicOr:
    priority = 3;
    token_chidren_position = TokenChidrenPosEnum::LeftAndRight;
    break;
  case Operator::KindEnum::BitAnd:
  case Operator::KindEnum::BitOr:
  case Operator::KindEnum::BitXor:
    priority = 6;
    token_chidren_position = TokenChidrenPosEnum::LeftAndRight;
    break;
  case Operator::KindEnum::AddTo:
  case Operator::KindEnum::SubstractTo:
  case Operator::KindEnum::MultiplyTo:
  case Operator::KindEnum::DivideTo:
  case Operator::KindEnum::ModuloTo:
  case Operator::KindEnum::AndTo:
  case Operator::KindEnum::OrTo:
  case Operator::KindEnum::XorTo:
    assigner = true;
    priority = 2;
    token_chidren_position = TokenChidrenPosEnum::LeftAndRight;
    break;
  case Operator::KindEnum::Increase:
  case Operator::KindEnum::Decrease:
    assigner = true;
    priority = 7;
    token_chidren_position = TokenChidrenPosEnum::LeftOrRight;
    break;
  default:
    break;
  }
}

std::string Operator::ToString() const {
  switch (kind) {
  case Operator::KindEnum::Add:
    return "+";
  case Operator::KindEnum::Substract:
    return "-";
  case Operator::KindEnum::Multipply:
    return "*";
  case Operator::KindEnum::Divide:
    return "/";
  case Operator::KindEnum::Modulo:
    return "%";
  case Operator::KindEnum::Copy:
    return "=";
  case Operator::KindEnum::Equal:
    return "==";
  case Operator::KindEnum::NotEqual:
    return "!=";
  case Operator::KindEnum::Greater:
    return ">";
  case Operator::KindEnum::GreaterEqual:
    return ">=";
  case Operator::KindEnum::Less:
    return "<";
  case Operator::KindEnum::LessEqual:
    return "<=";
  case Operator::KindEnum::LogicAnd:
    return "&&";
  case Operator::KindEnum::LogicOr:
    return "||";
  case Operator::KindEnum::BitAnd:
    return "&";
  case Operator::KindEnum::BitOr:
    return "|";
  case Operator::KindEnum::BitXor:
    return "^";
  case Operator::KindEnum::AddTo:
    return "+=";
  case Operator::KindEnum::SubstractTo:
    return "-=";
  case Operator::KindEnum::MultiplyTo:
    return "*=";
  case Operator::KindEnum::DivideTo:
    return "/=";
  case Operator::KindEnum::ModuloTo:
    return "%=";
  case Operator::KindEnum::AndTo:
    return "&=";
  case Operator::KindEnum::OrTo:
    return "|=";
  case Operator::KindEnum::XorTo:
    return "^=";
  case Operator::KindEnum::Increase:
    return "++";
  case Operator::KindEnum::Decrease:
    return "--";
  default:
    return "Unknown!";
  }
}

int Operator::ByteCode() const {
  switch (kind) {
  case Operator::KindEnum::Add:
    if (type == VariableDeclaration::Int)
      return vm::IntAdd;
    return -1;
  case Operator::KindEnum::Substract:
    if (type == VariableDeclaration::Int)
      return vm::IntSubstract;
    return -1;
  case Operator::KindEnum::Multipply:
    if (type == VariableDeclaration::Int)
      return vm::IntMultiply;
    return -1;
  case Operator::KindEnum::Divide:
    if (type == VariableDeclaration::Int)
      return vm::IntDivide;
    return -1;
  case Operator::KindEnum::Copy:
    if (type == VariableDeclaration::Int)
      return vm::IntCopy;
    return -1;
  case Operator::KindEnum::Equal:
    if (type == VariableDeclaration::Int)
      return vm::IntEqual;
    return -1;
  case Operator::KindEnum::NotEqual:
    if (type == VariableDeclaration::Int)
      return vm::IntNotEqual;
    return -1;
  case Operator::KindEnum::Greater:
    if (type == VariableDeclaration::Int)
      return vm::IntGreater;
    return -1;
  case Operator::KindEnum::GreaterEqual:
    if (type == VariableDeclaration::Int)
      return vm::IntGreaterEqual;
    return -1;
  case Operator::KindEnum::Less:
    if (type == VariableDeclaration::Int)
      return vm::IntLess;
    return -1;
  case Operator::KindEnum::LessEqual:
    if (type == VariableDeclaration::Int)
      return vm::IntLessEqual;
    return -1;
  case Operator::KindEnum::LogicAnd:
    break;
  case Operator::KindEnum::LogicOr:
    break;
  case Operator::KindEnum::BitAnd:
    break;
  case Operator::KindEnum::BitOr:
    break;
  case Operator::KindEnum::BitXor:
    break;
  case Operator::KindEnum::AddTo:
    break;
  case Operator::KindEnum::SubstractTo:
    break;
  case Operator::KindEnum::MultiplyTo:
    break;
  case Operator::KindEnum::DivideTo:
    break;
  case Operator::KindEnum::AndTo:
    break;
  case Operator::KindEnum::OrTo:
    break;
  case Operator::KindEnum::XorTo:
    break;
  case Operator::KindEnum::Increase:
    if (type == VariableDeclaration::Int)
      return vm::IntIncrease;
    return -1;
  case Operator::KindEnum::Decrease:
    if (type == VariableDeclaration::Int)
      return vm::IntDecrease;
    return -1;
  default:
    break;
  }
  return -1;
}

Declarer::Declarer(program::VariableDeclaration::TypeEnum kind, CodePostion const& position) :
  Base(TokenTypeEnum::TypeDeclarer, position, 1), kind(kind) {}

std::string Declarer::ToString() const {
  return string();
}

int Declarer::ByteCode() const {
  return -1;
}

Label::Label(string const& labelString, CodePostion const& position) :
  Base(TokenTypeEnum::Label, position, 10), label_string(labelString), kind(Label::KindEnum::Unknown), register_address(0) {
}

std::string Label::ToString() const {
  return label_string;
}

int Label::ByteCode() const {
  return -1;
}

ControlFlow::ControlFlow(KindEnum kind, CodePostion const& position) : Base(TokenTypeEnum::ControlFlow, position), kind(kind) {}
std::string ControlFlow::ToString() const {
  return string();
}

int ControlFlow::ByteCode() const {
  return -1;
}

}  // namespace token
}  // namespace charlie

