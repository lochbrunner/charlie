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

//#define logging_1(message)					logOut(message, __FILE__, __LINE__)
//#define logging_2(message, codePosition)	logOut(message, __FILE__, __LINE__, codePosition)
#define logging_1(message)					logOut(message, __FILE__, __LINE__)
#define logging_2(message, codePosition)	logOut(message, codePosition, __FILE__, __LINE__)

#define GET_3TH_ARG(arg1, arg2, arg3, ...) arg3
#define LOGGIN_MACRO_CHOOSER(...) \
    GET_3TH_ARG(__VA_ARGS__, \
                logging_2, logging_1, )

#define logging(...) LOGGIN_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#include <string>
#include <sstream>
#include <functional>

#include "..\token\base.h"

namespace charlie {
namespace common {

	class LogginComponent {
	public:
		LogginComponent();
		LogginComponent(std::function<void(std::string const &message)> messageDelegate);

	protected:
		std::function<void(std::string const &message)> _messageDelegate;
		void logOut(std::stringstream const &message);
		void logOut(std::stringstream const &message, const char* codefileName, int lineNumber);
		void logOut(std::string const &message);
		void logOut(std::string const &message, const char* codefileName, int lineNumber);

		void logOut(std::stringstream const &message, token::CodePostion& codePosition);
		void logOut(std::stringstream const &message, token::CodePostion& codePosition, const char* codefileName, int lineNumber);
		void logOut(std::string const &message, token::CodePostion& codePosition);
		void logOut(std::string const &message, token::CodePostion& codePosition, const char* codefileName, int lineNumber);

		const std::string *_pCurrentCode;

	private:
		void getLineNumberAndColumnPos(int pos, int& line, int& column);
		void getPositionString(int pos, std::stringstream& st);
	};
}
}

#endif // !CHARLIE_COMMON_LOGGIN_COMPONENT_H