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

#include "instruction.h"


namespace charlie {
	namespace program {

		using namespace std;

		array<functionType, InstructionEnums::Length> InstructionManager::Create() {
			//EnviromentStruct env = InstructionManager::Enviroment;

			array<functionType, InstructionEnums::Length> types = array<functionType, InstructionEnums::Length>();
			types[InstructionEnums::Call] = [](State& state) {
				int address = state.program[++state.pos];
				state.pos = address;
			};
			types[InstructionEnums::PushConst] = [](State& state) {
				int i = state.program[++state.pos];
				state.st.push(i);
			};
			types[InstructionEnums::Return] = [](State& state) {
				// Only for testing
				state.pos = -2;
			};
			types[InstructionEnums::CallEx] = [](State& state) {
				int id = state.program[++state.pos];
				if(state.pExternalFunctionManager != 0)
					state.pExternalFunctionManager->Invoke(id, state.st);

			};
			types[InstructionEnums::IntAdd] = [](State& state) {
				int a = state.st.top();
				state.st.pop();
				int b = state.st.top();
				state.st.pop();
				state.st.push(a+b);
			};

			

			return types;
		}

		functionType InstructionManager::Get(InstructionEnums bc) {
			return InstructionManager::Instructions[bc];
		}
		void InstructionManager::GetLegend(int instruction, queue<const char*>& comments)
		{
			switch (instruction)
			{
			case InstructionEnums::Push:
				comments.push("Push");
				break;
			case InstructionEnums::Return:
				comments.push("Return");
				break;
			case InstructionEnums::CallEx:
				comments.push("CallEx ...");
				comments.push("... Id of function");
				break;
			case InstructionEnums::Call:
				comments.push("Call ...");
				comments.push("... Address of function");
				break;
			case InstructionEnums::Jump:
				comments.push("Jump ...");
				comments.push("... Address to jump");
				break;
			case InstructionEnums::PushConst:
				comments.push("PushConst ...");
				comments.push("... Value to push");
				break;
			default:
				break;
			}
		}
		const array<functionType, InstructionEnums::Length> InstructionManager::Instructions = InstructionManager::Create();

		State::State(): st(), reg(), program(), pos(0), pExternalFunctionManager(0){}
	}
}