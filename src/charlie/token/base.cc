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

namespace charlie {
	namespace token {

		using namespace std;

		Base::Base(TokenType type) : Type(type){}

		Bracket::Bracket(KindEnum kind, DirectionEnum direction) : Base(TokenType::Bracket), Kind(kind), Direction(direction) {}
		std::string Bracket::ToString() {
			return string();
		}

		Constant::Constant(KindEnum kind, void* pointer) : Base(TokenType::Constant), Kind(kind), Pointer(pointer){}
		Constant::~Constant() {
			if (Pointer != 0) {
				delete Pointer;
			}
		}
		std::string Constant::ToString() {
			return string();
		}

		Operator::Operator(KindEnum kind) : Base(TokenType::Operator), Kind(kind) {}
		std::string Operator::ToString() {
			return string();
		}
		Declarer::Declarer(KindEnum kind) : Base(TokenType::TypeDeclarer), Kind(kind) {}
		std::string Declarer::ToString() {
			return string();
		}
		Label::Label(string *labelString) : Base(TokenType::TypeDeclarer), LabelString(labelString) {}
		Label::~Label() {
			if (LabelString != 0) {
				delete LabelString;
			}
		}
		std::string Label::ToString() {
			return string();
		}
	}
}


