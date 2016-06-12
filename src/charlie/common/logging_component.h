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

#ifndef CHARLIE_COMMON_LOGGIN_COMPONENT_H
#define CHARLIE_COMMON_LOGGIN_COMPONENT_H

#include <string>
#include <sstream>
#include <functional>

#include "..\token\base.h"

#include "..\common\exportDefs.h"

namespace charlie {
namespace common {
// Base class for all components which works on code files and uses logging functionality
class LoggingComponent {
 public:
  // Creates an object with disabled logging.
  LoggingComponent();
  // Creates an object which delegates the messages to the specified function pointer.
  LoggingComponent(std::function<void(std::string const &message)> messageDelegate);

 protected:
  // Stores all the relevant informations of the source code while proceeding its lexing and compiling.
  // Used to manange the compile error message output.
  struct CodeFileInfo {
    // Creates an object to the corresponding code string
    xprt CodeFileInfo(const std::string *code);
    // Sets a new code string and resets the members.
    xprt void set(const std::string *code);
    // Returns the character at the specified index position.
    const char at(int pos) const;
    // Returns the character at the current caret.
    const char current_char() const;
    // Validates the caret.
    const bool valid() const;
    // Pointer to the current code string.
    const std::string *code;
    // Caret position
    int pos;
    // Stores the length of the current code to avoid additional function calls to std::string::length().
    int length;
  };
  // Send the specifed message to the delegate if set
  // and includes the code position.
  // Optional generates compile error code out of the file and linenumber of the compiler implementation.
  // In order to find the position where this message was raised much faster. E.g. "C0124"
  void error_message_to_code(std::string const& message) const;
  void error_message_to_code(std::stringstream const& message) const;
  void error_message_to_code(std::string const& message, int pos) const;
  void error_message_to_code(std::stringstream const& message, int pos) const;
  void error_message_to_code(std::string const& message, const char *codefileName, int lineNumber) const;
  void error_message_to_code(std::stringstream const& message, const char *codefileName, int lineNumber) const;
  void error_message_to_code(std::string const& message, int pos, const char *codefileName, int lineNumber) const;
  void error_message_to_code(std::stringstream const& message, int pos, const char *codefileName, int lineNumber) const;
  // Send the specifed message to the delegate if set.
  // Optional generates compile error code out of the file and linenumber of the compiler implementation.
  // In order to find the position where this message was raised much faster. E.g. C0124 in order
  void error_message(std::string const& message, const char *codefileName, int lineNumber) const;
  void error_message(std::stringstream const& message, const char *codefileName, int lineNumber) const;
  void error_message(std::string const& message) const;
  void error_message(std::stringstream const& message) const;
  // Current code information object.
  CodeFileInfo codeInfo_;
  // A function pointer to the message output. If this is null, no message can be print.
  std::function<void(std::string const& message)> _messageDelegate;

 private:
  // Gets the line number and column position of the caret.
  void getLineNumberAndColumnPos(int *line, int *column) const;
  // Adds the information of the caret position to the stringstream.
  void getPositionString(std::stringstream *st) const;
  // Gets the line number and column of the specified position.
  void getLineNumberAndColumnPos(int pos, int *line, int *column) const;
  // Adds the information of the specified position to the stringstream.
  void getPositionString(int pos, std::stringstream *st) const;
};
}  // namespace common
}  // namespace charlie

#endif  // !CHARLIE_COMMON_LOGGIN_COMPONENT_H
