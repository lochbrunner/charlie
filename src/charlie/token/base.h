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

// See: http://www.quut.com/c/ANSI-C-grammar-y.html

#ifndef CHARLIE_TOKEN_BASE_H
#define CHARLIE_TOKEN_BASE_H

#include <string>
#include <functional>

#include "../program/variableDec.h"

namespace charlie {
	namespace token {

		class CodePostion {
		public:
			CodePostion(int characterPosition);
			int CharacterNumber;
		};

		class Base
		{
		public:
			enum class TokenTypeEnum
			{
				Bracket,				// (, ), {, }, ...
				Constant,				// "Hello", 12, ...
				ConstantInt,
				Operator,				// +, -, +=, ...
				TypeDeclarer,			// int, bool, ...
				Label,
				ControlFlow,			// for, while
				List, 
				Comma
			};

			enum class TokenChidrenPosEnum {
				None = 0,
				Left = 1,
				Right = 2,
				LeftAndRight = 3
			};
			Base(TokenTypeEnum tokentype, CodePostion& position, int priorty = 0, bool finished = false, program::VariableDec::TypeEnum type = program::VariableDec::Length);
			TokenTypeEnum TokenType;
			// label: 9,  bracket: 8, namesep (::; .): 8,  (de)ref: 7; mul/div: 6, add/sub: 5, 
			// comparer: 4, logic ops: 3, copy: 2 others: 1
			int Priority;
			bool Finished;
			TokenChidrenPosEnum TokenChidrenPos;
			CodePostion Position;

			program::VariableDec::TypeEnum Type;

			virtual int ByteCode()=0;
			virtual std::string ToString()=0;
		};

		class Bracket : public Base {
		public:

			enum KindEnum
			{
				Round,		// ()
				Curly,		// {}
				Square,		// []
				Triangle	// <>
			};

			enum DirectionEnum {
				Closing,
				Opening
			};

			Bracket(KindEnum kind, DirectionEnum direction, CodePostion position);
			KindEnum Kind;
			DirectionEnum Direction;

			virtual std::string ToString();
			virtual int ByteCode();
		};

		class Comma : public Base {
		public:
			Comma(CodePostion& position);
			virtual std::string ToString();
			virtual int ByteCode();
		};

		class List : public Base {
		public:
			List(CodePostion& position);
			virtual std::string ToString();
			virtual int ByteCode();
		};

		class Constant : public Base{
		public:
			enum KindEnum {
				String,			// use std::string
				Char,			// char
				Decimal,		// float
				Boolean			// bool
			};

			Constant(KindEnum kind, void* pointer, CodePostion& position);
			~Constant();

			KindEnum Kind;
			void* Pointer;

			virtual std::string ToString();
			virtual int ByteCode();
		};

		class ConstantInt : public Base {
		public:
			ConstantInt(int value, CodePostion& position);
			virtual std::string ToString();
			virtual int ByteCode();

			int Value;
		};

		class Operator : public Base {
		public:
			enum KindEnum		// TODO: Not complete!
			{
				Add,			// +
				Substract,		// -		// Could be also uniary
				Multipply,		// *		// Or dereference
				Divide,			// /
				Modulo,			// %
				Copy,			// =
				Equal,			// ==
				NotEqual,		// !=
				Greater,		// >		// Or template bracket
				GreaterEqual,	// >=
				Less,			// <		// Or template bracket
				LessEqual,		// <=
				LogicAnd,		// &&
				LogicOr,		// ||
				BitAnd,			// &		// Or reference
				BitOr,			// |
				BitXor,			// ^
				AddTo,			// +=
				SubstractTo,	// -=
				MultiplyTo,		// *=
				DivideTo,		// /=
				ModuloTo,		// /=
				AndTo,			// &=
				OrTo,			// |=,
				XorTo,			// ^=,
				Pop
			};
			KindEnum Kind;

			Operator(KindEnum kind, CodePostion& position);

			virtual std::string ToString();
			virtual int ByteCode();
		};

		class ControlFlow : public Base {
		public:
			enum KindEnum {
				While,
				For,
				Do,
				If,
				Else,
				Break,
				Continue,
				Return,
				Switch,
				Case,
				Goto
			};

			ControlFlow(KindEnum kind, CodePostion& position);

			KindEnum Kind;

			virtual std::string ToString();
			virtual int ByteCode();
		};

		class Declarer : public Base 
		{
		public:
			Declarer(program::VariableDec::TypeEnum kind, CodePostion& position);
			program::VariableDec::TypeEnum Kind;

			virtual std::string ToString();
			virtual int ByteCode();
		};

		class Label : public Base {
		public:

			enum KindEnum {
				Function,
				Variable,
				Unknown
			};

			Label(std::string& labelString, CodePostion& position);
			std::string LabelString;
			KindEnum Kind;
			std::function<int()> RegAddress;

			virtual std::string ToString();
			virtual int ByteCode();
		};
	}
}

#endif // !CHARLIE_TOKEN_BASE_H