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

#include <sstream>

#include "scanner.h"

#include "common\io.h"
#include "common\definitions.h"

#include "program\instruction.h"


namespace charlie {

	using namespace std;
	using namespace common;
	using namespace program;
	using namespace token;

	Compiler::Compiler() : 
		LogginComponent(), ExternalFunctionManager(), _program() 
	{
	}

	Compiler::Compiler(function<void(string const&message)> messageDelegate) : 
		LogginComponent(messageDelegate), ExternalFunctionManager(), _program()
	{
	}
	
	bool Compiler::Build(string const &filename)
	{
		_program.Clear();
		string code;
		if (!ascii2string(filename, code)) {
			std::stringstream str;
			str << "Can not open file \"" << filename << "\"";
			log(str.str());
			return false;
		}

		Scanner scanner = Scanner(&_program, &ExternalFunctionManager, _messageDelegate);

		if(!scanner.Scan(code)) {
			log("Scanning failed!");
			return false;
		}
		if (!compile()) {
			log("Compiling failed!");
			return false;
		}

		log("Building succeded!");
		return true;
	}

	bool Compiler::compile() {
		_program.Instructions.push_back(BYTECODE_VERSION);
		_program.Instructions.push_back(InstructionEnums::Jump);
		// Junp address will be inserted at the end
		int count = 3;

		auto funcPositions = std::map<FunctionDec, int, FunctionDec::comparer>();

		for (auto itF = _program.FunctionDecs.begin(); itF != _program.FunctionDecs.end(); ++itF) {
			if (!itF->HasDefinition) {
				stringstream st;
				st << "Missing defintion for function: " << (*itF);
				log(st.str());
				return false;
			}
			funcPositions.insert(make_pair((*itF), count));
			for (auto itI = itF->Definition.main.Instructions.begin(); itI != itF->Definition.main.Instructions.end(); ++itI) {
				_program.Instructions.push_back(*itI);
				++count;
			}
		}
		// Find entryPoint
		auto args = list<VariableDec>();
		args.push_back(VariableDec::Int);
		args.push_back(VariableDec::Char);
		auto main = funcPositions.find(FunctionDec(string("main"), VariableDec::Int, args));
		if (main == funcPositions.end())
		{
			log("Can not find entry point");
			return false;
		}


		auto third = _program.Instructions.begin();
		++third;
		++third;
		_program.Instructions.insert(third, main->second);
		return true;
	}
}