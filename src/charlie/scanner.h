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

#ifndef CHARLIE_SCANNER_H
#define CHARLIE_SCANNER_H


#include <list>
#include <string>

#include "common/logging_component.h"
#include "common/definitions.h"
#include "common/exportDefs.h"

#include "token/base.h"

#include "program/variable_declaration.h"
#include "program/function_declaration.h"
#include "program/scope.h"
#include "program/unresolved_program.h"
#include "program/statement.h"

#include "api/external_function_manager.h"

namespace charlie {
// Scans C-code and creates the corresponding syntax tree.
class Scanner : public common::LoggingComponent {
 public:
  // The used categories of each word in the C-code.
  enum class WordType {
    None,
    Name,
    Number,
    Operator,
    Comma,
    Semikolon,
    String,
    Char,
    Bracket
  };
  // Creates an object, with the specified program and external function manager.
  // Optional message delegate. See common::LoggingComponent
  xprt Scanner(program::UnresolvedProgram *program, api::ExternalFunctionManager *external_function_manager);
  xprt Scanner(program::UnresolvedProgram *program, api::ExternalFunctionManager *external_function_manager,
    std::function<void(std::string const &message)> messageDelegate);
  // Scans C-code and creates the corresponding syntax tree into program_.
  // Returns true if succeeded.
  xprt bool Scan(std::string const &code);

 private:
  // Scans the function argument types.
  // Returns true if succeeded.
  bool getFunctionDecArguments(std::list<program::VariableDeclaration> *args);
  // Scans the function definition and adds the statements and function-scope variables to the
  // corresponding function declarations.
  // Returns true if succeeded.
  bool getFunctionDefinition(program::FunctionDeclaration *dec);
  // Scans the beginning block and adds the statements and function-scope variables to the
  // corresponding function declarations.
  // Returns true if succeeded.
  bool getBlock(program::FunctionDeclaration const& dec, program::Scope *scope);
  // Scans the beginning statement and adds it into the current scope.
  // Returns true if succeeded.
  bool getStatement(std::string const& word, program::Scope *prog);
  // Scans the beginning expression and adds it into the current scope.
  // Returns true if succeeded.
  bool getExpression(program::Scope *prog, bool inBracket = false);
  // Copies all tokens into the corresponding statement list untill either a semikolon or closing round bracket is found.
  // Depending on the parameter "inBracket"
  // Returns the number of tokens. Returns -1 iff an error occured.
  int getStatemantTokens(program::Statement *linearStatements, bool inBracket = false);
  // Tries to make syntax tree out of the "linearStatements" satement or expression and saves it into "statement"
  // Returns true if succeeded.
  bool treeifyStatement(program::Scope const& scope,
    std::list<program::Statement> *linearStatements, program::Statement *statement);
  // Moves the content from the beginning bracket at "itOpening" of "linearStatements" into "outList".
  // Returns true if succeeded.
  bool getBracket(std::list<program::Statement>::const_iterator const& itOpening,
    std::list<program::Statement> *linearStatements, std::list<program::Statement> *outList);
  // Search for image type and index of the specified token.
  //  "scope": the current scope where this variable was found.
  // Returns true if succeeded.
  inline bool try_get_type_of_variable(program::Scope const& scope, token::Base *token);
  // Checks wheter the specified character could be the beginning of a labe. E.g. '_' in "_foo124".
  inline bool is_beginnging_of_label(char c);
  // Checks wheter the specified character could be part of an operator.
  inline bool is_operator(char c);
  // Checks wheter the specified character could be part of a number.
  inline bool is_numerical(char c);
  // Checks wheter the specified character could be a bracket.
  inline bool is_bracket(char c);
  // Processes the controll sequences of the speciefied string. E.g. "\\" -> "\"
  void proceess_controlsequences(std::string *text);

  // Updates the word reference only if the result is longer than 1 char. If not, use code.current_char() instead.
  FRIEND_TEST(ScannerTest, getNextWord);
  xprt void getNextWord(std::string *word, WordType *type);
  // Sets the code caret to the end of the beginning line comment
  // Returns true if succeeded.
  bool end_of_line_comment();
  // Sets the code caret to the end of the beginning block comment
  // Returns true if succeeded.
  bool end_of_block_comment();
  // Used to check function signatures.
  api::ExternalFunctionManager *external_function_manager_;
  // The current program.
  program::UnresolvedProgram *program_;
};
}  // namespace charlie

#endif  // !CHARLIE_SCANNER_H
