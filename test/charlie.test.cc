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

#include "compiler.h"
#include "gtest/gtest.h"
#include "scanner.h"

#include "program/unresolved_program.h"

#include "api/external_function_manager.h"

namespace charlie {

TEST(ScannerTest, getNextWord) {
  auto program = program::UnresolvedProgram();
  auto funcManager = api::ExternalFunctionManager();

  std::string const code = " a 12 ()";

  std::string word;
  Scanner::WordType type;

  Scanner scanner = Scanner(&program, &funcManager);

  scanner.codeInfo_.set(&code);

  scanner.getNextWord(&word, &type);
  EXPECT_EQ(word, "a");
  EXPECT_EQ(type, Scanner::WordType::Name);

  scanner.getNextWord(&word, &type);
  EXPECT_EQ(word, "12");
  EXPECT_EQ(type, Scanner::WordType::Number);

  scanner.getNextWord(&word, &type);
  EXPECT_EQ(code[scanner.codeInfo_.pos], '(');
  EXPECT_EQ(type, Scanner::WordType::Bracket);
}

}  // namespace charlie
