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

#include "variableDec.h"


namespace charlie {

	namespace program {
		VariableDec::VariableDec(TypeEnum imageType) : ImageType(imageType) {
			Name = "";
		}
		VariableDec::VariableDec(std::string name, TypeEnum imageType)
			: Name(name), ImageType(imageType) {}

		
		const char* typeStringArray[VariableDec::TypeEnum::Length] = {
			"Int",
			"Long",
			"Float",
			"Double",
			"Boolean",
			"Char",
			"ConstCharPointer",
			"Void"
		};

		const char* undefined = "undefined";

		const char * VariableDec::TypeString(TypeEnum type)
		{
			if (type > VariableDec::TypeEnum::Length - 2)
				return undefined;
			return typeStringArray[type];
		}

		bool VariableDec::comparer::operator()(const VariableDec& a, const VariableDec& b) {
			if (a.ImageType == b.ImageType)
				return std::strcmp(a.Name.c_str(), b.Name.c_str()) < 0;
			return  a.ImageType < b.ImageType;
		}
		bool VariableDec::comparer_only_type::operator()(const VariableDec& a, const VariableDec& b)
		{
			return a.ImageType < b.ImageType;
		}

		bool VariableDec::comparer_only_name::operator()(const VariableDec& a, const VariableDec& b)
		{
			return std::strcmp(a.Name.c_str(), b.Name.c_str()) < 0;
		}
	}
}