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

#include "functionDec.h"

#include <list>

namespace charlie {

	namespace program {

		using namespace std;

		FunctionDec::FunctionDec(std::string &label, VariableDec::TypeEnum imageType, std::list<VariableDec> &argumentType)
			: Label(label), ImageType(imageType), ArgumentType(argumentType), HasDefinition(false){}
		FunctionDec::FunctionDec(std::string &label, VariableDec::TypeEnum imageType)
			: Label(label), ImageType(imageType), HasDefinition(false)
		{
			ArgumentType = std::list<VariableDec>();
		}
		std::ostream& operator<<(std::ostream & stream, const FunctionDec &dec)
		{
			// See: http://stackoverflow.com/questions/476272/how-to-properly-overload-the-operator-for-an-ostream
			stream << VariableDec::TypeString(dec.ImageType) << '@' << dec.Label << '(';
			bool first = true;
			for (list<VariableDec>::const_iterator it = dec.ArgumentType.begin(); it != dec.ArgumentType.end(); ++it) {
				if (!first)
					stream << '@';
				else
					first = false;
				stream << VariableDec::TypeString(it->ImageType);
				return stream;
			}
			stream << ')';
			return stream;
		}
		// Is a "smaller" than b
		bool FunctionDec::comparer::operator()(const FunctionDec & a, const FunctionDec & b)
		{
			// See: http://stackoverflow.com/questions/5733254/create-an-own-comparator-for-map
			int name = strcmp(a.Label.c_str(), b.Label.c_str());
			if (name != 0)
				return name < 0;

			list<VariableDec>::const_iterator itA = a.ArgumentType.begin();
			list<VariableDec>::const_iterator itB = b.ArgumentType.begin();
			while (itA != a.ArgumentType.end() && itB != b.ArgumentType.end())
			{
				if (itA->ImageType != itB->ImageType)
					return itA->ImageType - itB->ImageType < 0;
				++itA;
				++itB;
			}
			if (itB != b.ArgumentType.end())
				return true;

			return false;
		}
	}
}