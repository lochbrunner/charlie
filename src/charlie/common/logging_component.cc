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

#include "logging_component.h"

#include <assert.h>

#include <sstream>
#include <iomanip>

namespace charlie {
namespace common {

using std::string;
using std::stringstream;
using std::function;
using std::setfill;
using std::setw;


char significantChar(const char* codefileName) {
  auto path = string(codefileName);
  string base_filename = path.substr(path.find_last_of("/\\") + 1);
  return static_cast<char>(toupper(path[path.find_last_of("/\\") + 1]));
}

LoggingComponent::CodeFileInfo::CodeFileInfo(const std::string* code) : code(code), pos(0) {
  if (code != nullptr)
    this->length = code->length();
}

void LoggingComponent::CodeFileInfo::set(const std::string* code) {
  this->code = code;
  if (code != nullptr)
    this->length = code->length();
}

const char LoggingComponent::CodeFileInfo::at(int pos) const {
  if (code != nullptr)
    return code->at(pos);
  return '\0';
}

const char LoggingComponent::CodeFileInfo::current_char() const {
  if (code != nullptr)
    return code->at(pos);
  return '\0';
}

const bool LoggingComponent::CodeFileInfo::valid() const {
  return pos < length;
}

LoggingComponent::LoggingComponent() : _messageDelegate(0), codeInfo_(nullptr) {
}

LoggingComponent::LoggingComponent(function<void(string const&message)> messageDelegate) :
  _messageDelegate(messageDelegate), codeInfo_(nullptr) {
}

void LoggingComponent::error_message_to_code(string const& message) const {
  if (_messageDelegate != NULL) {
    stringstream st;
    st << message;
    getPositionString(&st);
    _messageDelegate(st.str());
  }
}

void LoggingComponent::error_message_to_code(stringstream const& message) const {
  if (_messageDelegate != NULL) {
    stringstream st;
    st << message.str();
    getPositionString(&st);
    _messageDelegate(st.str());
  }
}

void LoggingComponent::error_message_to_code(string const& message, int pos) const {
  if (_messageDelegate != NULL) {
    stringstream st;
    st << message;
    getPositionString(pos, &st);
    _messageDelegate(st.str());
  }
}

void LoggingComponent::error_message_to_code(stringstream const& message, int pos) const {
  if (_messageDelegate != NULL) {
    stringstream st;
    st << message.str();
    getPositionString(pos, &st);
    _messageDelegate(st.str());
  }
}

void LoggingComponent::error_message(string const& message) const {
  if (_messageDelegate != NULL)
    _messageDelegate(message);
}

void LoggingComponent::error_message(stringstream const& message) const {
  if (_messageDelegate != NULL)
    _messageDelegate(message.str());
}

void LoggingComponent::error_message_to_code(string const& message, const char *codefileName, int lineNumber) const {
  if (_messageDelegate != NULL) {
    stringstream st;
    st << significantChar(codefileName) << setfill('0') << setw(4) << lineNumber << ": " << message;
    getPositionString(&st);
    _messageDelegate(st.str());
  }
}

void LoggingComponent::error_message_to_code(stringstream const& message, const char *codefileName, int lineNumber) const {
  if (_messageDelegate != NULL) {
    stringstream st;
    st << significantChar(codefileName) << setfill('0') << setw(4) << lineNumber << ": " << message.str();
    getPositionString(&st);
    _messageDelegate(st.str());
  }
}

void LoggingComponent::error_message_to_code(string const& message, int pos, const char *codefileName, int lineNumber) const {
  if (_messageDelegate != NULL) {
    stringstream st;
    st << significantChar(codefileName) << setfill('0') << setw(4) << lineNumber << ": " << message;
    getPositionString(pos, &st);
    _messageDelegate(st.str());
  }
}

void LoggingComponent::error_message_to_code(stringstream const& message, int pos, const char *codefileName, int lineNumber) const {
  if (_messageDelegate != NULL) {
    stringstream st;
    st << significantChar(codefileName) << setfill('0') << setw(4) << lineNumber << ": " << message.str();
    getPositionString(pos, &st);
    _messageDelegate(st.str());
  }
}

void LoggingComponent::error_message(string const& message, const char *codefileName, int lineNumber) const {
  if (_messageDelegate != NULL) {
    stringstream st;
    st << significantChar(codefileName) << setfill('0') << setw(4) << lineNumber << ": " << message;
    _messageDelegate(st.str());
  }
}

void LoggingComponent::error_message(stringstream const& message, const char *codefileName, int lineNumber) const {
  if (_messageDelegate != NULL) {
    stringstream st;
    st << significantChar(codefileName) << setfill('0') << setw(4) << lineNumber << ": " << message.str();
    _messageDelegate(st.str());
  }
}

void LoggingComponent::getPositionString(std::stringstream *st) const {
  if (codeInfo_.pos < 0) {
    *st << " at unspecifed position";
    return;
  }
  int line;
  int column;
  getLineNumberAndColumnPos(&line, &column);

  *st << " at line: " << line << " columns: " << column;
}

void LoggingComponent::getLineNumberAndColumnPos(int *line, int *column) const {
  if (codeInfo_.length != 0) {
    *column = 0;
    *line = 1;
    if (codeInfo_.length <= codeInfo_.pos)
      return;
    for (int i = 0; i <= codeInfo_.pos; ++i) {
      if (codeInfo_.at(i) == '\n') {
        ++*line;
        column = 0;
      } else {
        if (codeInfo_.at(i) == '\r')
          continue;
      }
      ++column;
    }
  }
}

void LoggingComponent::getPositionString(int pos, std::stringstream *st) const {
  if (pos < 0) {
    *st << " at unspecifed position";
    return;
  }
  int line;
  int column;
  getLineNumberAndColumnPos(&line, &column);

  *st << " at line: " << line << " columns: " << column;
}

void LoggingComponent::getLineNumberAndColumnPos(int pos, int *line, int *column) const {
  if (codeInfo_.length != 0) {
    *column = 0;
    *line = 1;
    if (codeInfo_.length <= pos)
      return;
    for (int i = 0; i <= pos; ++i) {
      if (codeInfo_.at(i) == '\n') {
        ++*line;
        *column = 0;
      } else {
        if (codeInfo_.at(i) == '\r')
          continue;
      }
      ++*column;
    }
  }
}

}  // namespace common
}  // namespace charlie
