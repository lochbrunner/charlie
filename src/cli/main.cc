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
#include <string>

#include <boost/program_options.hpp>

#include "log.h"

#include "common/comparer_string.h"
#include "compiler.h"
#include "vm/runtime.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

using charlie::Compiler;

namespace po = boost::program_options;

// Adds console output functions to the compiler
void addExternalFunctions(Compiler *compiler) {
  compiler->external_function_manager.AddFunction("print", [](const char *message) {
    cout << message;
    Log::buffer << message;
  });
  compiler->external_function_manager.AddFunction("println", [](const char *message) {
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
int main(int argn, char **argv) {
  po::options_description global("Global options");
  global.add_options()("help", "produce help message")("command", po::value<std::string>(), "command to execute")(
      "subargs", po::value<std::vector<std::string>>(), "Arguments for command");

  po::positional_options_description pos;
  pos.add("command", 1).add("subargs", -1);

  po::variables_map vm;

  auto parsed = po::command_line_parser(argn, argv).options(global).positional(pos).allow_unregistered().run();

  po::store(parsed, vm);

  auto cmd = vm["command"].as<std::string>();
  if (vm.count("help") > 0) {
    cerr << global;
  } else if (cmd == "run") {
    po::positional_options_description run_pos;
    run_pos.add("file", -1);

    po::options_description run_desc("run options");
    run_desc.add_options()("log,l", "logs the output")("ascii,a", "saves the program in ascii format")(
        "binary,b", "saves the program in binary format")("file", po::value<std::string>(), "Arguments for command");

    auto opts = po::collect_unrecognized(parsed.options, po::include_positional);
    opts.erase(opts.begin());

    // Args for run command
    po::store(po::command_line_parser(opts).options(run_desc).positional(run_pos).run(), vm);
    po::notify(vm);

    auto file = vm["file"].as<std::string>();

    Compiler compiler([](string const &message) { cerr << message << endl; });

    addExternalFunctions(&compiler);
    if (compiler.Build(file)) {
      if (vm.count("ascii") > 0) {
        if (compiler.SaveProgram(file, false)) cerr << "Saving program to " << file << ".bc.txt" << endl;
      }
      if (vm.count("bingitary") > 0) {
        if (compiler.SaveProgram(file, true)) cerr << "Saving program to " << file << ".bc" << endl;
      }
      cerr << "Running program ..\n\n";

      charlie::vm::Runtime runtime(compiler.GetProgram());
      int result = runtime.Run();
      cerr << endl;
      if (vm.count("log") > 0) Log::Save(file);
      if (result != 0) cerr << "Program exited with " << result << endl;
      return result;
    }
  } else {
    cerr << "'" << cmd << "' is not a charlie command. See 'charlie --help'." << cmd << endl;
    cerr << "Available commands are:\n  run\n";
  }

  return 0;
}
