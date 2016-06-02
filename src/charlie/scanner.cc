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


#include "scanner.h"
#include <sstream>


namespace charlie {

	using namespace std;
	using namespace token;

	struct cmp_str
	{
		bool operator()(char const *a, char const *b)
		{
			return std::strcmp(a, b) < 0;
		}
	};

	struct TypeDict {
		static map<const char*, Declarer::KindEnum, cmp_str> create() {
			map<const char*, Declarer::KindEnum, cmp_str> types;
			types["int"] = Declarer::Int;
			types["long"] = Declarer::Long;
			types["float"] = Declarer::Float;
			types["double"] = Declarer::Double;
			types["bool"] = Declarer::Boolean;
			types["char"] = Declarer::Char;
			return types;
		}
		static const map<const char*, Declarer::KindEnum, cmp_str> Types;
		static bool Contains(const char* name) {
			return TypeDict::Types.count(name) > 0;
		}
		static Declarer::KindEnum Get(const char* name) {
			auto it = TypeDict::Types.find(name);
			return it->second;
		}
	};

	const map<const char*, Declarer::KindEnum, cmp_str> TypeDict::Types = TypeDict::create();


	enum TokenKind
	{
		BracketOpeningRound,
		BracketClosingRound
	};

	Scanner::Scanner() : LogginComponent() 
	{
		
	};

	Scanner::Scanner(function<void(string const&message)> messageDelegate) : LogginComponent(messageDelegate)
	{
		_funcDecs = std::list<token::FunctionDec>();
		_variableDecs = std::list<token::VariableDec>();
	};

	bool Scanner::Scan(string const &code) 
	{
		_funcDecs.clear();
		_variableDecs.clear();

		// Search declarations
		int lenght = static_cast<int>(code.length());
		int pos = 0;
		WordType wordType;
		string word;

		while (pos != -1 && pos < lenght)
		{
			getNextWord(code, lenght, pos, word, wordType);
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

				getNextWord(code, lenght, pos, word, wordType);
				if (wordType != WordType::Name) {
					stringstream st;
					st << "Unexpected word \"" << word << "\" after type found!";
					log(st.str());
					return false;
				}
				auto variableName = word;

				getNextWord(code, lenght, pos, word, wordType);
				if (code[pos] == '('){
					++pos;
					list<VariableDec> args = list<VariableDec>();
					pos = getFunctionDecArguments(code, pos, lenght, args);
					if (pos == -1)
						return false;
					// is there a function definition following?
					getNextWord(code, lenght, pos, word, wordType);
					if (wordType == WordType::Semikolon) {
						++pos;
						_funcDecs.push_back(FunctionDec(variableName, type, args));
						continue;
					}
					else if(wordType == WordType::Bracket && code[pos] == '{') {
						++pos;
						auto dec = FunctionDec(variableName, type, args);
						FunctionDefinition def = FunctionDefinition();
						pos = getFunctionDefinition(code, lenght, pos, def);
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
					_variableDecs.push_back(dec);
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

	void Scanner::PrintState() {
	//	for (list<Base*>::const_iterator it = _tokens.begin(); it != _tokens.end(); ++it)
	//	{
	//		if ((*it) != 0) {
	//			(*it)->ToString();
	//		}
	//	}
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

	bool Scanner::isAlphaNumeric(char c) {
		if (c <= 'z' &&  c >= 'a')
			return true;
		if (c <= 'Z' &&  c >= 'A')
			return true;
		if (c <= '9' &&  c >= '0')
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

	int Scanner::endOfLineComments(std::string const &code, int lenght, int pos) {
		int end = code.find("\n", pos);
		return end;
	}

	int Scanner::endOfBlockComments(std::string const &code, int lenght, int pos) {
		int end = code.find("*/", pos);
		return end+1;
	}

	void Scanner::getNextWord(std::string const &code, int lenght, int &pos, std::string &word, WordType &type) {
		int oldStart = pos;
		int begin = pos;
		type = WordType::None;
		for (; pos < lenght; ++pos) {
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
				if (c == '/' && lenght > pos + 1) {
					if (code[pos + 1] == '/')
					{
						pos = endOfLineComments(code, lenght, pos);
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
						pos = endOfBlockComments(code, lenght, pos);
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
				if (pos > lenght - 2) {
					if (code[pos + 1] == '\\')
					{
						if (pos > lenght - 3) {
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
	int Scanner::getFunctionDecArguments(string const &code, int pos, int lenght, std::list<VariableDec> &args) {
		string word;
		WordType wordtype;
		bool first = true;
		while (pos<lenght)
		{
			getNextWord(code, lenght, pos, word, wordtype);
			
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

					getNextWord(code, lenght, pos, word, wordtype);
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
						
						getNextWord(code, lenght, pos, word, wordtype);
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

	void Scanner::proceessControlSequences(std::string &text) {
		text.replace(text.begin(), text.end(), "\\\\", "\\");
		text.replace(text.begin(), text.end(), "\\n", "\n");
		text.replace(text.begin(), text.end(), "\\t", "\t");
		text.replace(text.begin(), text.end(), "\\\"", "\"");
		text.replace(text.begin(), text.end(), "\\\'", "\'");
	}

	int Scanner::getFunctionDefinition(string const &code, int lenght, int pos, FunctionDefinition &definition) {
		// Declarations, statements or loops/ifs?
		WordType wordType;
		string word;

		while (pos < lenght && pos != -1)
		{
			getNextWord(code, lenght, pos, word, wordType);

			return -1;
		}


		return pos;
	}
}

