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

#include "compiler.h"

using namespace std;
using namespace charlie;

int main(int argn, char** argv)
{
	char* command = NULL;
	char* entry = NULL;

#ifdef _DEBUG
	if (argn > 2) {
		command = argv[1];
		entry = argv[argn-1];
	}
#else
	if (argn > 1) {
		command = argv[0];
		entry = argv[argn - 1];
}
#endif // _DEBUG

	if (command == NULL) {
		cout << "Please specify command";
		return -1;
	}

	if (entry == NULL) {
		cout << "Please specify entry point";
		return -1;
	}

	if (strcmp(command, "build") == 0) {
		Compiler compiler = Compiler([](string const &message) {
			cout << message << endl;
		});

		compiler.ExternalFunctionManager.AddFunction("print", [](const char* message) {
			cout << message;
		});
		compiler.ExternalFunctionManager.AddFunction("println", [](const char* message) {
			cout << message << endl;
		});
		compiler.ExternalFunctionManager.AddFunction("println", [](int number) {
			cout << number << endl;
		});
		compiler.ExternalFunctionManager.AddFunction("print", [](int number) {
			cout << number;
		});

		compiler.Build(entry);
	}

    return 0;
}

