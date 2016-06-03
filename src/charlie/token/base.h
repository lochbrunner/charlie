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


#ifndef CHARLIE_TOKEN_BASE_H
#define CHARLIE_TOKEN_BASE_H

#include <string>

#include "../program/variableDec.h"

namespace charlie {
	namespace token {

		enum TokenType
		{
			Bracket,				// (, ), {, }, ...
			Constant,				// "Hello", 12, ...
			Operator,				// +, -, +=, ...
			TypeDeclarer,			// int, bool, ...
			Label,
			ControlFlow				// for, while
		};

		class Base
		{
		public:
			Base(TokenType type);
			TokenType Type;

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

			Bracket(KindEnum kind, DirectionEnum direction);
			KindEnum Kind;
			DirectionEnum Direction;

			virtual std::string ToString();

		};

		class Constant : public Base{
		public:
			enum KindEnum {
				String,			// use std::string
				Integer,		// int
				Decimal,		// float
				Boolean			// bool
			};

			Constant(KindEnum kind, void* pointer);
			~Constant();

			KindEnum Kind;
			void* Pointer;

			virtual std::string ToString();

		};

		class Operator : public Base {
		public:
			enum KindEnum		// TODO: Not complete!
			{
				Add,			// +
				Substract,		// -		// Could be also uniary
				Multipply,		// *
				Divide,			// /
				Copy,			// =
				Equal,			// ==
				NotEqual,		// !=
				Greater,		// >
				GreaterEqual,	// >=
				Less,			// <
				LessEqual,		// <=
				LogicAnd,		// &&
				LogicOr,		// ||
				BitAnd,			// &
				BitOr,			// |
				BitXor,			// ^
				AddTo,			// +=
				SubstractTo,	// -=
				MultiplyTo,		// *=
				DivideTo,		// /=
				AndTo,			// &=
				OrTo,			// |=,
				XorTo			// ^=
			};
			KindEnum Kind;

			Operator(KindEnum kind);

			virtual std::string ToString();
		};

		class ControlFlow : Base {
		public:
			enum KindEnum {
				While,
				For,
				Do,
				If,
				Else,
				Continue,
				Break,
				Return,
				Goto
			};

			ControlFlow(KindEnum kind);

			KindEnum Kind;

			virtual std::string ToString();
		};

		class Declarer : Base 
		{
		public:
			Declarer(program::VariableDec::TypeEnum kind);
			program::VariableDec::TypeEnum Kind;

			virtual std::string ToString();
		};

		class Label : Base {
		public:

			Label(std::string *labelString);
			~Label();
			std::string *LabelString;

			virtual std::string ToString();

		};
	}
}

#endif // !CHARLIE_TOKEN_BASE_H