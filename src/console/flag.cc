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

#include "flag.h"

using namespace std;
using namespace charlie::common;

void Flag::Create()
{
	Dict = map<const char*, int, comparer_string>();
	Dict["binary"] = FlagEnum::Binary;
	Dict["-b"] = FlagEnum::Binary;
	Dict["text"] = FlagEnum::Ascii;
	Dict["-a"] = FlagEnum::Ascii;
	Dict["logoutput"] = FlagEnum::LogOutput;
	Dict["-lo"] = FlagEnum::LogOutput;
}

int Flag::Get(const char* command)
{
	auto it = Dict.find(command);
	if (it == Dict.end())
		return FlagEnum::None;
	return it->second;
}


map<const char*, int, comparer_string> Flag::Dict = map<const char*, int, comparer_string>();