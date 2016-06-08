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

#include "LoggingComponent.h"

#include <sstream>
#include <iomanip>

#include <assert.h>

namespace charlie {
	namespace common {
		using namespace std;

		char significantChar(const char* codefileName) {
			auto path = string(codefileName);
			string base_filename = path.substr(path.find_last_of("/\\") + 1);
			return static_cast<char>(toupper(path[path.find_last_of("/\\") + 1]));
		}


		LoggingComponent::LoggingComponent() : _messageDelegate(0), _pCurrentCode(0)
		{
		}

		LoggingComponent::LoggingComponent(function<void(string const&message)> messageDelegate) : _messageDelegate(messageDelegate), _pCurrentCode(0)
		{
		}

		void LoggingComponent::logOut(string const &message)
		{
			if (_messageDelegate != NULL)
				_messageDelegate(message);
		}
		void LoggingComponent::logOut(std::string const &message, const char* codefileName, int lineNumber) {
			if (_messageDelegate != NULL) {
				stringstream st;
				st << significantChar(codefileName) << setfill('0') << setw(4) << lineNumber << ": " << message;
				_messageDelegate(st.str());
			}
		}
		void LoggingComponent::logOut(stringstream const &message)
		{
			if (_messageDelegate != NULL)
				_messageDelegate(message.str());
		}
		void LoggingComponent::logOut(std::stringstream const &message, const char* codefileName, int lineNumber) {
			if (_messageDelegate != NULL) {
				stringstream st;
				st << significantChar(codefileName) << setfill('0') << setw(4) << lineNumber << ": " << message.str();
				_messageDelegate(st.str());
			}
		}
		void LoggingComponent::logOut(string const &message, token::CodePostion& codePosition)
		{
			if (_messageDelegate != NULL)
			{

				stringstream st;
				st << message;
				getPositionString(codePosition.CharacterNumber, st);
				_messageDelegate(st.str());
			}
		}
		void LoggingComponent::logOut(std::string const &message, token::CodePostion& codePosition, const char* codefileName, int lineNumber) {
			if (_messageDelegate != NULL) {
				stringstream st;
				st << significantChar(codefileName) << setfill('0') << setw(4) << lineNumber << ": " << message;
				getPositionString(codePosition.CharacterNumber, st);
				_messageDelegate(st.str());
			}
		}
		void LoggingComponent::logOut(stringstream const &message, token::CodePostion& codePosition)
		{
			if (_messageDelegate != NULL)
			{
				stringstream st;
				st << message.str();
				getPositionString(codePosition.CharacterNumber, st);
				_messageDelegate(st.str());
			}
		}
		void LoggingComponent::logOut(std::stringstream const &message, token::CodePostion& codePosition, const char* codefileName, int lineNumber) {
			if (_messageDelegate != NULL) {
				stringstream st;
				st << significantChar(codefileName) << setfill('0') << setw(4) << lineNumber << ": " << message.str();
				getPositionString(codePosition.CharacterNumber, st);
				_messageDelegate(st.str());
			}
		}

		void LoggingComponent::getPositionString(int pos, std::stringstream& st) {
			if (pos < 0)
			{
				st << " at unspecifed position";
				return;
			}
			int line;
			int column;
			getLineNumberAndColumnPos(pos, line, column);

			st << " at line: " << line << " columns: " << column;
		}

		void LoggingComponent::getLineNumberAndColumnPos(int pos, int& line, int& column)
		{
			if (_pCurrentCode != 0) {
				column = 0;
				line = 1;
				if (_pCurrentCode->length() <= static_cast<size_t>(pos))
					return;
				for (int i = 0; i <= pos; ++i) {
					if (_pCurrentCode->at(i) == '\n')
					{
						++line;
						column = 0;
					}
					else if (_pCurrentCode->at(i) == '\r')
						continue;
					++column;
				}
			}
		}
	}
}