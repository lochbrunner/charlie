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

#ifndef CHARLIE_API_EXTERNALFUNCTION_H
#define CHARLIE_API_EXTERNALFUNCTION_H

#include <map>
#include <set>
#include <string>
#include <stack>
#include <functional>

#include "..\common\exportDefs.h"

#include "..\program\functionDec.h"

namespace charlie {
	namespace api {

		class ExternalFunctionManager {
		public:
			ExternalFunctionManager();

			xprt void AddFunction(std::string funcName, std::function<void(void)> funcPointer);
			xprt void AddFunction(std::string funcName, std::function<void(int)> funcPointer);
			xprt void AddFunction(std::string funcName, std::function<void(const char*)> funcPointer);

			int GetId(program::FunctionDec &dec);

			void Invoke(int id, std::stack<int> &callStack);

			void Invoke(int id);
			void Invoke(int id, int arg1);
			void Invoke(int id, const char* arg1);

		private:
			template<class _Fty>
			struct FunctionInfo {
				FunctionInfo(std::function<_Fty> &pointer, program::FunctionDec &declaration) :
					Pointer(pointer), Declaration(declaration){}
				std::function<_Fty> Pointer;
				program::FunctionDec Declaration;
			};

			std::map<int, FunctionInfo<void(void)>> _pointers_v_v;
			std::map<int, FunctionInfo<void(int)>> _pointers_v_i;
			std::map<int, FunctionInfo<void(const char*)>> _pointers_v_ccp;
			
			std::map<program::FunctionDec, int, program::FunctionDec::comparer> _decs;

			int _curId;
		};
	}
}

#endif // !CHARLIE_API_EXTERNALFUNCTION_H