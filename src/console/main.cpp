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

#include <iostream>
#include <map>

#include "flag.h"
#include "command.h"

#include "compiler.h"
#include "common\comparer_string.h"

using namespace std;
using namespace charlie;

void addExternalFunctions(Compiler &compiler)
{
	compiler.ExternalFunctionManager.AddFunction("print", [](const char* message) 
	{
		cout << message;
	});
	compiler.ExternalFunctionManager.AddFunction("println", [](const char* message) 
	{
		cout << message << endl;
	});
	compiler.ExternalFunctionManager.AddFunction("println", [](int number) 
	{
		cout << number << endl;
	});
	compiler.ExternalFunctionManager.AddFunction("print", [](int number) 
	{
		cout << number;
	});
}



// <command> <filename> <options>
int main(int argn, char** argv)
{
	Command::CommandEnum command = Command::None;
	int flag = Flag::None;
	char* entry = NULL;

	Command::Create();
	Flag::Create();


#ifdef _DEBUG
	if (argn > 2) 
	{
		command = Command::Get(argv[1]);
		entry = argv[2];
	}
	for (int i = 3; i < argn; ++i) 
	{
		flag |= Flag::Get(argv[i]);
	}
#else
	if (argn > 1) 
	{
		command = Commands::Get(argv[0]);
		entry = argv[1];
	}
	for (int i = 2; i < argn; ++i)
	{
		flag |= Flags::Get(argv[i]);
	}
#endif // _DEBUG

	if (command == Command::None)
	{
		cout << "Please specify a command";
		return -1;
	}

	if (entry == NULL) 
	{
		cout << "Please specify an entry point";
		return -1;
	}

	if (command == Command::Build)
	{
		Compiler compiler = Compiler([](string const &message) {
			cout << message << endl;
		});

		addExternalFunctions(compiler);

		compiler.Build(entry);
		compiler.SaveProgram(entry, false);
	}

    return 0;
}

