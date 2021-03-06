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

#include "runtime.h"

#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

#include <boost/asio.hpp>

#include <google/protobuf/util/json_util.h>

#include "debug.pb.h"
#include "instruction.h"

using boost::asio::ip::tcp;

namespace charlie::vm {

inline void sendLength(tcp::socket& socket, int i) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(4) << i;
  boost::system::error_code ignored_error;
  std::string st = ss.str();
  boost::asio::write(socket, boost::asio::buffer(st), ignored_error);
}

inline void send_string(tcp::socket& socket, const std::string& content) {
  boost::system::error_code ignored_error;
  std::stringstream header;
  header << std::setfill('0') << std::setw(4) << content.length();
  boost::asio::write(socket, boost::asio::buffer(header.str()), ignored_error);
  boost::asio::write(socket, boost::asio::buffer(content), ignored_error);
}

class DebugConnection {
 public:
  DebugConnection(int port)
      : io_context_(), socket_(io_context_), acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)) {}

  void listen() {
    std::cerr << "Waiting for debug client to connect on port " << (acceptor_.local_endpoint().port()) << " ...";
    acceptor_.accept(socket_);
    std::cerr << " connected\n";
  }

  std::unique_ptr<charlie::debug::Command> wait_for_command() {
    auto available_bytes = static_cast<int>(socket_.available());
    boost::system::error_code error;
    if (available_bytes == 0) {
      std::vector<char> received_buffer(1);
      auto l = socket_.read_some(boost::asio::buffer(received_buffer), error);
      auto s = std::string(received_buffer.begin(), received_buffer.end());
      if (s != "n") {
        buffer_ << s;
      }

      available_bytes = static_cast<int>(socket_.available());
    }
    std::vector<char> received_buffer(available_bytes);
    size_t len = socket_.read_some(boost::asio::buffer(received_buffer), error);
    if (len == 0) return nullptr;

    if (error == boost::asio::error::eof) {
      std::cerr << "Debug client closed the connection!\n";
      return nullptr;
    } else if (error) {
      std::cerr << "Unexpected connection-error to debug client. Errorcode: " << error << std::endl;
    }
    auto add = std::string(received_buffer.begin(), received_buffer.end());
    buffer_ << add;
    char body_vec[256];
    buffer_.getline(body_vec, sizeof(body_vec), '\0');
    auto flag = buffer_.rdstate();
    auto body = std::string(body_vec);
    auto command = std::make_unique<charlie::debug::Command>();
    auto status = google::protobuf::util::JsonStringToMessage(body, command.get());
    if (status.ok()) {
      return std::move(command);
    }
    std::cerr << status.error_message();
    buffer_ << body_vec;
    return nullptr;
  }

  // For now, ignore multiple commands
  std::unique_ptr<charlie::debug::Command> poll_command() {
    if (socket_.available() > 0) {
      return std::move(wait_for_command());
    }
    return nullptr;
  }

  std::unique_ptr<charlie::debug::Command> get_command(bool blocking) {
    if (blocking || socket_.available() > 0) {
      return std::move(wait_for_command());
    }
    return nullptr;
  }

  void send_message(const google::protobuf::Message& message) {
    std::string event_json;

    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    MessageToJsonString(message, &event_json, options);
    send_string(socket_, event_json);
  }

 private:
  boost::asio::io_context io_context_;
  tcp::socket socket_;
  std::stringstream buffer_;
  tcp::acceptor acceptor_;
};  // namespace charlie::vm

Runtime::Runtime(std::unique_ptr<State> state, std::shared_ptr<program::Mapping> mapping)
    : state_(std::move(state)), mapping_(mapping) {}
int Runtime::Run() {
  while (state_->pos > -1 /* && !state.call_stack.empty()*/) {
    int r = InstructionManager::Instructions[state_->program[state_->pos]](*state_);
    if (r < 0) break;
  }
  if (state_->alu_stack.empty()) return 0;
  return state_->alu_stack.top();
}

void add_variables(const std::vector<std::unique_ptr<program::Mapping::Scope>>& scopes_map, int pos,
                   const Register& reg, charlie::debug::Event::State* proto_state) {
  // Find scopes
  for (auto& scope : scopes_map) {
    auto proto_scope = proto_state->add_scope();
    // proto_scope->set_name(scope->);
    if (scope->begin < pos && scope->end >= pos) {
      for (auto& variable : scope->variables) {
        program::Mapping::Variable v;
        auto proto_var = proto_scope->add_variable();
        int value = 0;  // For now everything is an integer
        reg.GetValue(variable->position, &value);
        proto_var->set_value(value);
        proto_var->set_name(variable->name);
        proto_var->set_type(variable->type);
      }
    }
  }
}

std::string try_function_name(const std::vector<std::unique_ptr<program::Mapping::Function>>& functions_map, int pos) {
  for (auto& func : functions_map) {
    if (func->scope.begin <= pos && func->scope.end >= pos) {
      return func->name;
    }
  }
  return "__global__";  // For initialisations beside the main() function
}

void add_callstack(const State& state, std::shared_ptr<program::Mapping> mapping,
                   charlie::debug::Event::State* proto_state) {
  auto call_stack = state.call_stack;
  if (mapping == nullptr) {
    return;
  }

  *(proto_state->add_callstack_item()) = try_function_name(mapping->Functions, state.pos);

  while (!call_stack.empty()) {
    int pos = call_stack.top();
    call_stack.pop();
    *(proto_state->add_callstack_item()) = try_function_name(mapping->Functions, pos);
    // proto_state->add_callstack_item(call_stack.top());
  }
}

program::Mapping::Location* Runtime::get_location(int pos) {
  auto statement_it = mapping_->Instructions.find(pos);
  if (statement_it != mapping_->Instructions.end()) {
    return &(statement_it->second);
  }
  return nullptr;
}

void Runtime::send_event(int code, DebugConnection* connection, int reason) {
  charlie::debug::Event event;
  event.set_bytecode(code);

  auto state = new charlie::debug::Event::State();
  add_variables(mapping_->Scopes, state_->pos, state_->reg, state);
  add_callstack(*state_, mapping_, state);

  auto loc = get_location(state_->pos);
  if (loc != nullptr) {
    auto position = new charlie::debug::Position();
    position->set_line(loc->line);
    position->set_column(loc->column);
    event.set_allocated_position(position);
  }

  event.set_allocated_state(state);
  event.set_reason(static_cast<charlie::debug::Event_Reason>(reason));
  connection->send_message(event);
}

void send_breakpoints_list(const std::set<int>& breakpoints, DebugConnection* connection) {
  charlie::debug::BreakpointsList breakpoints_list;
  for (auto& breakpoint : breakpoints) {
    auto protp_pos = breakpoints_list.add_position();
    protp_pos->set_line(breakpoint);
  }
  connection->send_message(breakpoints_list);
}

enum class DebugState { ENTRY, RUNNING, PAUSED, TO_NEXT_LINE };

int Runtime::Debug(int port) {
  std::set<int> breakpoints;  // Ignoring filename and column for now
  int last_line;              // Needed for stepping line by line
  int next_line;
  try {
    DebugConnection connection(port);
    connection.listen();

    auto debug_state = DebugState::ENTRY;

    while (state_->pos > -1) {
      // Process commands
      auto command = connection.get_command(debug_state == DebugState::PAUSED);
      if (command) switch (command->type()) {
          case debug::Command::NEXT_STEP:
            next_line = last_line;
            debug_state = DebugState::TO_NEXT_LINE;
            break;
          case debug::Command::RUN:
            debug_state = DebugState::RUNNING;
            break;
          case debug::Command::SET_BREAKPOINT:
            breakpoints.insert(command->position().line());
            break;
          case debug::Command::CLEAR_BREAKPOINT:
            breakpoints.erase(command->position().line());
            break;
          case debug::Command::CLEAR_BREAKPOINTS:
            breakpoints.clear();
            break;
          case debug::Command::QUIT:
            std::cerr << "Client requested a quit\n";
            return 0;
          case debug::Command::LIST_BREAKPOINTS:
            send_breakpoints_list(breakpoints, &connection);
            break;
        }

      int code = state_->program[state_->pos];
      auto loc = get_location(state_->pos);
      // Communicate
      switch (debug_state) {
        case DebugState::RUNNING: {
          // Hitting an breakpoint?
          if (loc != nullptr) {
            if (last_line != loc->line && breakpoints.find(loc->line) != breakpoints.end()) {
              send_event(code, &connection, debug::Event_Reason::Event_Reason_ON_BREAKPOINT);
              debug_state = DebugState::PAUSED;
              last_line = loc->line;
              continue;
            }
            last_line = loc->line;
          }
          int r = InstructionManager::Instructions[code](*state_);
          if (r < 0) goto end;
        } break;
        case DebugState::ENTRY:
        case DebugState::TO_NEXT_LINE: {
          if (loc && last_line != loc->line) {
            last_line = loc->line;
            send_event(code, &connection,
                       debug_state == DebugState::TO_NEXT_LINE ? debug::Event_Reason::Event_Reason_ON_STEP
                                                               : debug::Event_Reason::Event_Reason_ON_ENTRY);
            debug_state = DebugState::PAUSED;
            continue;
          }
          int r = InstructionManager::Instructions[code](*state_);
          if (r < 0) goto end;
        } break;
      }
    }
  end:
    // Wait for client to stop
    sleep(100);  // TODO: Wait for QUIT command from client
    if (state_->alu_stack.empty()) return 0;
    return state_->alu_stack.top();
  } catch (std::exception& e) {
    std::cerr << "\n" << e.what();
    return -1;
  }
  return 0;
}
}  // namespace charlie::vm