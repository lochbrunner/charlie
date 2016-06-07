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

#ifndef  CHARLIE_TOKEN_VARIABLEDEC_H
#define CHARLIE_TOKEN_VARIABLEDEC_H

#include <string>

namespace charlie {

	namespace program {

		class VariableDec {
		public:
			enum TypeEnum {
				Int,
				Long,
				Float,
				Double,
				Boolean,
				Char,
				ConstCharPointer,
				Void,
				Length
			};

			struct comparer {
				bool operator()(const VariableDec& a, const VariableDec& b);
			};

			struct comparer_only_type {
				bool operator()(const VariableDec& a, const VariableDec& b);
			};

			struct comparer_only_name {
				bool operator()(const VariableDec& a, const VariableDec& b);
			};

			std::string Name;
			TypeEnum ImageType;

			VariableDec(TypeEnum imageType);
			VariableDec(std::string name, TypeEnum imageType);

			static const char* TypeString(TypeEnum type);
		};
	}
}

#endif // ! CHARLIE_TOKEN_VARIABLEDEC_H