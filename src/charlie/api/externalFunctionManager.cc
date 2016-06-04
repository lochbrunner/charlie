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
#include <sstream>
#include <utility>

namespace charlie {
	namespace api {
		using namespace std;
		using namespace program;


		ExternalFunctionManager::ExternalFunctionManager() : 
			_curId(0),
			_pointers_v_v(), _pointers_v_i(), _pointers_v_ccp(), 
			_decs()
		{
		}
		void ExternalFunctionManager::AddFunction(string funcName, function<void(void)> funcPointer)
		{
			list<VariableDec> args = list<VariableDec>();
			FunctionDec dec = FunctionDec(funcName, VariableDec::Void, args);

			int id = _curId++;
			_decs[dec] = id;
			_pointers_v_v.insert(make_pair(id, FunctionInfo<void(void)>(funcPointer, dec)));
		}
		void ExternalFunctionManager::AddFunction(string funcName, function<void(int)> funcPointer)
		{
			list<VariableDec> args = list<VariableDec>();
			args.push_back(VariableDec(VariableDec::Int));
			FunctionDec dec = FunctionDec(funcName, VariableDec::Void, args);

			int id = _curId++;
			_decs[dec] = id;
			_pointers_v_i.insert(make_pair(id, FunctionInfo<void(int)>(funcPointer, dec)));
		}
		void ExternalFunctionManager::AddFunction(string funcName, function<void(const char*)> funcPointer)
		{
			list<VariableDec> args = list<VariableDec>();
			args.push_back(VariableDec(VariableDec::ConstCharPointer));
			FunctionDec dec = FunctionDec(funcName, VariableDec::Void, args);

			int id = _curId++;
			_decs[dec] = id;
			_pointers_v_ccp.insert(make_pair(id, FunctionInfo<void(const char*)>(funcPointer, dec)));
		}

		int ExternalFunctionManager::GetId(FunctionDec &dec)
		{
			auto it = _decs.find(dec);
			if (it == _decs.end())
				return -1;
			return it->second;
		}

		void ExternalFunctionManager::Invoke(int id, std::stack<int>& callStack)
		{
			auto it_v_v = _pointers_v_v.find(id);
			if (it_v_v != _pointers_v_v.end()) {
				it_v_v->second.Pointer();
				return;
			}
			auto it_v_i = _pointers_v_i.find(id);
			if (it_v_i != _pointers_v_i.end()) {
				int i = callStack.top();
				callStack.pop();
				it_v_i->second.Pointer(i);
				return;
			}
			auto it_v_cpp = _pointers_v_ccp.find(id);
			if (it_v_cpp != _pointers_v_ccp.end()) {
				int i = callStack.top();
				callStack.pop();
				it_v_cpp->second.Pointer(reinterpret_cast<const char*>(i));
				return;
			}
		}

		void ExternalFunctionManager::Invoke(int id)
		{
			auto it = _pointers_v_v.find(id);
			if (it == _pointers_v_v.end())
				return;
			it->second.Pointer();
		}

		void ExternalFunctionManager::Invoke(int id, int arg1)
		{
			auto it = _pointers_v_i.find(id);
			if (it == _pointers_v_i.end())
				return;
			it->second.Pointer(arg1);
		}

		void ExternalFunctionManager::Invoke(int id, const char * arg1)
		{
			auto it = _pointers_v_ccp.find(id);
			if (it == _pointers_v_ccp.end())
				return;
			it->second.Pointer(arg1);
		}
	}
}