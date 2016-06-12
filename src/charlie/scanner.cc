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
#include <set>
#include <sstream>

#include "scanner.h"

#include "common\comparer_string.h"
#include "program\unresolved_program.h"

#include "vm\instruction.h"

#define ERROR_MESSAGE_MAKE_CODE(message) error_message(message, __FILE__, __LINE__)
#define ERROR_MESSAGE_MAKE_CODE_AND_POS(message) error_message_to_code(message, __FILE__, __LINE__)
#define ERROR_MESSAGE_WITH_POS_MAKE_CODE(message, pos) error_message_to_code(message, pos, __FILE__, __LINE__)


namespace charlie {

  using namespace std;
  using namespace token;
  using namespace program;
  using namespace api;
  using namespace common;

  inline bool isBracketToken(Base* token, Bracket::DirectionEnum direction, Bracket::KindEnum kind) {
    return token->token_type == Base::TokenTypeEnum::Bracket && dynamic_cast<Bracket*>(token)->kind == kind &&
      dynamic_cast<Bracket*>(token)->direction == direction;
  }

  struct TypeDict {
    static map<const char*, VariableDeclaration::TypeEnum, comparer_string> create() {
      map<const char*, VariableDeclaration::TypeEnum, comparer_string> types;
      types["int"] = VariableDeclaration::Int;
      types["long"] = VariableDeclaration::Long;
      types["float"] = VariableDeclaration::Float;
      types["double"] = VariableDeclaration::Double;
      types["bool"] = VariableDeclaration::Boolean;
      types["char"] = VariableDeclaration::Char;
      types["void"] = VariableDeclaration::Void;
      return types;
    }
    static const map<const char*, VariableDeclaration::TypeEnum, comparer_string> Types;
    static bool Contains(const char* name) {
      return TypeDict::Types.count(name) > 0;
    }
    static VariableDeclaration::TypeEnum Get(const char* name) {
      auto it = TypeDict::Types.find(name);
      return it->second;
    }
  };

  struct ControlFlowDict {
    static map<const char*, ControlFlow::KindEnum, comparer_string> create() {
      map<const char*, ControlFlow::KindEnum, comparer_string> types;
      types["while"] = ControlFlow::KindEnum::While;
      types["for"] = ControlFlow::KindEnum::For;
      types["do"] = ControlFlow::KindEnum::Do;
      types["if"] = ControlFlow::KindEnum::If;
      types["else"] = ControlFlow::KindEnum::Else;
      types["continue"] = ControlFlow::KindEnum::Continue;
      types["break"] = ControlFlow::KindEnum::Break;
      types["return"] = ControlFlow::KindEnum::Return;
      types["goto"] = ControlFlow::KindEnum::Goto;
      return types;
    }
    static const map<const char*, ControlFlow::KindEnum, comparer_string> Controls;
    static bool Contains(const char* name) {
      return ControlFlowDict::Controls.count(name) > 0;
    }
    static ControlFlow::KindEnum Get(const char* name) {
      auto it = ControlFlowDict::Controls.find(name);
      return it->second;
    }
  };

  const map<const char*, VariableDeclaration::TypeEnum, comparer_string> TypeDict::Types = TypeDict::create();
  const map<const char*, ControlFlow::KindEnum, comparer_string> ControlFlowDict::Controls = ControlFlowDict::create();


  Scanner::Scanner(program::UnresolvedProgram *program, api::ExternalFunctionManager *external_function_manager) :
    LoggingComponent(), program_(program), external_function_manager_(external_function_manager)
  {

  };

  Scanner::Scanner(program::UnresolvedProgram *program, api::ExternalFunctionManager *external_function_manager, function<void(string const&message)> messageDelegate) :
    LoggingComponent(messageDelegate), program_(program), external_function_manager_(external_function_manager)
  {
  };

  bool Scanner::Scan(string const &code)
  {
    codeInfo_.set(&code);
    program_->Dispose();

    // Search declarations
    WordType wordType;
    string word;

    while (codeInfo_.pos != -1 && codeInfo_.pos < codeInfo_.length)
    {
      getNextWord(word, wordType);
      // A single simikolon does not make sense but it is still valid
      if (wordType == WordType::Semikolon)
        continue;
      if (wordType != WordType::Name)
      {
        stringstream st;
        st << "Unexpected word \"" << word << "\" found!";
        ERROR_MESSAGE_MAKE_CODE_AND_POS(st);
        return false;
      }

      // Looking for Declarations (function & variable) and definitions
      auto typeName = word.c_str();
      if (TypeDict::Contains(typeName)) {
        auto type = TypeDict::Get(typeName);

        getNextWord(word, wordType);
        if (wordType != WordType::Name) {
          stringstream st;
          st << "Unexpected word \"" << word << "\" after type found!";
          ERROR_MESSAGE_MAKE_CODE_AND_POS(st);
          return false;
        }
        auto variableName = word;

        getNextWord(word, wordType);
        if (code[codeInfo_.pos] == '(') {
          ++codeInfo_.pos;
          list<VariableDeclaration> args = list<VariableDeclaration>();
          if (!getFunctionDecArguments(args))
            return false;
          if (codeInfo_.pos == -1)
            return false;
          // is there a function definition following?
          getNextWord(word, wordType);
          if (wordType == WordType::Semikolon) {
            ++codeInfo_.pos;
            program_->function_declarations.push_back(FunctionDeclaration(variableName, type, args, &program_->root));
            continue;
          }
          else if (wordType == WordType::Bracket && code[codeInfo_.pos] == '{') {
            ++codeInfo_.pos;
            auto dec = FunctionDeclaration(variableName, type, args, &program_->root);
            if (!getFunctionDefinition(dec))
              return false;
            dec.has_definition = true;
            program_->function_declarations.push_back(dec);
            if (codeInfo_.pos == -1)
              return false;

          }
          else {
            ERROR_MESSAGE_MAKE_CODE_AND_POS("Unexpected symbol after function declaration");
            return false;
          }

        }
        else if (code[codeInfo_.pos] == ';') {
          ++codeInfo_.pos;
          VariableDeclaration dec = VariableDeclaration(variableName, type);
          program_->root.AddVariableDec(dec);
        }
        else if (word.length() == 1 && word[0] == '=') {
          VariableDeclaration dec = VariableDeclaration(variableName, type);
          program_->root.AddVariableDec(dec);
          --codeInfo_.pos;
          getStatement(program_->root, variableName);
        }
        else {
          stringstream st;
          st << "Unexpected word \"" << word << "\" after variable name";
          ERROR_MESSAGE_MAKE_CODE_AND_POS(st);
          return false;
        }
      }
      else
      {
        stringstream st;
        st << "Unsupported type \"" << word << "\" found!";
        ERROR_MESSAGE_MAKE_CODE_AND_POS(st);
        return false;
      }
    }
    return true;
  }

  bool Scanner::is_beginnging_of_label(char c) {
    if (c <= 'z' &&  c >= 'a')
      return true;
    if (c <= 'Z' &&  c >= 'A')
      return true;
    if (c == '_')
      return true;
    return false;
  }

  bool Scanner::is_operator(char c) {
    return (c == '/' || c == '*' || c == '+' || c == '-' ||
      c == '!' || c == '^' || c == '=' || c == '~' ||
      c == '<' || c == '>' || c == '|' || c == '%');
  }

  bool Scanner::is_bracket(char c) {
    return (c == '(' || c == ')' || c == '[' || c == ']' ||
      c == '{' || c == '}' || c == '<' || c == '>');
  }

  bool Scanner::is_numerical(char c) {
    return c <= '9' && c >= '0';
  }

  bool Scanner::end_of_line_comment() {
    codeInfo_.pos = codeInfo_.code->find("\n", codeInfo_.pos);
    return codeInfo_.pos >= 0;
  }

  bool Scanner::end_of_block_comment() {
    codeInfo_.pos = codeInfo_.code->find("*/", codeInfo_.pos);
    if (codeInfo_.pos < 0)
      return false;
    ++codeInfo_.pos;
    return true;
  }

  void Scanner::getNextWord(std::string &word, WordType &type) {
    int oldStart = codeInfo_.pos;
    int begin = codeInfo_.pos;
    type = WordType::None;
    for (; codeInfo_.pos < codeInfo_.length; ++codeInfo_.pos) {
      char c = codeInfo_.current_char();
      if (c == ' ' || c == '\n' || c == '\t') {
        if (type == WordType::None)
          continue;
        else if (type == WordType::String) {
          if (c == ' ' || c == '\t')
            continue;
          else
          {
            ERROR_MESSAGE_MAKE_CODE_AND_POS("No newline in a string allowed!");
            codeInfo_.pos = -1;
            return;
          }
        }
        else
          break;
      }
      else if (c == ',' || c == ';')
      {
        if (type == WordType::String)
          continue;
        else if (type == WordType::None) {
          type = c == ';' ? WordType::Semikolon : WordType::Comma;
          break;
        }
        else
          break;
      }
      else if (is_beginnging_of_label(c)) {
        if (type == WordType::None)
        {
          begin = codeInfo_.pos;
          type = WordType::Name;
          continue;
        }
        else if (type == WordType::Name) {
          continue;
        }
        else if (type == WordType::Number) {
          if (c == 'f' || c == 'd') {
            break;
          }
          else
          {
            ERROR_MESSAGE_MAKE_CODE_AND_POS("Why is there a letter after a number?");
            codeInfo_.pos = -1;
            return;
          }
        }
        else if (type == WordType::String)
          continue;
        else
          break;
      }
      else if (is_numerical(c))
      {
        if (type == WordType::None)
        {
          begin = codeInfo_.pos;
          type = WordType::Number;
          continue;
        }
        else if (type == WordType::String)
          continue;
        else if (type == WordType::Name)
          continue;
        else if (type == WordType::Number)
          continue;
        else
          break;
      }
      else if (c == '.') {
        if (type == WordType::String)
          continue;
        else if (type == WordType::Name || type == WordType::Number) {
          continue;
        }
        else {
          ERROR_MESSAGE_MAKE_CODE_AND_POS("Could not understand why there is dot?");
          codeInfo_.pos = -1;
          return;
        }
      }
      else if (is_operator(c)) {
        if (c == '/' && codeInfo_.length > codeInfo_.pos + 1) {
          if (codeInfo_.at(codeInfo_.pos + 1) == '/')
          {
            if (!end_of_line_comment())
              return;

            if (type != WordType::None)
              break;
            else
              continue;
          }
          else if (codeInfo_.at(codeInfo_.pos + 1) == '*')
          {
            if (!end_of_block_comment())
            {
              ERROR_MESSAGE_MAKE_CODE_AND_POS("Could not find end of block comment!");
              return;
            }
            if (type != WordType::None)
              break;
            else
              continue;
          }
        }
        if (type == WordType::Operator)
          continue;
        else if (type == WordType::None) {
          begin = codeInfo_.pos;
          type = WordType::Operator;
        }
        else if (type == WordType::String)
          continue;
        else
          break;
      }
      else if (c == '\"') {
        if (type == WordType::String) {
          if (oldStart < codeInfo_.pos && codeInfo_.at(codeInfo_.pos - 1) == '\\')
          {
            continue;
          }
          else
            break;
        }
        else if (type == WordType::None) {
          type = WordType::String;
          begin = codeInfo_.pos;
        }
        else {
          break;
        }
      }
      else if (c == '\'') {
        if (oldStart < codeInfo_.pos && codeInfo_.at(codeInfo_.pos - 1) == '\\' && type == WordType::String)
          continue;
        if (codeInfo_.pos > codeInfo_.length - 2) {
          if (codeInfo_.at(codeInfo_.pos + 1) == '\\')
          {
            if (codeInfo_.pos > codeInfo_.length - 3) {
              if (codeInfo_.at(codeInfo_.pos + 3) == '\'')
              {
                begin = codeInfo_.pos + 1;
                codeInfo_.pos += 3;
              }
              else {
                ERROR_MESSAGE_MAKE_CODE_AND_POS("A String must be embeddend in \" and not in \'");
                codeInfo_.pos = -1;
                return;
              }
            }
            else {
              ERROR_MESSAGE_MAKE_CODE_AND_POS("Error after \'\\");
              codeInfo_.pos = -1;
              return;
            }
          }
          else {
            if (codeInfo_.at(codeInfo_.pos + 2) == '\'')
            {
              begin = codeInfo_.pos + 1;
              codeInfo_.pos += 2;
            }
            else {
              ERROR_MESSAGE_MAKE_CODE_AND_POS("A String must be embeddend in \" and not in \'");
              codeInfo_.pos = -1;
              return;
            }
          }
        }
        else {
          ERROR_MESSAGE_MAKE_CODE_AND_POS("Error after \'");
          codeInfo_.pos = -1;
          return;
        }
      }
      else if (is_bracket(c)) {
        if (type == WordType::None) {
          type = WordType::Bracket;
          break;
        }
        else if (type == WordType::String)
          continue;
        else
          break;
      }
      else {
        if (type == WordType::String)
          continue;
        stringstream st;
        st << "Unkown character found \'" << c << "\'";
        ERROR_MESSAGE_MAKE_CODE_AND_POS(st);
        codeInfo_.pos = -1;
        return;
      }
    }
    switch (type)
    {
    case WordType::None:
      word = "";
      return;
    case WordType::Name:
      word = codeInfo_.code->substr(begin, codeInfo_.pos - begin);
      return;
    case WordType::Number:
      word = codeInfo_.code->substr(begin, codeInfo_.pos - begin);
      return;
    case WordType::Operator:
      word = codeInfo_.code->substr(begin, codeInfo_.pos - begin);
      return;
    case WordType::Bracket:
      return;
    case WordType::Comma:
    case WordType::Semikolon:
      return;
    case WordType::Char:
      word = codeInfo_.code->substr(begin, codeInfo_.pos - begin);
      return;
    case WordType::String:
      word = codeInfo_.code->substr(begin, codeInfo_.pos - begin);
      return;
    default:
      break;
    }
    if (type != WordType::None)
      word = codeInfo_.code->substr(begin, codeInfo_.pos - begin);
    else
      word = "";
  }
  // Call this after opening bracket: i.e "int main ("
  bool Scanner::getFunctionDecArguments(std::list<VariableDeclaration> &args) {
    string word;
    WordType wordtype;
    bool first = true;
    while (codeInfo_.valid())
    {
      getNextWord(word, wordtype);

      if (wordtype == WordType::Bracket && codeInfo_.current_char() == ')')
      {
        ++codeInfo_.pos;
        if (first)
          break;
        else {
          ERROR_MESSAGE_MAKE_CODE_AND_POS("Missing type after comma");
          return false;
        }
      }
      else if (wordtype == WordType::Name) {
        auto typeBuf = word.c_str();
        if (TypeDict::Contains(typeBuf)) {
          auto varType = TypeDict::Get(typeBuf);

          getNextWord(word, wordtype);
          if (wordtype == WordType::Comma)
          {
            ++codeInfo_.pos;
            args.push_back(VariableDeclaration(varType));
            continue;
          }
          else if (wordtype == WordType::Bracket && codeInfo_.current_char() == ')') {
            ++codeInfo_.pos;
            args.push_back(VariableDeclaration(varType));
            break;
          }
          else if (wordtype == WordType::Name) {
            args.push_back(VariableDeclaration(word, varType));

            getNextWord(word, wordtype);
            if (wordtype == WordType::Comma)
            {
              ++codeInfo_.pos;
              continue;
            }
            else if (wordtype == WordType::Bracket && codeInfo_.current_char() == ')') {
              ++codeInfo_.pos;
              break;
            }
            else {
              ERROR_MESSAGE_MAKE_CODE_AND_POS("Unexpected symbols after variable name");
              return false;
            }
          }
          else {
            ERROR_MESSAGE_MAKE_CODE_AND_POS("Unexpected symbols after type");
            return false;
          }
        }
        else {
          stringstream st;
          st << "Unknown word type \"" << typeBuf << "\" !";
          ERROR_MESSAGE_MAKE_CODE_AND_POS(st);
          return false;
        }
      }
      else {
        ERROR_MESSAGE_MAKE_CODE_AND_POS("Unexpected word in argument list.");
        return false;
      }
      first = false;
    }


    return true;
  }

  void Scanner::proceess_controlsequences(std::string &text)
  {
    text.replace(text.begin(), text.end(), "\\\\", "\\");
    text.replace(text.begin(), text.end(), "\\n", "\n");
    text.replace(text.begin(), text.end(), "\\t", "\t");
    text.replace(text.begin(), text.end(), "\\\"", "\"");
    text.replace(text.begin(), text.end(), "\\\'", "\'");
  }

  bool Scanner::getFunctionDefinition(FunctionDeclaration &dec) {
    WordType wordType;
    string word;
    const char* wordBuffer;

    // Are Arguments declared?
    for (auto it = dec.argument_types.begin(); it != dec.argument_types.end(); ++it)
    {
      dec.definition.AddVariableDec(*it);
      if (it->image_type == VariableDeclaration::Int) {
        auto pOp = new Operator(Operator::KindEnum::Pop, CodePostion(codeInfo_.pos));
        pOp->type = it->image_type;
        Statement statement(pOp);
        auto pLa = new Label(it->name, CodePostion(codeInfo_.pos));
        try_get_type_of_variable(pLa, dec.definition);
        statement.arguments.push_back(pLa);
        dec.definition.statements.push_back(statement);
      }
      else {
        ERROR_MESSAGE_MAKE_CODE_AND_POS("Unspported type");
        return false;
      }
    }

    while (codeInfo_.pos < codeInfo_.length && codeInfo_.pos != -1)
    {
      getNextWord(word, wordType);
      // Declarations, statements or loops/ifs?
      if (wordType == WordType::Bracket && codeInfo_.current_char() == '}')
      {
        ++codeInfo_.pos;
        break;
      }
      if (wordType == WordType::Semikolon)
      {
        ++codeInfo_.pos;
        continue;
      }
      else if (wordType == WordType::Operator)
      {
        // TODO: prefix operators i.e. ++i;
        ERROR_MESSAGE_MAKE_CODE_AND_POS("Not implemented exeption!");
        return false;
      }
      else if (wordType != WordType::Name)
      {
        ERROR_MESSAGE_MAKE_CODE_AND_POS("Unexpected symbol in function definition");
        return false;
      }
      wordBuffer = word.c_str();
      if (TypeDict::Contains(wordBuffer))
      {
        auto type = TypeDict::Get(wordBuffer);
        string variableName;
        getNextWord(variableName, wordType);

        if (wordType == WordType::Name)
        {
          dec.definition.AddVariableDec(VariableDeclaration(variableName, type));
          if (!getStatement(dec.definition, variableName))
          {
            return false;
          }
        }
        else if (wordType == WordType::Semikolon)
        {
          ++codeInfo_.pos;
          continue;
        }
        else
        {
          ERROR_MESSAGE_MAKE_CODE_AND_POS("Unexpected symbol in function definition");
          return false;
        }
      }
      else if (ControlFlowDict::Contains(wordBuffer))
      {
        auto control = ControlFlowDict::Get(wordBuffer);
        switch (control)
        {
        case ControlFlow::KindEnum::While:
          break;
        case ControlFlow::KindEnum::For:
          break;
        case ControlFlow::KindEnum::Do:
          break;
        case ControlFlow::KindEnum::If:
          getNextWord(word, wordType);
          if (wordType != WordType::Bracket || codeInfo_.current_char() != '(') {
            ERROR_MESSAGE_MAKE_CODE_AND_POS("Expected opening bracket after word");
            return false;
          }
          else {
            ++codeInfo_.pos;
            Scope expression = Scope(&dec.definition);
            getExpression(expression, true);
            getNextWord(word, wordType);
            if (wordType != WordType::Bracket || codeInfo_.current_char() != '{') {
              Scope block = Scope(&dec.definition);

            }
            else
            {

            }
          }
          break;
        case ControlFlow::KindEnum::Else:
          break;
        case ControlFlow::KindEnum::Break:
          break;
        case ControlFlow::KindEnum::Continue:
          break;
        case ControlFlow::KindEnum::Return:
          if (dec.image_type != VariableDeclaration::Void) {

            if (getExpression(dec.definition))
            {
              dec.definition.statements.push_back(Statement(new ControlFlow(control, CodePostion(codeInfo_.pos))));
              break;
            }
            return false;
          }
          else {
            getNextWord(word, wordType);
            if (wordType == WordType::Semikolon)
            {
              dec.definition.statements.push_back(Statement(new ControlFlow(control, CodePostion(codeInfo_.pos))));
              break;
            }
            else
            {
              ERROR_MESSAGE_MAKE_CODE_AND_POS("This function has returning type of void and nothing else!");
            }
          }
        case ControlFlow::KindEnum::Switch:
          break;
        case ControlFlow::KindEnum::Case:
          break;
        case ControlFlow::KindEnum::Goto:
          break;
        default:
          break;
        }

      }
      else {
        if (!getStatement(dec.definition, word))
        {
          return false;
        }
      }
    }

    return true;
  }

  bool Scanner::getExpression(Scope& prog, bool inBracket) {
    Statement tokens = Statement(0);

    int num = getStatemantTokens(tokens, inBracket);
    if (num > 0)
    {
      Statement statement = 0;
      if (!treeifyStatement(tokens.arguments, prog, statement))
        return false;
      prog.statements.push_back(statement);
    }
    else if (num < 0)
      return false;
    return true;
  }

  bool Scanner::getStatement(Scope& prog, string &word)
  {
    Statement tokens = Statement(0);

    tokens.arguments.push_back(new Label(word, CodePostion(codeInfo_.pos)));

    int num = getStatemantTokens(tokens);
    if (num > 0)
    {
      Statement statement = 0;
      if (!treeifyStatement(tokens.arguments, prog, statement))
        return false;
      prog.statements.push_back(statement);
    }
    else if (num < 0)
      return false;
    return true;
  }

  bool Scanner::treeifyStatement(list<Statement> &linearStatements, program::Scope& scope, Statement& statement)
  {
    if (++(linearStatements.begin()) == linearStatements.end() && linearStatements.begin()->value->finished) {
      statement = *linearStatements.begin();
      return true;
    }

    int maxPriority = 1;
    std::list<Statement>::iterator itMax;

    while (maxPriority > 0)
    {
      maxPriority = 0;
      for (auto it = linearStatements.begin(); it != linearStatements.end(); ++it) {
        int priority = it->priority();
        if (priority > maxPriority) {
          itMax = it;
          maxPriority = priority;
        }
      }
      if (maxPriority == 0)
      {
        if (++(linearStatements.begin()) == linearStatements.end() && linearStatements.begin()->value->finished) {
          statement = *linearStatements.begin();
          return true;
        }
        else {
          ERROR_MESSAGE_WITH_POS_MAKE_CODE("Could not proceed statement", linearStatements.begin()->value->position.character_position);
          return false;
        }
      }

      std::list<Statement>::const_iterator itTemp;
      auto tokenType = itMax->value->token_type;
      switch (tokenType)
      {
      case Base::TokenTypeEnum::Label:
        // Function or variable?
        itTemp = itMax;
        ++itTemp;
        if (itTemp == linearStatements.end() || !isBracketToken(itTemp->value, Bracket::DirectionEnum::Opening, Bracket::KindEnum::Round))
        {
          if (!try_get_type_of_variable(itMax->value, scope))
          {
            stringstream st;
            st << "Not declared \"" << itMax->value->ToString() << "\" variable used";
            ERROR_MESSAGE_WITH_POS_MAKE_CODE(st, itTemp->value->position.character_position);
            return 0;
          }
          itMax->value->finished = true;
        }
        else
        {
          auto functionNode = *itMax;
          dynamic_cast<Label*>(functionNode.value)->kind = Label::KindEnum::Function;
          getBracket(linearStatements, itTemp, functionNode.arguments);
          if (++(functionNode.arguments.begin()) == functionNode.arguments.end())
          {
            functionNode.value->finished = true;
            statement = functionNode;
            return true;
          }
          else
          {
            if (!treeifyStatement(functionNode.arguments, scope, statement))
              return false;
            functionNode.value->finished = true;
            itMax->arguments.push_back(statement);
            itMax->value->finished = true;
          }
        }
        break;
      case Base::TokenTypeEnum::Operator:
        if (itMax->value->token_chidren_position == Base::TokenChidrenPosEnum::LeftAndRight) {
          std::list<Statement>::const_iterator prev = itMax;
          --prev;
          std::list<Statement>::const_iterator post = itMax;
          ++post;

          if (prev == linearStatements.end())
          {
            ERROR_MESSAGE_MAKE_CODE_AND_POS("Missing symbol on the left side of a operator");
            return false;
          }
          if (post == linearStatements.end())
          {
            ERROR_MESSAGE_MAKE_CODE_AND_POS("Missing symbol on the right side of a operator");
            return false;
          }
          if (prev->value->token_type == Base::TokenTypeEnum::Label && dynamic_cast<Label*>(prev->value)->kind != Label::KindEnum::Function && !try_get_type_of_variable(prev->value, scope))
            return false;
          if (post->value->token_type == Base::TokenTypeEnum::Label && dynamic_cast<Label*>(post->value)->kind != Label::KindEnum::Function && !try_get_type_of_variable(post->value, scope))
            return false;

          if (prev->value->finished && prev->value->type == VariableDeclaration::Int || prev->value->finished && prev->value->type == VariableDeclaration::Length) {
            if (post->value->finished && post->value->type == VariableDeclaration::Int) {

              itMax->value->finished = true;
              itMax->value->type = VariableDeclaration::Int;
              itMax->arguments.push_back(*prev);
              linearStatements.erase(prev);
              itMax->arguments.push_back(*post);
              linearStatements.erase(post);
            }
            else {
              ERROR_MESSAGE_MAKE_CODE_AND_POS("Right symbol should be an int!");
              return false;
            }
          }
          else
          {
            ERROR_MESSAGE_MAKE_CODE_AND_POS("Unspecified error");
            return false;
          }
        }
        break;
      case Base::TokenTypeEnum::ConstantInt:
        break;
      default:
        break;
      }
    }


    return true;
  }

  int Scanner::getStatemantTokens(Statement& linearStatements, bool inBracket) {
    string  word;
    WordType wordType;
    int i;
    int count = 0;

    int bracketStateRound = 0;
    int bracketStateSquare = 0;
    int bracketStateCurly = 0;

    do {
      getNextWord(word, wordType);
      ++count;
      switch (wordType)
      {
      case WordType::Bracket:
        switch (codeInfo_.current_char())
        {
        case '(':
          linearStatements.arguments.push_back(new Bracket(Bracket::KindEnum::Round, Bracket::DirectionEnum::Opening, CodePostion(codeInfo_.pos)));
          ++bracketStateRound;
          break;
        case ')':
          if (bracketStateRound <= 0)
          {
            if (inBracket) {
              ++codeInfo_.pos;
              return count - 1;
            }
            ERROR_MESSAGE_MAKE_CODE_AND_POS("There is nothing to close with a round bracket");
            return -1;
          }
          linearStatements.arguments.push_back(new Bracket(Bracket::KindEnum::Round, Bracket::DirectionEnum::Closing, CodePostion(codeInfo_.pos)));
          --bracketStateRound;
          break;
        case '[':
          linearStatements.arguments.push_back(new Bracket(Bracket::KindEnum::Square, Bracket::DirectionEnum::Opening, CodePostion(codeInfo_.pos)));
          ++bracketStateSquare;
          break;
        case ']':
          if (bracketStateSquare <= 0)
          {
            ERROR_MESSAGE_MAKE_CODE_AND_POS("There is nothing to close with a square bracket");
            return -1;
          }
          linearStatements.arguments.push_back(new Bracket(Bracket::KindEnum::Square, Bracket::DirectionEnum::Closing, CodePostion(codeInfo_.pos)));
          --bracketStateSquare;
          break;
        case '{':
          linearStatements.arguments.push_back(new Bracket(Bracket::KindEnum::Curly, Bracket::DirectionEnum::Opening, CodePostion(codeInfo_.pos)));
          ++bracketStateCurly;
          break;
        case '}':
          if (bracketStateCurly <= 0)
          {
            ERROR_MESSAGE_MAKE_CODE_AND_POS("There is nothing to close with a curly bracket");
            return -1;
          }
          linearStatements.arguments.push_back(new Bracket(Bracket::KindEnum::Curly, Bracket::DirectionEnum::Closing, CodePostion(codeInfo_.pos)));
          --bracketStateCurly;
          break;
        case '<':
          linearStatements.arguments.push_back(new Bracket(Bracket::KindEnum::Triangle, Bracket::DirectionEnum::Opening, CodePostion(codeInfo_.pos)));
          break;
        case '>':
          linearStatements.arguments.push_back(new Bracket(Bracket::KindEnum::Triangle, Bracket::DirectionEnum::Closing, CodePostion(codeInfo_.pos)));
          break;
        default:
          break;
        }
        ++codeInfo_.pos;
        break;
      case WordType::Char:
        linearStatements.arguments.push_back(new Constant(Constant::KindEnum::Char, new char(codeInfo_.current_char()), CodePostion(codeInfo_.pos)));
        break;
      case WordType::Comma:
        linearStatements.arguments.push_back(new Comma(CodePostion(codeInfo_.pos)));
        ++codeInfo_.pos;
        break;
      case WordType::Name:
        linearStatements.arguments.push_back(new Label(word, CodePostion(codeInfo_.pos)));
        break;
      case WordType::Number:
        i = atoi(word.c_str());
        linearStatements.arguments.push_back(new ConstantInt(i, CodePostion(codeInfo_.pos)));
        break;
      case WordType::Operator:
        if (word.length() == 1) {
          switch (word[0])
          {
          case '+':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::Add, CodePostion(codeInfo_.pos)));
            break;
          case '-':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::Substract, CodePostion(codeInfo_.pos)));
            break;
          case '*':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::Multipply, CodePostion(codeInfo_.pos)));
            break;
          case '/':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::Divide, CodePostion(codeInfo_.pos)));
            break;
          case '%':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::Modulo, CodePostion(codeInfo_.pos)));
            break;
          case '=':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::Copy, CodePostion(codeInfo_.pos)));
            break;
          case '>':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::Greater, CodePostion(codeInfo_.pos)));
            break;
          case '<':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::Less, CodePostion(codeInfo_.pos)));
            break;
          case '|':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::BitOr, CodePostion(codeInfo_.pos)));
            break;
          case '&':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::BitAnd, CodePostion(codeInfo_.pos)));
            break;
          case '^':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::BitXor, CodePostion(codeInfo_.pos)));
            break;
          default:
            ERROR_MESSAGE_MAKE_CODE_AND_POS("Unexpected operator type");
            return -1;
          }
        }
        else if (word.length() == 2 && word[1] == '=') {
          switch (word[0])
          {
          case '=':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::Equal, CodePostion(codeInfo_.pos)));
            break;
          case '!':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::NotEqual, CodePostion(codeInfo_.pos)));
            break;
          case '>':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::GreaterEqual, CodePostion(codeInfo_.pos)));
            break;
          case '<':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::LessEqual, CodePostion(codeInfo_.pos)));
            break;
          case '+':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::AddTo, CodePostion(codeInfo_.pos)));
            break;
          case '-':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::SubstractTo, CodePostion(codeInfo_.pos)));
            break;
          case '*':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::MultiplyTo, CodePostion(codeInfo_.pos)));
            break;
          case '/':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::DivideTo, CodePostion(codeInfo_.pos)));
            break;
          case '%':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::ModuloTo, CodePostion(codeInfo_.pos)));
            break;
          case '&':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::AndTo, CodePostion(codeInfo_.pos)));
            break;
          case '|':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::OrTo, CodePostion(codeInfo_.pos)));
            break;
          case '^':
            linearStatements.arguments.push_back(new Operator(Operator::KindEnum::XorTo, CodePostion(codeInfo_.pos)));
            break;
          default:
            ERROR_MESSAGE_MAKE_CODE_AND_POS("Unexpected operator type");
            return -1;
          }
        }
        else if (word.length() == 2 && word[0] == '&' && word[1] == '&')
          linearStatements.arguments.push_back(new Operator(Operator::KindEnum::LogicAnd, CodePostion(codeInfo_.pos)));
        else if (word.length() == 2 && word[0] == '|' && word[1] == '|')
          linearStatements.arguments.push_back(new Operator(Operator::KindEnum::LogicOr, CodePostion(codeInfo_.pos)));
        break;
      case WordType::Semikolon:
        ++codeInfo_.pos;
        break;
      case WordType::String:
        proceess_controlsequences(word);
        linearStatements.arguments.push_back(new Constant(Constant::KindEnum::String, new string(word), CodePostion(codeInfo_.pos)));
        break;
      default:
        ERROR_MESSAGE_MAKE_CODE_AND_POS("Unexpected word type");
        return -1;
      }


    } while (wordType != WordType::Semikolon);
    if (bracketStateRound != 0) {
      ERROR_MESSAGE_MAKE_CODE_AND_POS("There is still an open round bracket");
    }
    if (bracketStateSquare != 0) {
      ERROR_MESSAGE_MAKE_CODE_AND_POS("There is still an open square bracket");
    }
    if (bracketStateCurly != 0) {
      ERROR_MESSAGE_MAKE_CODE_AND_POS("There is still an open curly bracket");
    }
    return count - 1;
  }


  // Call this after opening bracket
  bool Scanner::getBracket(list<Statement>& linearStatements, list<Statement>::const_iterator& itOpening, std::list<program::Statement>& outList)
  {
    auto it = itOpening;
    list<Statement>::const_iterator itClosing = linearStatements.end();

    int openBrackets = 1;

    delete it->value;

    for (++it; it != linearStatements.end(); ++it) {
      if (isBracketToken(it->value, Bracket::DirectionEnum::Opening, Bracket::KindEnum::Round))
        ++openBrackets;
      else if (isBracketToken(it->value, Bracket::DirectionEnum::Closing, Bracket::KindEnum::Round)) {
        --openBrackets;
        if (openBrackets == 0) {
          delete it->value;
          itClosing = it;
          ++itClosing;
          break;
        }
      }
      outList.push_back(*it);
    }

    linearStatements.erase(itOpening, itClosing);

    return true;
  }

  bool Scanner::try_get_type_of_variable(Base *token, program::Scope& scope) {
    if (token->token_type == Base::TokenTypeEnum::Label)
    {
      if (dynamic_cast<Label*>(token)->kind == Label::KindEnum::Function)
      {
        //if(token->type == VariableDeclaration::Length)
        //{
        //	stringstream st;
        //	st << "Unknown Function found: \"" << dynamic_cast<Label*>(token)->label_string << "\"!";
        //	logging(st.str());
        //	return false;
        //}
      }
      else
      {
        if (token->type == VariableDeclaration::Length)
        {
          auto dec = VariableDeclaration(dynamic_cast<Label*>(token)->label_string, VariableDeclaration::Length);
          auto info = scope.GetVariableInfo(dec);

          if (info.offset == 0) {
            stringstream st;
            st << "Unknown Variable found: \"" << dynamic_cast<Label*>(token)->label_string << "\"!";
            ERROR_MESSAGE_MAKE_CODE_AND_POS(st);
            return false;
          }
          token->type = info.type;
          dynamic_cast<Label*>(token)->register_address = info.offset;
          dynamic_cast<Label*>(token)->kind = Label::KindEnum::Variable;
        }
      }
    }
    return true;
  }
}

#undef ERROR_MESSAGE_MAKE_CODE
#undef ERROR_MESSAGE_MAKE_CODE_AND_POS
#undef ERROR_MESSAGE_WITH_POS_MAKE_CODE