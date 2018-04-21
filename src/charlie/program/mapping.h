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

#ifndef CHARLIE_PROGRAM_MAPPING_H
#define CHARLIE_PROGRAM_MAPPING_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace charlie::program {
class Mapping {
 public:
  struct Variable {
    std::string name;
    int position;
    std::string type;
  };

  struct Scope {
    int begin;
    int end;
    std::vector<std::unique_ptr<Variable>> variables;
  };

  struct Function {
    Function(const std::string &name) : name(name) {}
    std::string name;
    Scope scope;
  };

  struct Location {
    Location() {}
    Location(int line, int column) : line(line), column(column) {}
    int filename_id;
    int line;
    int column;
  };

  struct Instruction {
    int bytecode_address;
  };

  std::vector<std::unique_ptr<Function>> Functions;
  std::vector<std::unique_ptr<Scope>> Scopes;

  std::unordered_map<int, Location> Instructions;
  std::vector<std::string> Filenames;  // key: filename id

  bool Save(const std::string &filename) const;

  bool Load(const std::string &filename);

 private:
  std::vector<Function> functions_;
};
}  // namespace charlie::program

#endif