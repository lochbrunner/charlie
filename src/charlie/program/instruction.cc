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
				state.callStack.push(state.pos);
				int address = state.program[++state.pos];
				state.pos = address;
			};
			types[InstructionEnums::PushConst] = [](State& state) {
				int i = state.program[++state.pos];
				state.aluStack.push(i);
			};
			types[InstructionEnums::Return] = [](State& state) {
				// Only for testing
				state.callStack.pop();
				if (state.callStack.empty())
					state.pos = -2;
				else
					state.pos = state.callStack.top();
			};
			types[InstructionEnums::CallEx] = [](State& state) {
				int id = state.program[++state.pos];
				if(state.pExternalFunctionManager != 0)
					state.pExternalFunctionManager->Invoke(id, state.aluStack);

			};
			types[InstructionEnums::IntAdd] = [](State& state) {
				int a = state.aluStack.top();
				state.aluStack.pop();
				int b = state.aluStack.top();
				state.aluStack.pop();
				state.aluStack.push(a+b);
			};
			types[InstructionEnums::IntMultiply] = [](State& state) {
				int a = state.aluStack.top();
				state.aluStack.pop();
				int b = state.aluStack.top();
				state.aluStack.pop();
				state.aluStack.push(a * b);
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
				comments.push("Pushs");
				break;
			case InstructionEnums::Return:
				comments.push("Returns");
				break;
			case InstructionEnums::CallEx:
				comments.push("Calls external function ...");
				comments.push("... Id of function");
				break;
			case InstructionEnums::Call:
				comments.push("Calls function ...");
				comments.push("... address of function");
				break;
			case InstructionEnums::Jump:
				comments.push("Jumps ...");
				comments.push("... address to jump");
				break;
			case InstructionEnums::PushConst:
				comments.push("Pushs a constant ...");
				comments.push("... value to push");
				break;
			case InstructionEnums::Pop:
				comments.push("Pops from stack and copies to register ...");
				comments.push("... at adress");
				break;
			case InstructionEnums::IntAdd:
				comments.push("Adds two integers");
				break;
			case InstructionEnums::IntSubstract:
				comments.push("Substracts two integers");
				break;
			case InstructionEnums::IntMultiply:
				comments.push("Muliplies two integers");
				break;
			case InstructionEnums::IntDivide:
				comments.push("Divides two integers");
				break;
			default:
				break;
			}
		}
		const array<functionType, InstructionEnums::Length> InstructionManager::Instructions = InstructionManager::Create();

		State::State(): aluStack(), callStack(), reg(), program(), pos(0), pExternalFunctionManager(0){}
	}
}