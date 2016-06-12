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

#include "scope.h"

namespace charlie {
  namespace program {
    using namespace std;

    Scope::Scope(Scope* parent) :
      statements(), variable_declarations_(), num_variable_declarations(0), parent_(parent)
    {
    }
    Scope::VariableInfo Scope::GetVariableInfo(VariableDeclaration & dec) const
    {
      auto it = variable_declarations_.find(dec);
      if (it == variable_declarations_.end())
      {
        if (parent_ == 0)
          return VariableInfo(0, VariableDeclaration::TypeEnum::Length);
        return parent_->GetVariableInfo(dec);
      }
      int pos = it->second;
      auto par = parent_;

      return VariableInfo([=]() {
        if (par == 0)
          return pos;
        return pos + par->ParentOffset() + par->num_variable_declarations;
      }, it->first.image_type);
    }
    int Scope::AddVariableDec(VariableDeclaration& dec)
    {
      variable_declarations_.insert(make_pair(dec, num_variable_declarations++));
      return num_variable_declarations;
    }

    int Scope::ParentOffset() const
    {
      if (parent_ == 0)
        return 0;
      return parent_->ParentOffset() + parent_->num_variable_declarations;
    }

    void Scope::Dispose()
    {
      for (auto it = statements.begin(); it != statements.end(); ++it)
        it->Dispose();
    }

    Scope::VariableInfo::VariableInfo(std::function<int()> offset, VariableDeclaration::TypeEnum type) : offset(offset), type(type)
    {
    }
  }
}