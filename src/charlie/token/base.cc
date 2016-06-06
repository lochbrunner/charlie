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
#include "..\vm\instruction.h"

namespace charlie {
	namespace token {

		using namespace std;
		using namespace program;

		Base::Base(TokenTypeEnum tokentype, int priorty, bool finished, VariableDec::TypeEnum type) :
			TokenType(tokentype), Priority(priorty), Finished(finished), Type(type), TokenChidrenPos(TokenChidrenPosEnum::None){}

		Bracket::Bracket(KindEnum kind, DirectionEnum direction) : 
			Base(TokenTypeEnum::Bracket, 8), Kind(kind), Direction(direction) {}
		std::string Bracket::ToString() 
		{
			return string();
		}
		int Bracket::ByteCode() 
		{
			return -1;
		}

		Comma::Comma() : Base(TokenTypeEnum::Comma, 1) {}
		std::string Comma::ToString() 
		{
			return string();
		}
		int Comma::ByteCode()
		{
			return -1;
		}

		List::List() : Base(TokenTypeEnum::List, 1) {}
		std::string List::ToString()
		{
			return string();
		}
		int List::ByteCode()
		{
			return -1;
		}

		Constant::Constant(KindEnum kind, void* pointer) : Base(TokenTypeEnum::Constant, 1, true), Kind(kind), Pointer(pointer){}
		Constant::~Constant() {
			if (Pointer != 0) {
				delete Pointer;
			}
		}
		std::string Constant::ToString()
		{
			return string();
		}
		int Constant::ByteCode()
		{
			return -1;
		}

		ConstantInt::ConstantInt(int value) : Base(TokenTypeEnum::ConstantInt, 1, true, VariableDec::Int), Value(value){}
		std::string ConstantInt::ToString()
		{
			return string();
		}
		int ConstantInt::ByteCode()
		{
			return Value;
		}

		Operator::Operator(KindEnum kind) : Base(TokenTypeEnum::Operator), Kind(kind)
		{
			switch (kind)
			{
			case charlie::token::Operator::Add:
			case charlie::token::Operator::Substract:
				Priority = 5;
				TokenChidrenPos = TokenChidrenPosEnum::LeftAndRight;
				break;
			case charlie::token::Operator::Multipply:
			case charlie::token::Operator::Divide:
				Priority = 6;
				TokenChidrenPos = TokenChidrenPosEnum::LeftAndRight;
				break;
			case charlie::token::Operator::Copy:
				Priority = 2;
				TokenChidrenPos = TokenChidrenPosEnum::LeftAndRight;
				break;
			case charlie::token::Operator::Equal:
			case charlie::token::Operator::NotEqual:
			case charlie::token::Operator::Greater:
			case charlie::token::Operator::GreaterEqual:
			case charlie::token::Operator::Less:
			case charlie::token::Operator::LessEqual:
				Priority = 4;
				TokenChidrenPos = TokenChidrenPosEnum::LeftAndRight;
				break;
			case charlie::token::Operator::LogicAnd:
			case charlie::token::Operator::LogicOr:
				Priority = 3;
				TokenChidrenPos = TokenChidrenPosEnum::LeftAndRight;
				break;
			case charlie::token::Operator::BitAnd:
			case charlie::token::Operator::BitOr:
			case charlie::token::Operator::BitXor:
				Priority = 6;
				TokenChidrenPos = TokenChidrenPosEnum::LeftAndRight;
				break;
			case charlie::token::Operator::AddTo:
			case charlie::token::Operator::SubstractTo:
			case charlie::token::Operator::MultiplyTo:
			case charlie::token::Operator::DivideTo:
			case charlie::token::Operator::AndTo:
			case charlie::token::Operator::OrTo:
			case charlie::token::Operator::XorTo:
				Priority = 2;
				TokenChidrenPos = TokenChidrenPosEnum::LeftAndRight;
				break;
			default:
				break;
			}
		}
		std::string Operator::ToString() {
			return string();
		}
		int Operator::ByteCode()
		{
			switch (Kind)
			{
			case charlie::token::Operator::Add:
				if(Type == VariableDec::Int)
					return vm::IntAdd;
				return -1;
			case charlie::token::Operator::Substract:
				if (Type == VariableDec::Int)
					return vm::IntSubstract;
				return -1;
			case charlie::token::Operator::Multipply:
				if (Type == VariableDec::Int)
					return vm::IntMultiply;
				return -1;
			case charlie::token::Operator::Divide:
				if (Type == VariableDec::Int)
					return vm::IntDivide;
				return -1;
			case charlie::token::Operator::Copy:
				break;
			case charlie::token::Operator::Equal:
				break;
			case charlie::token::Operator::NotEqual:
				break;
			case charlie::token::Operator::Greater:
				break;
			case charlie::token::Operator::GreaterEqual:
				break;
			case charlie::token::Operator::Less:
				break;
			case charlie::token::Operator::LessEqual:
				break;
			case charlie::token::Operator::LogicAnd:
				break;
			case charlie::token::Operator::LogicOr:
				break;
			case charlie::token::Operator::BitAnd:
				break;
			case charlie::token::Operator::BitOr:
				break;
			case charlie::token::Operator::BitXor:
				break;
			case charlie::token::Operator::AddTo:
				break;
			case charlie::token::Operator::SubstractTo:
				break;
			case charlie::token::Operator::MultiplyTo:
				break;
			case charlie::token::Operator::DivideTo:
				break;
			case charlie::token::Operator::AndTo:
				break;
			case charlie::token::Operator::OrTo:
				break;
			case charlie::token::Operator::XorTo:
				break;
			default:
				break;
			}
			return -1;
		}
		Declarer::Declarer(program::VariableDec::TypeEnum kind) : Base(TokenTypeEnum::TypeDeclarer, 1), Kind(kind) {}
		std::string Declarer::ToString() {
			return string();
		}
		int Declarer::ByteCode()
		{
			return -1;
		}
		Label::Label(string& labelString) : Base(TokenTypeEnum::Label, 9), LabelString(labelString), Kind(Unknown){}
		std::string Label::ToString()
		{
			return string();
		}
		int Label::ByteCode()
		{
			return -1;
		}
		ControlFlow::ControlFlow(KindEnum kind) : Base(TokenTypeEnum::ControlFlow), Kind(kind) {}
		std::string ControlFlow::ToString()
		{
			return string();
		}
		int ControlFlow::ByteCode()
		{
			return -1;
		}
	}
}


