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

#ifndef  CHARLIE_COMPILER_H
#define CHARLIE_COMPILER_H

#include <map>
#include <string>
#include <functional>
#include "common\exportDefs.h"
#include "common\LoggingComponent.h"

#include "program\functionDec.h"
#include "program\unresolvedProgram.h"
#include "program\statement.h"

#include "api\externalFunctionManager.h"


namespace charlie {

	class Compiler : public common::LoggingComponent {
	public:
		xprt Compiler();
		xprt Compiler(std::function<void(std::string const &message)> messageDelegate);
		/// Compiles the speciefed file
		xprt bool Build(std::string const &filename);
		xprt bool SaveProgram(std::string const &filename, bool binary = true);

		xprt int Run();
		xprt int Run(int argn, char** argv);

		api::ExternalFunctionManager ExternalFunctionManager;

	private:
		bool compile();
		bool enroleStatement(program::Statement& statement, int& count);
		
		program::UnresolvedProgram _program;
	};

}

#endif // ! CHARLIE_COMPILER_H