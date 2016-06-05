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

#ifndef CHARLIE_PROGRAM_INSTRUCTION_H
#define CHARLIE_PROGRAM_INSTRUCTION_H

#include <functional>

#include <array>
#include <stack>
#include <queue>
#include <vector>

#include "..\api\externalFunctionManager.h"

namespace charlie {
	namespace program {

		struct State {
			State();
			std::stack<int> aluStack;
			std::stack<int> callStack;
			std::vector<int> reg;
			std::vector<int> program;
			int pos;
			api::ExternalFunctionManager *pExternalFunctionManager;
		};

		enum InstructionEnums
		{
			PushConst,
			Push,
			Pop,
			Call,
			CallEx,
			Jump,
			IntAdd,
			IntSubstract,
			IntMultiply,
			IntDivide,
			Return,
			Length
		};

		typedef std::function<void(State&)> functionType;

		struct InstructionManager {
			static std::array<functionType, InstructionEnums::Length> Create();
			static const std::array<functionType, InstructionEnums::Length> Instructions;
			static functionType Get(InstructionEnums bc);
			static void GetLegend(int instruction, std::queue<const char*> &comments);
		};
	}
}


#endif // !CHARLIE_PROGRAM_INSTRUCTION_H