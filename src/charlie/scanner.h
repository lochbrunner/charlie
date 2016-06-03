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

#ifndef CHARLIE_SCANNER_H
#define CHARLIE_SCANNER_H

#include <string>
#include <list>
#include <set>
#include <map>

#include "common\logginComponent.h"

#include "token\base.h"

#include "program\functionDef.h"
#include "program\variableDec.h"
#include "program\functionDec.h"
#include "program\scopeRoutine.h"

namespace charlie {

	class Scanner : public common::LogginComponent{
	public:
		Scanner();
		Scanner(std::function<void(std::string const &message)> messageDelegate);

		bool Scan(std::string const &code);

		void PrintState();

	private:

		enum WordType {
			None,
			Name,
			Number,
			Operator,
			Comma,
			Semikolon,
			String,
			Char,
			Bracket
		};

		//std::map<program::FunctionDec> _externalFuncDecs;
		std::list<program::FunctionDec> _funcDecs;
		std::list<program::VariableDec> _variableDecs;

		int getFunctionDecArguments(std::string const &code, int begin, int length, std::list<program::VariableDec> &args);
		int getFunctionDefinition(std::string const &code, int length, int pos, program::FunctionDefinition &definition);

		bool getStatement(std::string const &code, int length, int &pos, program::Scope &prog, std::string &word);
		bool getBracket(std::string const &code, int length, int &pos, program::Scope &prog);

		inline bool isLabelBeginning(char c);
		inline bool isAlphaNumeric(char c);
		inline bool isOperator(char c);
		inline bool isNumerical(char c);
		inline bool isBracket(char c);
		void proceessControlSequences(std::string &text);

		// updates word only if the result is longer than 1 char. If not use code[pos++] instead
		void getNextWord(std::string const &code, int length, int &pos, std::string &word, WordType &type);
		int endOfLineComments(std::string const &code, int length, int pos);
		int endOfBlockComments(std::string const &code, int length, int pos);
	};
}


#endif // !CHARLIE_SCANNER_H