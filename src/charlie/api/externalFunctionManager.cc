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

#include "externalFunctionManager.h"

namespace charlie {
	namespace api {
		using namespace std;
		using namespace token;
		using namespace program;

		ExternalFunctionManager::ExternalFunctionManager() : 
			_v_v_external(), _v_i_external(), _v_ccp_external()
		{
		}
		void ExternalFunctionManager::AddFunction(string funcName, function<void(void)> funcPointer)
		{
			list<VariableDec> args = list<VariableDec>();
			FunctionDec dec = FunctionDec(funcName, VariableDec::Void, args);

			_v_v_external[dec] = funcPointer;
		}
		void ExternalFunctionManager::AddFunction(string funcName, function<void(int)> funcPointer)
		{
			list<VariableDec> args = list<VariableDec>();
			args.push_back(VariableDec(VariableDec::Char));
			FunctionDec dec = FunctionDec(funcName, VariableDec::Void, args);

			_v_i_external[dec] = funcPointer;
		}
		void ExternalFunctionManager::AddFunction(string funcName, function<void(const char*)> funcPointer)
		{
			list<VariableDec> args = list<VariableDec>();
			args.push_back(VariableDec(VariableDec::Int));
			FunctionDec dec = FunctionDec(funcName, VariableDec::Void, args);

			_v_ccp_external[dec] = funcPointer;
		}
		bool ExternalFunctionManager::Contains_V_V()
		{
			return false;
		}
		bool ExternalFunctionManager::Contains_V_I()
		{
			return false;
		}
		bool ExternalFunctionManager::Contains_V_CCP()
		{
			return false;
		}
		std::function<void(void)> ExternalFunctionManager::GetV_V()
		{
			return std::function<void(void)>();
		}
		std::function<void(void)> ExternalFunctionManager::GetV_I()
		{
			return std::function<void(void)>();
		}
		std::function<void(void)> ExternalFunctionManager::GetV_CCP()
		{
			return std::function<void(void)>();
		}
	}
}