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

#include "statement.h"

//#include "..\token\base.h"
#include "scope.h"

namespace charlie::program {

using token::Base;

Statement::Statement(Base* value) : value(value), block(nullptr), arguments() {}
Statement::Statement(Base* value, const Mapping::Location& location)
    : value(value), block(nullptr), arguments(), location(location) {}

Statement::Statement(Scope* block) : value(nullptr), block(block), arguments() {}

Statement::Statement() : value(nullptr), block(nullptr), arguments() {}

Statement::~Statement() {}

void Statement::Dispose() {
  if (value != nullptr) {
    delete value;
    value = nullptr;
  }
  if (block != nullptr) {
    delete block;
    block = nullptr;
  }
}

bool Statement::Finished() const {
  if (block != nullptr) return true;
  if (value == nullptr) return false;
  return value->finished;
}

int Statement::priority() const {
  if (value == nullptr || value->finished) return 0;
  return value->priority;
}
}  // namespace charlie::program
