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
#include "program\UnresolvedProgram.h"

#include "vm\instruction.h"

namespace charlie {

	using namespace std;
	using namespace token;
	using namespace program;
	using namespace api;
	using namespace common;

	inline bool isBracketToken(token::Base* token, Bracket::DirectionEnum direction, Bracket::KindEnum kind) {
		return token->TokenType == Base::TokenTypeEnum::Bracket && dynamic_cast<Bracket*>(token)->Kind == kind &&
			dynamic_cast<token::Bracket*>(token)->Direction == direction;
	}

	struct TypeDict {
		static map<const char*, VariableDec::TypeEnum, comparer_string> create() {
			map<const char*, VariableDec::TypeEnum, comparer_string> types;
			types["int"] = VariableDec::Int;
			types["long"] = VariableDec::Long;
			types["float"] = VariableDec::Float;
			types["double"] = VariableDec::Double;
			types["bool"] = VariableDec::Boolean;
			types["char"] = VariableDec::Char;
			types["void"] = VariableDec::Void;
			return types;
		}
		static const map<const char*, VariableDec::TypeEnum, comparer_string> Types;
		static bool Contains(const char* name) {
			return TypeDict::Types.count(name) > 0;
		}
		static VariableDec::TypeEnum Get(const char* name) {
			auto it = TypeDict::Types.find(name);
			return it->second;
		}
	};

	struct ControlFlowDict {
		static map<const char*, ControlFlow::KindEnum, comparer_string> create() {
			map<const char*, ControlFlow::KindEnum, comparer_string> types;
			types["while"] = ControlFlow::While;
			types["for"] = ControlFlow::For;
			types["do"] = ControlFlow::Do;
			types["if"] = ControlFlow::If;
			types["else"] = ControlFlow::Else;
			types["continue"] = ControlFlow::Continue;
			types["break"] = ControlFlow::Break;
			types["return"] = ControlFlow::Return;
			types["goto"] = ControlFlow::Goto;
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

	const map<const char*, VariableDec::TypeEnum, comparer_string> TypeDict::Types = TypeDict::create();
	const map<const char*, ControlFlow::KindEnum, comparer_string> ControlFlowDict::Controls = ControlFlowDict::create();


	Scanner::Scanner(program::UnresolvedProgram *pProgram, api::ExternalFunctionManager *pExternalFunctionManager) :
		LogginComponent(), _pProgram(pProgram), _pExternalFunctionManager(pExternalFunctionManager)
	{
		
	};

	Scanner::Scanner(program::UnresolvedProgram *pProgram, api::ExternalFunctionManager *pExternalFunctionManager, function<void(string const&message)> messageDelegate) :
		LogginComponent(messageDelegate), _pProgram(pProgram), _pExternalFunctionManager(pExternalFunctionManager)
	{
	};

	bool Scanner::Scan(string const &code) 
	{
		_pCurrentCode = &code;
		_pProgram->Clear();

		// Search declarations
		int length = static_cast<int>(code.length());
		int pos = 0;
		WordType wordType;
		string word;

		while (pos != -1 && pos < length)
		{
			getNextWord(code, length, pos, word, wordType);
			// A single simikolon does not make sense but it is still valid
			if (wordType == WordType::Semikolon)
				continue;
			if (wordType != WordType::Name)
			{
				stringstream st;
				st << "Unexpected word \"" << word << "\" found!";
				logging(st, CodePostion(pos));
				return false;
			}

			// Looking for Declarations (function & variable) and definitions
			auto typeName = word.c_str();
			if (TypeDict::Contains(typeName)) {
				auto type = TypeDict::Get(typeName);

				getNextWord(code, length, pos, word, wordType);
				if (wordType != WordType::Name) {
					stringstream st;
					st << "Unexpected word \"" << word << "\" after type found!";
					logging(st, CodePostion(pos));
					return false;
				}
				auto variableName = word;

				getNextWord(code, length, pos, word, wordType);
				if (code[pos] == '('){
					++pos;
					list<VariableDec> args = list<VariableDec>();
					pos = getFunctionDecArguments(code, pos, length, args);
					if (pos == -1)
						return false;
					// is there a function definition following?
					getNextWord(code, length, pos, word, wordType);
					if (wordType == WordType::Semikolon) {
						++pos;
						_pProgram->FunctionDecs.push_back(FunctionDec(variableName, type, args));
						continue;
					}
					else if(wordType == WordType::Bracket && code[pos] == '{') {
						++pos;
						auto dec = FunctionDec(variableName, type, args);
						// TODO: copy arguments into variables
						pos = getFunctionDefinition(code, length, pos, dec.Definition);
						dec.HasDefinition = true;
						_pProgram->FunctionDecs.push_back(dec);
						if (pos == -1)
							return false;

					}
					else {
						logging("Unexpected symbol after function declaration", CodePostion(pos));
						return false;
					}

				}
				else if (code[pos] == ';') {
					++pos;
					VariableDec dec = VariableDec(variableName, type);
					_pProgram->VariableDecs.push_back(dec);
				}
				else {
					stringstream st;
					st << "Unexpected word \"" << word << "\" after variable name";
					logging(st, CodePostion(pos));
					return false;
				}
			}
			else
			{
				stringstream st;
				st << "Unsupported type \"" << word << "\" found!";
				logging(st, CodePostion(pos));
				return false;
			}
		}
		return true;
	}

	bool Scanner::isLabelBeginning(char c) {
		if (c <= 'z' &&  c >= 'a')
			return true;
		if (c <= 'Z' &&  c >= 'A')
			return true;
		if (c == '_')
			return true;
		return false;
	}

	bool Scanner::isOperator(char c) {
		return (c == '/' || c == '*' || c == '+' || c == '-' ||
			c == '!' || c == '^' || c == '=' || c == '~' ||
			c == '<' || c == '>' || c == '|' || c == '%');
	}

	bool Scanner::isBracket(char c) {
		return (c == '(' || c == ')' || c == '[' || c == ']' ||
			c == '{' || c == '}' || c == '<' || c == '>');
	}

	bool Scanner::isNumerical(char c) {
		return c <= '9' && c >= '0';
	}

	int Scanner::endOfLineComments(std::string const &code, int length, int pos) {
		int end = code.find("\n", pos);
		return end;
	}

	int Scanner::endOfBlockComments(std::string const &code, int length, int pos) {
		int end = code.find("*/", pos);
		return end+1;
	}

	void Scanner::getNextWord(std::string const &code, int length, int &pos, std::string &word, WordType &type) {
		int oldStart = pos;
		int begin = pos;
		type = WordType::None;
		for (; pos < length; ++pos) {
			char c = code[pos];
			if (c == ' ' || c == '\n' || c == '\t') {
				if (type == WordType::None)
					continue;
				else if (type == WordType::String) {
					if (c == ' ' || c == '\t')
						continue;
					else
					{
						logging("No newline in a string allowed!");
						pos = -1;
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
			else if (isLabelBeginning(c)) {
				if (type == WordType::None)
				{
					begin = pos;
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
						logging("Why is there a letter after a number?");
						pos = -1;
						return;
					}
				}
				else if (type == WordType::String)
					continue;
				else
					break;
			}
			else if (isNumerical(c))
			{
				if (type == WordType::None)
				{
					begin = pos;
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
					logging("Could not understand why there is dot?");
					pos = -1;
					return;
				}
			}
			else if (isOperator(c)) {
				if (c == '/' && length > pos + 1) {
					if (code[pos + 1] == '/')
					{
						pos = endOfLineComments(code, length, pos);
						if (pos == -1)
						{
							return;
						}
						if(type != WordType::None)
							break;
						else
							continue;
					}
					else if (code[pos + 1] == '*')
					{
						pos = endOfBlockComments(code, length, pos);
						if (pos == -1)
						{
							logging("Could not find end of block comment!");
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
					begin = pos;
					type = WordType::Operator;
				}
				else if (type == WordType::String)
					continue;
				else
					break;
			}
			else if (c == '\"') {
				if (type == WordType::String) {
					if (oldStart < pos && code[pos - 1] == '\\')
					{
						continue;
					}
					else
						break;
				}
				else if (type == WordType::None) {
					type = WordType::String;
					begin = pos;
				}
				else {
					break;
				}
			}
			else if (c == '\'') {
				if (oldStart < pos && code[pos - 1] == '\\' && type == WordType::String)
					continue;
				if (pos > length - 2) {
					if (code[pos + 1] == '\\')
					{
						if (pos > length - 3) {
							if (code[pos + 3] == '\'')
							{
								begin = pos + 1;
								pos += 3;
							}
							else {
								logging("A String must be embeddend in \" and not in \'");
								pos = -1;
								return;
							}
						}
						else {
							logging("Error after \'\\");
							pos = -1;
							return;
						}
					}
					else {
						if (code[pos + 2] == '\'')
						{
							begin = pos + 1;
							pos += 2;
						}
						else {
							logging("A String must be embeddend in \" and not in \'");
							pos = -1;
							return;
						}
					}
				}
				else {
					logging("Error after \'");
					pos = -1;
					return;
				}
			}
			else if (isBracket(c)) {
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
				logging(st.str());
				pos = -1;
				return;
			}
		}
		switch (type)
		{
		case WordType::None:
			word = "";
			return;
		case WordType::Name:
			word = code.substr(begin, pos - begin);
			return;
		case WordType::Number:
			word = code.substr(begin, pos - begin);
			return;
		case WordType::Operator:
			word = code.substr(begin, pos - begin);
			return;
		case WordType::Bracket:
			return;
		case WordType::Comma:
		case WordType::Semikolon:
			return;
		case WordType::Char:
			word = code.substr(begin, pos-begin);
			return;
		case WordType::String:
			word = code.substr(begin, pos - begin);
			return;
		default:
			break;
		}
		if (type != WordType::None)
			word = code.substr(begin, pos - begin);
		else
			word = "";
	}
	// Call this after opening bracket: i.e "int main ("
	int Scanner::getFunctionDecArguments(string const &code, int pos, int length, std::list<VariableDec> &args) {
		string word;
		WordType wordtype;
		bool first = true;
		while (pos<length)
		{
			getNextWord(code, length, pos, word, wordtype);
			
			if(wordtype == WordType::Bracket && code[pos] == ')')
			{
				++pos;
				if(first)
					break;
				else {
					logging("Missing Type after comma");
					return -1;
				}
			}
			else if (wordtype == WordType::Name) {
				auto typeBuf = word.c_str();
				if (TypeDict::Contains(typeBuf)) {
					auto varType = TypeDict::Get(typeBuf);

					getNextWord(code, length, pos, word, wordtype);
					if (wordtype == WordType::Comma)
					{
						++pos;
						args.push_back(VariableDec(varType));
						continue;
					}
					else if (wordtype == WordType::Bracket && code[pos] == ')') {
						++pos;
						args.push_back(VariableDec(varType));
						break;
					}
					else if (wordtype == WordType::Name) {
						args.push_back(VariableDec(word, varType));
						
						getNextWord(code, length, pos, word, wordtype);
						if (wordtype == WordType::Comma)
						{
							++pos;
							continue;
						}
						else if (wordtype == WordType::Bracket && code[pos] == ')') {
							++pos;
							break;
						}
						else {
							logging("Unexpected symbols after variable name");
							return -1;
						}
					}
					else {
						logging("Unexpected symbols after type");
						return -1;
					}
				}
				else {
					stringstream st;
					st << "Unknown word type \"" << typeBuf << "\" !";
					logging(st.str());
					return -1;
				}
			}
			else {
				logging("Unexpected word in argument list.");
				return -1;
			}
			first = false;
		}
		

		return pos;
	}

	void Scanner::proceessControlSequences(std::string &text) 
	{
		text.replace(text.begin(), text.end(), "\\\\", "\\");
		text.replace(text.begin(), text.end(), "\\n", "\n");
		text.replace(text.begin(), text.end(), "\\t", "\t");
		text.replace(text.begin(), text.end(), "\\\"", "\"");
		text.replace(text.begin(), text.end(), "\\\'", "\'");
	}

	int Scanner::getFunctionDefinition(string const &code, int length, int pos, FunctionDefinition &definition) {
		WordType wordType;
		string word;
		const char* wordBuffer;

		while (pos < length && pos != -1)
		{
			getNextWord(code, length, pos, word, wordType);
			// Declarations, statements or loops/ifs?
			if (wordType == WordType::Bracket && code[pos] == '}')
			{
				++pos;
				break;
			}
			if (wordType == WordType::Semikolon)
			{
				++pos;
				continue;
			}
			else if (wordType == WordType::Operator) 
			{
				// TODO: prefix operators i.e. ++i;
			}
			else if (wordType != WordType::Name) 
			{
				logging("Unexpected symbol in function definition", CodePostion(pos));
				return -1;
			}
			wordBuffer = word.c_str();
			if (TypeDict::Contains(wordBuffer))
			{
				auto type = TypeDict::Get(wordBuffer);
				string variableName;
				getNextWord(code, length, pos, variableName, wordType);
				
				if (wordType == WordType::Name)
				{
					definition.main.AddVariableDec(VariableDec(variableName, type));
					if (!getStatement(code, length, pos, definition.main, variableName))
					{
						return -1;
					}
				}
				else if (wordType == WordType::Semikolon) 
				{
					++pos;
					continue;
				}
				else
				{
					logging("Unexpected symbol in function definition", CodePostion(pos));
					return -1;
				}
			}
			else if (ControlFlowDict::Contains(wordBuffer))
			{
				return -1;
			}
			else {
				if (!getStatement(code, length, pos, definition.main, word))
				{
					return -1;
				}
			}
		}

		return pos;
	}

	bool Scanner::getStatement(string const &code, int length, int &pos, Scope& prog, string &word)
	{
		Statement tokens = Statement(0);

		tokens.Arguments.push_back(new Label(word, CodePostion(pos)));

		int num = getStatemantTokens(code, length, pos, tokens);
		if (num > 0)
		{
			auto statement = treeifyStatement(tokens.Arguments, prog);
			prog.Statements.push_back(statement);
		}
		else if (num < 0)
			return false;
		return true;
	}

	Statement Scanner::treeifyStatement(list<Statement> &linearStatements, program::Scope& scope)
	{
		if (++(linearStatements.begin()) == linearStatements.end() && linearStatements.begin()->Value->Finished) {
			return *linearStatements.begin();
		}

		int maxPriority = 1;
		std::list<Statement>::iterator itMax;

		while (maxPriority>0)
		{
			maxPriority = 0;
			for (auto it = linearStatements.begin(); it != linearStatements.end(); ++it) {
				int priority = it->Priority();
				if (priority > maxPriority) {
					itMax = it;
					maxPriority = priority;
				}
			}
			if (maxPriority == 0)
			{
				if (++(linearStatements.begin()) == linearStatements.end() && linearStatements.begin()->Value->Finished) {
					return *linearStatements.begin();
				}
				else {
					logging("Could not proceed statement", linearStatements.begin()->Value->Position);
					return 0;
				}
			}

			std::list<Statement>::const_iterator itTemp;
			auto tokenType = itMax->Value->TokenType;
			switch (tokenType)
			{
			case Base::TokenTypeEnum::Label:
				// Function or variable?
				itTemp = itMax;
				++itTemp;
				if (itTemp == linearStatements.end() || !isBracketToken(itTemp->Value, Bracket::Opening, Bracket::Round))
				{
					if(!tryGettingTypeOfVariable(itMax->Value, scope))
					{
						stringstream st;
						st << "Not declared \"" << itMax->Value->ToString() << "\" variable used";
						logging(st, itTemp->Value->Position);
						return 0;
					}
					itMax->Value->Finished = true;
				}
				else
				{
					auto functionNode = *itMax;
					dynamic_cast<Label*>(functionNode.Value)->Kind = Label::Function;

					getBracket(linearStatements, itTemp, functionNode.Arguments);
					if (++(functionNode.Arguments.begin()) == functionNode.Arguments.end())
					{
						functionNode.Value->Finished = true;
						return Statement(functionNode);
					}
					else
					{
						auto arg = treeifyStatement(functionNode.Arguments, scope);
						functionNode.Value->Finished = true;
						return Statement(functionNode);
					}
				}
				break;
			case Base::TokenTypeEnum::Operator:
				if (itMax->Value->TokenChidrenPos == Base::TokenChidrenPosEnum::LeftAndRight) {
					std::list<Statement>::const_iterator prev = itMax;
					--prev;
					std::list<Statement>::const_iterator post = itMax;
					++post;

					if (prev == linearStatements.end())
					{
						logging("Missing symbol on the left side of a operator");
						return 0;
					}
					if (post == linearStatements.end())
					{
						logging("Missing symbol on the right side of a operator");
						return 0;
					}

					if (!tryGettingTypeOfVariable(prev->Value, scope))
						return 0;
					if(!tryGettingTypeOfVariable(post->Value, scope))
						return 0;

					if (prev->Value->Finished && prev->Value->Type == VariableDec::Int) {
						if (post->Value->Finished && post->Value->Type == VariableDec::Int) {

							itMax->Value->Finished = true;
							itMax->Value->Type = VariableDec::Int;
							itMax->Arguments.push_back(*prev);
							linearStatements.erase(prev);
							itMax->Arguments.push_back(*post);
							linearStatements.erase(post);
						}
						else {
							logging("Right symbol should be an int!");
							return 0;
						}
					}
					else
					{
						logging("Unspecified error");
						return 0;
					}
				}
				break;
			case Base::TokenTypeEnum::ConstantInt:
				break;
			default:
				break;
			}
		}


		return 0;
	}

	int Scanner::getStatemantTokens(string const& code, int length, int& pos, Statement& linearStatements) {
		string  word;
		WordType wordType;
		int i;
		int count = 0;

		int bracketStateRound = 0;
		int bracketStateSquare = 0;
		int bracketStateCurly = 0;

		do {
			getNextWord(code, length, pos, word, wordType);
			++count;
			switch (wordType)
			{
			case WordType::Bracket:
				switch (code[pos])
				{
				case '(':
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Round, Bracket::Opening, CodePostion(pos)));
					++bracketStateRound;
					break;
				case ')':
					if (bracketStateRound <= 0)
					{
						logging("There is nothing to close with a round bracket");
						return -1;
					}
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Round, Bracket::Closing, CodePostion(pos)));
					--bracketStateRound;
					break;
				case '[':
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Square, Bracket::Opening, CodePostion(pos)));
					++bracketStateSquare;
					break;
				case ']':
					if (bracketStateSquare <= 0)
					{
						logging("There is nothing to close with a square bracket");
						return -1;
					}
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Square, Bracket::Closing, CodePostion(pos)));
					--bracketStateSquare;
					break;
				case '{':
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Curly, Bracket::Opening, CodePostion(pos)));
					++bracketStateCurly;
					break;
				case '}':
					if (bracketStateCurly <= 0)
					{
						logging("There is nothing to close with a curly bracket");
						return -1;
					}
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Curly, Bracket::Closing, CodePostion(pos)));
					--bracketStateCurly;
					break;
				case '<':
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Triangle, Bracket::Opening, CodePostion(pos)));
					break;
				case '>':
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Triangle, Bracket::Closing, CodePostion(pos)));
					break;
				default:
					break;
				}
				++pos;
				break;
			case WordType::Char:
				linearStatements.Arguments.push_back(new Constant(Constant::Char, new char(code[pos]), CodePostion(pos)));
				break;
			case WordType::Comma:
				linearStatements.Arguments.push_back(new token::Comma(CodePostion(pos)));
				++pos;
				break;
			case WordType::Name:
				linearStatements.Arguments.push_back(new Label(word, CodePostion(pos)));
				break;
			case WordType::Number:
				i = atoi(word.c_str());
				linearStatements.Arguments.push_back(new ConstantInt(i, CodePostion(pos)));
				break;
			case WordType::Operator:
				if (word.length() == 1) {
					switch (word[0])
					{
					case '+':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Add, CodePostion(pos)));
						break;
					case '-':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Substract, CodePostion(pos)));
						break;
					case '*':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Multipply, CodePostion(pos)));
						break;
					case '/':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Divide, CodePostion(pos)));
						break;
					case '=':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Copy, CodePostion(pos)));
						break;
					case '>':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Greater, CodePostion(pos)));
						break;
					case '<':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Less, CodePostion(pos)));
						break;
					case '|':
						linearStatements.Arguments.push_back(new token::Operator(Operator::BitOr, CodePostion(pos)));
						break;
					case '&':
						linearStatements.Arguments.push_back(new token::Operator(Operator::BitAnd, CodePostion(pos)));
						break;
					case '^':
						linearStatements.Arguments.push_back(new token::Operator(Operator::BitXor, CodePostion(pos)));
						break;
					default:
						logging("Unexpected operator type");
						return -1;
					}
				}
				else if (word.length() == 2 && word[1] == '=') {
					switch (word[0])
					{
					case '=':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Equal, CodePostion(pos)));
						break;
					case '!':
						linearStatements.Arguments.push_back(new token::Operator(Operator::NotEqual, CodePostion(pos)));
						break;
					case '>':
						linearStatements.Arguments.push_back(new token::Operator(Operator::GreaterEqual, CodePostion(pos)));
						break;
					case '<':
						linearStatements.Arguments.push_back(new token::Operator(Operator::LessEqual, CodePostion(pos)));
						break;
					case '+':
						linearStatements.Arguments.push_back(new token::Operator(Operator::AddTo, CodePostion(pos)));
						break;
					case '-':
						linearStatements.Arguments.push_back(new token::Operator(Operator::SubstractTo, CodePostion(pos)));
						break;
					case '*':
						linearStatements.Arguments.push_back(new token::Operator(Operator::MultiplyTo, CodePostion(pos)));
						break;
					case '/':
						linearStatements.Arguments.push_back(new token::Operator(Operator::DivideTo, CodePostion(pos)));
						break;
					case '&':
						linearStatements.Arguments.push_back(new token::Operator(Operator::AndTo, CodePostion(pos)));
						break;
					case '|':
						linearStatements.Arguments.push_back(new token::Operator(Operator::OrTo, CodePostion(pos)));
						break;
					case '^':
						linearStatements.Arguments.push_back(new token::Operator(Operator::XorTo, CodePostion(pos)));
						break;
					default:
						logging("Unexpected operator type");
						return -1;
					}
				}
				else if (word.length() == 2 && word[0] == '&' && word[1] == '&')
					linearStatements.Arguments.push_back(new token::Operator(Operator::LogicAnd, CodePostion(pos)));
				else if(word.length() == 2 && word[0]=='|' && word[1] == '|')
					linearStatements.Arguments.push_back(new token::Operator(Operator::LogicOr, CodePostion(pos)));
				break;
			case WordType::Semikolon:
				++pos;
				break;
			case WordType::String:
				proceessControlSequences(word);
				linearStatements.Arguments.push_back(new token::Constant(Constant::String, new string(word), CodePostion(pos)));
				break;
			default:
				logging("Unexpected word type");
				return -1;
			}


		} while (wordType != WordType::Semikolon);
		if (bracketStateRound != 0) {
			logging("There is still an open round bracket");
		}
		if (bracketStateSquare != 0) {
			logging("There is still an open square bracket");
		}
		if (bracketStateCurly != 0) {
			logging("There is still an open curly bracket");
		}
		return count-1;
	}


	// Call this after opening bracket
	bool Scanner::getBracket(list<Statement>& linearStatements, list<Statement>::const_iterator& itOpening, std::list<program::Statement>& outList)
	{
		auto it = itOpening;
		list<Statement>::const_iterator itClosing = linearStatements.end();
		
		int openBrackets = 1;

		delete it->Value;

		for (++it; it != linearStatements.end(); ++it) {
			if (isBracketToken(it->Value, Bracket::Opening, Bracket::Round))
				++openBrackets;
			else if (isBracketToken(it->Value, Bracket::Closing, Bracket::Round)) {
				--openBrackets;
				if (openBrackets == 0) {
					delete it->Value;
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

	bool Scanner::tryGettingTypeOfVariable(token::Base *token, program::Scope& scope) {
		if (token->Type == VariableDec::Length && token->TokenType == Base::TokenTypeEnum::Label)
		{
			auto it = scope.VariableDecs.find(VariableDec(dynamic_cast<Label*>(token)->LabelString, VariableDec::Length));
			if (it == scope.VariableDecs.end()) {
				stringstream st;
				st << "Unknown Variable found: \"" << dynamic_cast<Label*>(token)->LabelString << "\"!";
				logging(st.str());
				return false;
			}
			token->Type = it->first.ImageType;
			dynamic_cast<Label*>(token)->Address = it->second;
			dynamic_cast<Label*>(token)->Kind = Label::Variable;
		}
		return true;
	}
}

