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

  // For now, ignore multiple commands
  std::unique_ptr<charlie::debug::Command> poll_command() {
    if (socket_.available() > 0) {
      auto available_bytes = socket_.available();
      boost::system::error_code error;
      std::vector<char> received_buffer(available_bytes + 1);
      size_t len = socket_.read_some(boost::asio::buffer(received_buffer), error);

      if (error == boost::asio::error::eof) {
        std::cerr << "Debug client closed the connection!\n";
        return nullptr;
      } else if (error) {
        std::cerr << "Unexpected connection-error to debug client. Errorcode: " << error << std::endl;
      }
      buffer_ << std::string(received_buffer.begin(), received_buffer.end());
      char header[6];
      buffer_.read(header, sizeof(header) - 1);
      header[sizeof(header) - 1] = '\0';
      int length = atoi(&header[1]);

      // Copy the relevant json code into std::string
      // if (buffer_.gcount() >= length) {
      std::vector<char> body_vec(length + 1);
      buffer_.read(&body_vec[0], length);
      body_vec[length] = '\0';
      auto body = std::string(&body_vec[0], &body_vec[length]);
      auto command = std::make_unique<charlie::debug::Command>();
      auto status = google::protobuf::util::JsonStringToMessage(body, command.get());
      if (status.ok()) {
        return command;
      }
      std::cerr << status.error_message();
      // }
    }
    return nullptr;
  }

  void send_event(const charlie::debug::Event& event) {
    std::string event_json;

    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    MessageToJsonString(event, &event_json, options);
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

void add_variables(const Register& reg, charlie::debug::Event::State* state) {
  for (int i = 0; i < reg.GetSize(); ++i) {
    auto variable = state->add_variable();
    int value;
    reg.GetValue(i, &value);
    variable->set_value(value);
    std::stringstream ss;
    ss << "i" << i;
    variable->set_name(ss.str());
  }
}

std::string try_function_name(const std::vector<std::unique_ptr<program::Mapping::Function>>& functions_map, int pos) {
  for (auto& func : functions_map) {
    if (func->scope.begin <= pos && func->scope.end >= pos) {
      return func->name;
      // proto_state->add_callstack_item(func->name);
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

enum class DebugState { RUNNING, PAUSED, TO_NEXT_LINE };

int Runtime::Debug(int port) {
  try {
    DebugConnection connection(port);
    connection.listen();

    auto debug_state = DebugState::TO_NEXT_LINE;

    while (state_->pos > -1 /* && !state.call_stack.empty()*/) {
      int code = state_->program[state_->pos];
      // Communicate
      if (debug_state == DebugState::TO_NEXT_LINE) {
        charlie::debug::Event event;
        event.set_bytecode(code);
        auto position = new charlie::debug::Position();
        position->set_line(state_->pos);
        event.set_allocated_position(position);

        auto state = new charlie::debug::Event::State();
        add_variables(state_->reg, state);
        add_callstack(*state_, mapping_, state);

        event.set_allocated_state(state);

        connection.send_event(event);
      }
      auto command = connection.poll_command();

      int r = InstructionManager::Instructions[code](*state_);
      if (r < 0) break;
    }
    // Wait for client to stop
    sleep(100);
    if (state_->alu_stack.empty()) return 0;
    return state_->alu_stack.top();
  } catch (std::exception& e) {
    std::cerr << "\n" << e.what();
    return -1;
  }
  return 0;
}
}  // namespace charlie::vm