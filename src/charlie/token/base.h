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

// See: http://www.quut.com/c/ANSI-C-grammar-y.html

#ifndef CHARLIE_TOKEN_BASE_H
#define CHARLIE_TOKEN_BASE_H

#include <string>
#include <functional>

#include "../program/variable_declaration.h"

namespace charlie {
namespace token {
// Stores the caret position.
struct CodePostion {
  CodePostion(int character_position);
  int character_position;
};

// Base class of all tokens
class Base {
 public:
  // Token type indicates which class the corresponding inherited instance is.
  enum class TokenTypeEnum {
    Bracket,          // (, ), {, }, ...
    Constant,         // "Hello", 12, ...
    ConstantInt,
    Operator,         // +, -, +=, ...
    TypeDeclarer,     // int, bool, ...
    Label,
    ControlFlow,      // for, while
    List,
    Comma
  };
  // Indicates where children or arguments are corresponding to this token.
  enum class TokenChidrenPosEnum {
    None = 0,
    Left = 1,
    Right = 2,
    LeftAndRight = 3,
    LeftOrRight = 8,    // E.g. "++i", or "i++"
  };
  // Creates an object.
  //    tokentype:  Should be set by the inherited class constructor.
  //    position:   Position where the token appears in the source or header code.
  //    priorty:    (optional) the priority of this token.
  //    finished:   (optional) indicates wheter this token is finished parsed or not.
  //    type:       (optional) image type of this token.
  Base(TokenTypeEnum tokentype, CodePostion const& position, int priorty = 0, bool finished = false,
    program::VariableDeclaration::TypeEnum type = program::VariableDeclaration::Length);
  // Returns the bytecode of this token, if possible.
  // If not possible it returns -1;
  virtual int ByteCode() const = 0;
  // Returns a string that represents the current object.
  virtual std::string ToString() const = 0;

  // Should be set by the inherited class constructor.
  const TokenTypeEnum token_type;
  // label: 10,  bracket: 9, namesep (::; .): 9,  (de)ref: 8; 
  // in-/de-crease: 7 mul/div: 6, add/sub: 5,
  // comparer: 4, logic ops: 3, copy: 2 others: 1
  int priority;
  // Indicates wheter this token is finished parsed or not.
  bool finished;
  // Position of the children or arguments of the current token.
  TokenChidrenPosEnum token_chidren_position;
  // Caret position where the token appears in the source or header code.
  CodePostion position;
  // The image type of this token. E.g. int, float, ...
  program::VariableDeclaration::TypeEnum type;
};

// Represents all brackets as a token
class Bracket : public Base {
 public:
  // Enum of bracket kinds
  enum class KindEnum {
    Round,    // ()
    Curly,    // {}
    Square,   // []
    Triangle  // <>
  };
  // Enum of possible bracket directions.
  enum class DirectionEnum {
    Closing,
    Opening
  };
  // Creates an object.
  //    kind:       Kind of bracket
  //    direction:  Direction of the bracket
  //    position:   Caret position in the code of this bracket.
  Bracket(KindEnum kind, DirectionEnum direction, CodePostion const& position);
  // Returns a string that represents the current object.
  virtual std::string ToString() const;
  // Returns the bytecode of this token, if possible.
  // If not possible it returns -1;
  virtual int ByteCode() const;
  // Kind of this bracket
  KindEnum kind;
  // Direction of this bracket
  DirectionEnum direction;
};

// Represents a comma as a token
class Comma : public Base {
 public:
  // Creates an object.
  // Needs the caret position where this token appears in the code.
  Comma(CodePostion const& position);
  // Returns a string that represents the current object.
  virtual std::string ToString() const;
  // Returns the bytecode of this token, if possible.
  // If not possible it returns -1;
  virtual int ByteCode() const;
};
// Represents a list as a token.
class List : public Base {
 public:
  // Creates an object.
  // Needs the caret position where this token appears in the code.
  List(CodePostion const& position);
  // Returns a string that represents the current object.
  virtual std::string ToString() const;
  // Returns the bytecode of this token, if possible.
  // If not possible it returns -1;
  virtual int ByteCode() const;
};

// Represents a constant as a token
// and stores its value.
// DEPRECATED!
class Constant : public Base {
 public:
  // Type of constant
  enum class KindEnum {
    String,     // use std::string
    Char,       // char
    Decimal,    // float
    Boolean     // bool
  };
  // Creates an object.
  //    kind:     Kind/Type of this constant
  //    pointer:  The pointer to the value which should be stored.
  //    position: The caret position where this token appears in the code.
  Constant(KindEnum kind, void* pointer, CodePostion const& position);
  // Deletes the stored value.
  // Destrcutor must be virtual because the base class needs also an desctructor.
  virtual ~Constant();
  // Returns a string that represents the current object.
  virtual std::string ToString() const;
  // Returns the bytecode of this token, if possible.
  // If not possible it returns -1;
  virtual int ByteCode() const;
  // Kind of this constant
  KindEnum kind;
  // Pointer to the value which should be stored in this token.
  void* pointer;
};

// Represents a constant Integer
// and stores its value.
class ConstantInt : public Base {
 public:
  // Creates an object.
  //    value:    The integer value which should be stored.
  //    position: The caret position where this token appears in the code.
  ConstantInt(int value, CodePostion const& position);
  // Returns a string that represents the current object.
  virtual std::string ToString() const;
  // Returns the bytecode of this token, if possible.
  // If not possible it returns -1;
  virtual int ByteCode() const;
  // The integer value which should be stored.
  int value;
};

// Represents a operator as a token
class Operator : public Base {
 public:
  // Kind enum of operators
  enum class KindEnum {   // TODO(lochbrunner): Not complete!
    Add,            // +
    Substract,      // -    // Could be also uniary
    Multipply,      // *    // Or dereference
    Divide,         // /
    Modulo,         // %
    Copy,           // =
    Equal,          // ==
    NotEqual,       // !=
    Greater,        // >    // Or template bracket
    GreaterEqual,   //>=
    Less,           // <    // Or template bracket
    LessEqual,      // <=
    LogicAnd,       // &&
    LogicOr,        // ||
    BitAnd,         // &    // Or reference
    BitOr,          // |
    BitXor,         // ^
    AddTo,          // +=
    SubstractTo,    // -=
    MultiplyTo,     // *=
    DivideTo,       // /=
    ModuloTo,       // /=
    AndTo,          // &=
    OrTo,           // |=,
    XorTo,          // ^=,
    Increase,       // ++
    Decrease,       // --
    Pop
  };
  // Creates an object.
  //    kind:     Kind of this operator
  //    position: The caret position where this token appears in the code.
  Operator(KindEnum kind, CodePostion const& position);
  // Returns a string that represents the current object.
  virtual std::string ToString() const;
  // Returns the bytecode of this token, if possible.
  // If not possible it returns -1;
  virtual int ByteCode() const;
  // Kind of this operator
  KindEnum kind;
  // Indicates if the left variable should be assigned (e.g. "=", "++", "+=")
  bool assigner;
};

// Represents a control flow word as a token
class ControlFlow : public Base {
 public:
  // Kind enum of control flows
  enum class KindEnum {
    While,
    For,
    Do,
    If,
    Else,
    Break,
    Continue,
    Return,
    Switch,
    Case,
    Goto
  };
  // Creates an object.
  //    kind:     Kind of this control flow
  //    position: The caret position where this token appears in the code.
  ControlFlow(KindEnum kind, CodePostion const& position);
  // Returns a string that represents the current object.
  virtual std::string ToString() const;
  // Returns the bytecode of this token, if possible.
  // If not possible it returns -1;
  virtual int ByteCode() const;
  // Kind of this control flow
  KindEnum kind;
};

// Represents a declarer as a token.
// E.g. "int", "float", ...
class Declarer : public Base {
 public:
  // Creates an object.
  //    kind:     Type of the following declaration
  //    position: The caret position where this token appears in the code.
  Declarer(program::VariableDeclaration::TypeEnum kind, CodePostion const& position);
  // Returns a string that represents the current object.
  virtual std::string ToString() const;
  // Returns the bytecode of this token, if possible.
  // If not possible it returns -1;
  virtual int ByteCode() const;
  // Type of the following declaration
  program::VariableDeclaration::TypeEnum kind;
};

// Represents a label as a token
class Label : public Base {
 public:
  // Usage enum of the label
  enum class KindEnum {
    Function,
    Variable,
    Unknown
  };
  // Creates an object.
  // Creates an object.
  //    labelString:  Text of this label.
  //    position:     The caret position where this token appears in the code.
  Label(std::string const& labelString, CodePostion const& position);
  // Returns a string that represents the current object.
  virtual std::string ToString() const;
  // Returns the bytecode of this token, if possible.
  // If not possible it returns -1;
  virtual int ByteCode() const;
  // Text of this label.
  std::string label_string;
  // Specifies whether this label is used for a variable or a function name.
  KindEnum kind;
  // A delegate which can be used to get the register address
  // at the end of the compiling process.
  std::function<int()> register_address;
};

}  // namespace token
}  // namespace charlie

#endif  // !CHARLIE_TOKEN_BASE_H
