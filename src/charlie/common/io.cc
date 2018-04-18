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

#include "io.h"
#include <fstream>
#include <list>
#include <queue>
#include <sstream>

#include "../vm/instruction.h"

using std::ifstream;
using std::ios;
using std::list;
using std::ofstream;
using std::queue;
using std::string;

namespace charlie::common::io {
bool ascii2string(std::string const& filename, std::string* result) {
  string fullfilename = filename;
  // fullfilename.append(".cc");
  ifstream file(fullfilename);
  if (!file.is_open()) {
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();
  *result = buffer.str();

  return true;
}
bool saveProgramAscii(std::string const& filename, program::UnresolvedProgram const& program) {
  auto it = program.instructions.cbegin();
  if (it == program.instructions.end()) return false;

  std::string fullfilename = filename;
  fullfilename.append(".bc.txt");
  ofstream file(fullfilename);
  if (!file.is_open()) {
    return false;
  }
  file << (*it) << "\t// Version\n";
  auto comments = queue<const char*>();
  for (++it; it != program.instructions.cend(); ++it) {
    if (comments.empty()) vm::InstructionManager::GetLegend((*it), &comments);
    file << (*it);
    if (!comments.empty()) {
      file << "\t// " << comments.front();
      comments.pop();
    }
    file << "\n";
  }

  file.close();
  return true;
}
bool saveProgramBinary(std::string const& filename, program::UnresolvedProgram const& program) {
  auto fullfilename = filename;
  fullfilename.append(".bc");
  ofstream file(fullfilename, ios::binary | ios::out | ios::trunc);
  if (!file.is_open()) {
    return false;
  }
  auto it = program.instructions.cbegin();
  for (; it != program.instructions.cend(); ++it) {
    file << (*it);
  }

  file.close();
  return true;
}
}  // namespace charlie::common::io
