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

#ifndef CHARLIE_TOKEN_FUNCTIONDEC_H
#define CHARLIE_TOKEN_FUNCTIONDEC_H

#include <list>
#include <string>
#include <sstream>

#include "variableDec.h"
#include "functionDef.h"

namespace charlie {

	namespace program {

		class FunctionDec {
		public:
			VariableDec::TypeEnum ImageType;
			std::list<program::VariableDec> ArgumentType;
			std::string Label;
			bool HasDefinition;

			FunctionDec(std::string &label, VariableDec::TypeEnum imageType, std::list<VariableDec> &argumentType);
			FunctionDec(std::string &label, VariableDec::TypeEnum imageType);

			FunctionDefinition Definition;

			struct comparer {
				bool operator()(const FunctionDec &a, const FunctionDec &b);
			};

			friend std::ostream& operator<<(std::ostream &stream, const FunctionDec &dec);
		};
	}
}


#endif // !CHARLIE_TOKEN_FUNCTIONDEC_H