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

#include "scopeRoutine.h"

namespace charlie {
	namespace program {
		using namespace std;

		Scope::Scope(Scope* pParant) :
			Statements(), VariableDecs(), CountVariableDecs(0), _pParant(pParant)
		{
		}
		Scope::VariableInfo Scope::GetVariableInfo(VariableDec & dec)
		{
			auto it = VariableDecs.find(dec);
			if (it == VariableDecs.end())
			{
				if(_pParant == 0)
					return VariableInfo(0, VariableDec::TypeEnum::Length);
				return _pParant->GetVariableInfo(dec);
			}
			int pos = it->second;
			auto par = _pParant;

			return VariableInfo([=](){ 
				if (par == 0)
					return pos;
				return pos + par->ParentOffset() + par->CountVariableDecs;
			}, it->first.ImageType);
		}
		int Scope::AddVariableDec(VariableDec& dec)
		{
			VariableDecs.insert(make_pair(dec, CountVariableDecs++));
			return CountVariableDecs;
		}

		int Scope::ParentOffset() const
		{
			if (_pParant == 0)
				return 0;
			return _pParant->ParentOffset() + _pParant->CountVariableDecs;
		}
		Scope::VariableInfo::VariableInfo(std::function<int()> offset, VariableDec::TypeEnum type) : Offset(offset), Type(type)
		{
		}
	}
}