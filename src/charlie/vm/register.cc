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

#include <memory.h>
#include <algorithm>
#include <cassert>

#include "register.h"

namespace charlie::vm {

Register::Register() : data_(nullptr), size_(0), scope_sizes_(), functions_() {}

Register::~Register() {
  if (data_ != nullptr) {
    delete data_;
    data_ = nullptr;
  }
}

bool Register::resize(int change, bool updateFunc) {
  if (change == 0) return true;

  int newSize = size_ + change;
  int *newData = new int[newSize];

  if (memcpy(newData, data_, sizeof(int) * std::min(size_, newSize)) == nullptr) {
    delete[] newData;
    return false;
  }

  delete[] data_;
  data_ = newData;
  size_ = newSize;

  if (updateFunc && !functions_.empty()) {
    functions_.top().size += change;
  }

  return true;
}

bool Register::Increase(int size) {
  if (data_ == nullptr) {
    size_ = size;
    data_ = new int[size_];
    return true;
  }

  if (!resize(size)) return false;

  scope_sizes_.push(size);
  return true;
}

bool Register::Decrease() {
  if (scope_sizes_.empty()) return false;

  int change = -scope_sizes_.top();
  scope_sizes_.pop();

  if (!resize(change)) return false;

  return true;
}

void Register::StoreFunctionScopes() {
  if (!functions_.empty()) {
    functions_.top().data = new int[functions_.top().size];

    const int size = functions_.top().size;

    if (memcpy(functions_.top().data, &data_[size_ - size], sizeof(int) * size) == nullptr) {
      functions_.pop();
    }

    int i = 0;
    while (i < size) {
      functions_.top().scope_sizes_.push(scope_sizes_.top());
      i += scope_sizes_.top();
      scope_sizes_.pop();
    }

    assert(i == size);
    resize(-size, false);
  }
  functions_.push(FunctionScope());
}

void Register::RestoreFunctionScopes() {
  if (functions_.empty()) return;

  functions_.pop();

  if (functions_.empty()) return;

  const int size = functions_.top().size;

  resize(size);

  memcpy(&data_[size_ - size], functions_.top().data, sizeof(int) * size);

  while (!functions_.top().scope_sizes_.empty()) {
    scope_sizes_.push(functions_.top().scope_sizes_.top());
    functions_.top().scope_sizes_.pop();
  }
  functions_.top().clear();
}

bool Register::GetValue(int index, int *value) const {
  if (index >= size_) return false;
  *value = data_[index];
  return true;
}

bool Register::SetValue(int index, int value) {
  if (index >= size_) return false;
  data_[index] = value;
  return true;
}

size_t Register::GetSize() const { return static_cast<size_t>(size_); }

Register::FunctionScope::FunctionScope() : scope_sizes_(), size(), data(nullptr) {}

Register::FunctionScope::~FunctionScope() {
  if (data != nullptr) {
    delete data;
    data = nullptr;
  }
}

void Register::FunctionScope::clear() {
  if (data != nullptr) {
    delete data;
    data = nullptr;
  }
  size = 0;
  while (!scope_sizes_.empty()) scope_sizes_.pop();
}

}  // namespace charlie::vm