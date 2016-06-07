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

#include "vm\instruction.h"


namespace charlie {

	using namespace std;
	using namespace common;
	using namespace program;
	using namespace vm;
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
		if (!io::ascii2string(filename, code)) {
			std::stringstream str;
			str << "Can not open file \"" << filename << "\"";
			logOut(str.str());
			return false;
		}

		Scanner scanner = Scanner(&_program, &ExternalFunctionManager, _messageDelegate);

		if(!scanner.Scan(code)) {
			logOut("Scanning failed!");
			return false;
		}
		if (!compile()) {
			logOut("Compiling failed!");
			return false;
		}

		logOut("Building succeded!");
		return true;
	}

	bool Compiler::SaveProgram(std::string const &filename, bool binary) {
		if(binary)
			return io::saveProgramBinary(filename, _program);
		else
			return io::saveProgramAscii(filename, _program);
	}

	bool Compiler::compile() {
		_program.Instructions.push_back(BYTECODE_VERSION);
		_program.Instructions.push_back(InstructionEnums::Call);
		// Junp address will be inserted at the end
		int count = 2;

		auto funcPositions = std::map<FunctionDec, int, FunctionDec::comparer>();

		for (auto itF = _program.FunctionDecs.begin(); itF != _program.FunctionDecs.end(); ++itF) {
			if (!itF->HasDefinition) {
				stringstream st;
				st << "Missing defintion for function: " << (*itF);
				logging(st.str());
				return false;
			}
			funcPositions.insert(make_pair((*itF), count));
			for (auto itV = itF->Definition.main.VariableDecs.begin(); itV != itF->Definition.main.VariableDecs.end(); ++itV) {
				_program.Instructions.push_back(InstructionEnums::IncreaseRegister);
			}
			
			// Insert variable declaration and defintion of the argument list
			for (auto itI = itF->Definition.main.Statements.begin(); itI != itF->Definition.main.Statements.end(); ++itI) {
				enroleStatement(*itI, count);
			}
			for (auto itV = itF->Definition.main.VariableDecs.begin(); itV != itF->Definition.main.VariableDecs.end(); ++itV) {
				_program.Instructions.push_back(InstructionEnums::DecreaseRegister);
			}
			_program.Instructions.push_back(InstructionEnums::Return);
			++count;
		}
		// Find entryPoint
		auto args = list<VariableDec>();
		args.push_back(VariableDec::Int);
		args.push_back(VariableDec::Char);
		auto main = funcPositions.find(FunctionDec(string("main"), VariableDec::Int, args));
		if (main == funcPositions.end())
		{
			logging("Can not find entry point");
			return false;
		}


		auto third = _program.Instructions.begin();
		++third;
		++third;
		_program.Instructions.insert(third, main->second);
		return true;
	}

	void Compiler::enroleStatement(program::Statement& statement, int& count) {
		auto tokenType = statement.Value->TokenType;
		if (tokenType == Base::TokenTypeEnum::ConstantInt) {
			_program.Instructions.push_back(InstructionEnums::PushConst);
			++count;
			_program.Instructions.push_back(statement.Value->ByteCode());
			delete statement.Value;
			++count;
		}
		else if (tokenType == Base::TokenTypeEnum::Label) {
			if (dynamic_cast<Label*>(statement.Value)->Kind == Label::Function) {
				auto label = dynamic_cast<Label*>(statement.Value);

				auto argTypes = std::list<VariableDec>();
				for (auto it = statement.Arguments.begin(); it != statement.Arguments.end(); ++it)
				{
					argTypes.push_back(it->Value->Type);
					enroleStatement(*it, count);
				}
				auto dec = FunctionDec(label->LabelString, VariableDec::Length, argTypes);
				delete statement.Value;

				int id = ExternalFunctionManager.GetId(dec);
				if (id > -1) {
					_program.Instructions.push_back(InstructionEnums::CallEx);
					++count;
					_program.Instructions.push_back(id);
					++count;
				}
			}
			else
			{
				Label* label = dynamic_cast<Label*>(statement.Value);
				if (label->Address > -1) {
					_program.Instructions.push_back(InstructionEnums::PushConst);
					_program.Instructions.push_back(label->Address);
				}
				else 
				{
					logging("Unknown variable found!");
				}
			}
		}
		else if(tokenType == Base::TokenTypeEnum::Operator)
		{
			for (auto it = statement.Arguments.begin(); it != statement.Arguments.end(); ++it)
				enroleStatement(*it, count);
			_program.Instructions.push_back(statement.Value->ByteCode());
			delete statement.Value;
			++count;
		}
	}

	int Compiler::Run(int argn, char** argv) {
		list<int>::const_iterator it = _program.Instructions.begin();
		if (it == _program.Instructions.end())
			return false;

		auto state = State();
		state.pExternalFunctionManager = &ExternalFunctionManager;
		state.aluStack.push(argn);
		state.aluStack.push(reinterpret_cast<int>(argv));

		int version = (*it++);

		if (version != BYTECODE_VERSION) {
			logging("Wrong bytecode version");
			return -1;
		}

		for (; it != _program.Instructions.end(); ++it) {
			state.program.push_back(*it);
		}

		while (state.pos>-1/* && !state.callStack.empty()*/)
		{
			int r = InstructionManager::Instructions[state.program[state.pos]](state);
			if (r < 0)
				break;
		}
		if (state.aluStack.empty())
			return 0;
		return state.aluStack.top();
	}

	int Compiler::Run() {
		list<int>::const_iterator it = _program.Instructions.begin();
		if (it == _program.Instructions.end())
			return false;

		auto state = State();
		state.pExternalFunctionManager = &ExternalFunctionManager;

		int version = (*it++);

		if (version != BYTECODE_VERSION) {
			logging("Wrong bytecode version");
			return -1;
		}

		for (;it != _program.Instructions.end();++it) {
			state.program.push_back(*it);
		}

		while (state.pos>-1/* && !state.callStack.empty()*/)
		{
			int r = InstructionManager::Instructions[state.program[state.pos]](state);
			if (r < 0)
				break;
		}
		if(state.aluStack.empty())
			return 0;
		return state.aluStack.top();
	}
}