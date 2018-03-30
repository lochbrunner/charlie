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

#include <map>
#include <string>
#include <iostream>

#include "flag.h"
#include "command.h"
#include "log.h"

#include "compiler.h"
#include "common/comparer_string.h"

using std::string;
using std::cout;
using std::endl;

using charlie::Compiler;


// Adds console output functions to the compiler
void addExternalFunctions(Compiler *compiler) {
  compiler->external_function_manager.AddFunction("print", [](const char* message) {
    cout << message;
    Log::buffer << message;
  });
  compiler->external_function_manager.AddFunction("println", [](const char* message) {
    cout << message << endl;
    Log::buffer << message << endl;
  });
  compiler->external_function_manager.AddFunction("println", [](int number) {
    cout << number << endl;
    Log::buffer << number << endl;
  });
  compiler->external_function_manager.AddFunction("print", [](int number) {
    cout << number;
    Log::buffer << number;
  });
}


// <command> <filename> <options>
int main(int argn, char** argv) {
  Command::CommandEnum command = Command::None;
  int flag = Flag::None;
  char* entry = NULL;

  Command::Create();
  Flag::Create();


#ifndef _DEBUG
  if (argn > 2) {
    command = Command::Get(argv[1]);
    entry = argv[2];
  }
  for (int i = 3; i < argn; ++i) {
    flag |= Flag::Get(argv[i]);
  }
#else
  if (argn > 1) {
    command = Command::Get(argv[0]);
    entry = argv[1];
  }
  for (int i = 2; i < argn; ++i) {
    flag |= Flag::Get(argv[i]);
  }
#endif  // _DEBUG

  if (command == Command::None) {
    cout << "Please specify a command\n";
    return -1;
  }

  if (entry == NULL) {
    cout << "Please specify an entry point\n";
    return -1;
  }

  int result = 0;
  if (command == Command::Build) {
    Compiler compiler([](string const &message) {
      cout << message << endl;
    });

    addExternalFunctions(&compiler);
    if (compiler.Build(entry)) {
      if (compiler.SaveProgram(entry, flag & Flag::Binary))
        cout << "Saving program to " << entry << (flag & Flag::Binary ? ".bc" : ".bc.txt") << endl;
      cout << "Running program ..\n\n";
      result = compiler.Run();
      cout << endl;
      if (flag & Flag::LogOutput)
        Log::Save(entry);
    }
  }

  return result;
}

