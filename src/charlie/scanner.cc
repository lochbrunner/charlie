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
				log(st.str());
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
					log(st.str());
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
						log("Unexpected symbol after function declaration");
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
					log(st.str());
					return false;
				}
			}
			else
			{
				stringstream st;
				st << "Unsupported type \"" << word << "\" found!";
				log(st.str());
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
						log("No newline in a string allowed!");
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
						log("Why is there a letter after a number?");
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
					log("Could not understand why there is dot?");
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
							log("Could not find end of block comment!");
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
								log("A String must be embeddend in \" and not in \'");
								pos = -1;
								return;
							}
						}
						else {
							log("Error after \'\\");
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
							log("A String must be embeddend in \" and not in \'");
							pos = -1;
							return;
						}
					}
				}
				else {
					log("Error after \'");
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
				log(st.str());
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
					log("Missing Type after comma");
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
							log("Unexpected symbols after variable name");
							return -1;
						}
					}
					else {
						log("Unexpected symbols after type");
						return -1;
					}
				}
				else {
					stringstream st;
					st << "Unknown word type \"" << typeBuf << "\" !";
					log(st.str());
					return -1;
				}
			}
			else {
				log("Unexpected word in argument list.");
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
				log("Unexpected symbol in function definition");
				return -1;
			}
			wordBuffer = word.c_str();
			if (TypeDict::Contains(wordBuffer))
			{
				auto type = TypeDict::Get(wordBuffer);
				return -1;

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

		tokens.Arguments.push_back(new Label(word));

		getStatemantTokens(code, length, pos, tokens);

		auto statement = treeifyStatement(tokens.Arguments);

		prog.Statements.push_back(statement);

		return true;
	}

	Statement Scanner::treeifyStatement(list<Statement> &linearStatements)
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
					log("Could not proceed statement", __FILE__, __LINE__);
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
				if (isBracketToken(itTemp->Value, Bracket::Opening, Bracket::Round))
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
						auto arg = treeifyStatement(functionNode.Arguments);
						//functionNode.Arguments.push_back(arg);
						functionNode.Value->Finished = true;
						return Statement(functionNode);
					}
				}
				else 
				{
					dynamic_cast<Label*>(itMax->Value)->Kind = Label::Variable;
					//TODO
				}
				break;
			case Base::TokenTypeEnum::Operator:
				if (itMax->Value->TokenChidrenPos == Base::TokenChidrenPosEnum::LeftAndRight) {
					std::list<Statement>::const_iterator prev = itMax;
					--prev;
					std::list<Statement>::const_iterator post = itMax;
					++post;
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
							log("Right symbol should be an int!", __FILE__, __LINE__);
						}
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

	void Scanner::getStatemantTokens(string const& code, int length, int& pos, Statement& linearStatements) {
		string  word;
		WordType wordType;
		int i;

		int bracketStateRound = 0;
		int bracketStateSquare = 0;
		int bracketStateCurly = 0;

		do {
			getNextWord(code, length, pos, word, wordType);
			switch (wordType)
			{
			case WordType::Bracket:
				switch (code[pos])
				{
				case '(':
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Round, Bracket::Opening));
					++bracketStateRound;
					break;
				case ')':
					if (bracketStateRound <= 0)
					{
						log("There is nothing to close with a round bracket", __FILE__, __LINE__);
						return;
					}
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Round, Bracket::Closing));
					--bracketStateRound;
					break;
				case '[':
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Square, Bracket::Opening));
					++bracketStateSquare;
					break;
				case ']':
					if (bracketStateSquare <= 0)
					{
						log("There is nothing to close with a square bracket", __FILE__, __LINE__);
						return;
					}
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Square, Bracket::Closing));
					--bracketStateSquare;
					break;
				case '{':
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Curly, Bracket::Opening));
					++bracketStateCurly;
					break;
				case '}':
					if (bracketStateCurly <= 0)
					{
						log("There is nothing to close with a curly bracket", __FILE__, __LINE__);
						return;
					}
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Curly, Bracket::Closing));
					--bracketStateCurly;
					break;
				case '<':
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Triangle, Bracket::Opening));
					break;
				case '>':
					linearStatements.Arguments.push_back(new token::Bracket(Bracket::Triangle, Bracket::Closing));
					break;
				default:
					break;
				}
				++pos;
				break;
			case WordType::Char:
				linearStatements.Arguments.push_back(new Constant(Constant::Char, new char(code[pos])));
				break;
			case WordType::Comma:
				linearStatements.Arguments.push_back(new token::Comma());
				++pos;
				break;
			case WordType::Name:
				linearStatements.Arguments.push_back(new Label(word));
				break;
			case WordType::Number:
				i = atoi(word.c_str());
				linearStatements.Arguments.push_back(new ConstantInt(i));
				break;
			case WordType::Operator:
				if (word.length() == 1) {
					switch (word[0])
					{
					case '+':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Add));
						break;
					case '-':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Substract));
						break;
					case '*':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Multipply));
						break;
					case '/':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Divide));
						break;
					case '=':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Copy));
						break;
					case '>':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Greater));
						break;
					case '<':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Less));
						break;
					case '|':
						linearStatements.Arguments.push_back(new token::Operator(Operator::BitOr));
						break;
					case '&':
						linearStatements.Arguments.push_back(new token::Operator(Operator::BitAnd));
						break;
					case '^':
						linearStatements.Arguments.push_back(new token::Operator(Operator::BitXor));
						break;
					default:
						log("Unexpected operator type", __FILE__, __LINE__);
						return;
					}
				}
				else if (word.length() == 2 && word[1] == '=') {
					switch (word[0])
					{
					case '=':
						linearStatements.Arguments.push_back(new token::Operator(Operator::Equal));
						break;
					case '!':
						linearStatements.Arguments.push_back(new token::Operator(Operator::NotEqual));
						break;
					case '>':
						linearStatements.Arguments.push_back(new token::Operator(Operator::GreaterEqual));
						break;
					case '<':
						linearStatements.Arguments.push_back(new token::Operator(Operator::LessEqual));
						break;
					case '+':
						linearStatements.Arguments.push_back(new token::Operator(Operator::AddTo));
						break;
					case '-':
						linearStatements.Arguments.push_back(new token::Operator(Operator::SubstractTo));
						break;
					case '*':
						linearStatements.Arguments.push_back(new token::Operator(Operator::MultiplyTo));
						break;
					case '/':
						linearStatements.Arguments.push_back(new token::Operator(Operator::DivideTo));
						break;
					case '&':
						linearStatements.Arguments.push_back(new token::Operator(Operator::AndTo));
						break;
					case '|':
						linearStatements.Arguments.push_back(new token::Operator(Operator::OrTo));
						break;
					case '^':
						linearStatements.Arguments.push_back(new token::Operator(Operator::XorTo));
						break;
					default:
						log("Unexpected operator type", __FILE__, __LINE__);
						return;
					}
				}
				else if (word.length() == 2 && word[0] == '&' && word[1] == '&')
					linearStatements.Arguments.push_back(new token::Operator(Operator::LogicAnd));
				else if(word.length() == 2 && word[0]=='|' && word[1] == '|')
					linearStatements.Arguments.push_back(new token::Operator(Operator::LogicOr));
				break;
			case WordType::Semikolon:
				++pos;
				break;
			case WordType::String:
				proceessControlSequences(word);
				linearStatements.Arguments.push_back(new token::Constant(Constant::String, new string(word)));
				break;
			default:
				log("Unexpected word type", __FILE__, __LINE__);
				return;
			}


		} while (wordType != WordType::Semikolon);
		if (bracketStateRound != 0) {
			log("There is still an open round bracket", __FILE__, __LINE__);
		}
		if (bracketStateSquare != 0) {
			log("There is still an open square bracket", __FILE__, __LINE__);
		}
		if (bracketStateCurly != 0) {
			log("There is still an open curly bracket", __FILE__, __LINE__);
		}
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
}

