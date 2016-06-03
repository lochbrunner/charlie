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
#include <string>
#include <functional>

#include "../program\functionDec.h"
#include "../common/exportDefs.h"

namespace charlie {
	namespace api {
		class ExternalFunctionManager {
		public:
			ExternalFunctionManager();

			xprt void AddFunction(std::string funcName, std::function<void(void)> funcPointer);
			xprt void AddFunction(std::string funcName, std::function<void(int)> funcPointer);
			xprt void AddFunction(std::string funcName, std::function<void(const char*)> funcPointer);

			bool Contains_V_V();
			bool Contains_V_I();
			bool Contains_V_CCP();

			std::function<void(void)> GetV_V();
			std::function<void(void)> GetV_I();
			std::function<void(void)> GetV_CCP();

		private:
			std::map<program::FunctionDec, std::function<void(void)>, program::FunctionDec::comparer> _v_v_external;
			std::map<program::FunctionDec, std::function<void(int)>, program::FunctionDec::comparer> _v_i_external;
			std::map<program::FunctionDec, std::function<void(const char*)>, program::FunctionDec::comparer> _v_ccp_external;
		};
	}
}

#endif // !CHARLIE_API_EXTERNALFUNCTION_H