import {EventEmitter} from 'events';
import * as net from 'net';

import * as protocol from '../../common/protocol';

export class ConnectorRuntime extends EventEmitter {
  private readonly client_ = new net.Socket();
  private remaining_code_ = '';

  private last_state_: protocol.Event = {
    bytecode: -1,
    position: {column: -1, filename: '', line: -1},
    reason: 1,
    state: {callstackItem: [], variable: []}
  };

  public start(program: string, hostname: string, port: number) {
    this.client_.connect(port, hostname, () => {});

    this.client_.on('data', (data: string) => {
      // First four characters indicate length
      data = this.remaining_code_ + data;
      const length = data.length;
      while (true) {
        const chunkLength = parseInt(data.substr(0, 4));
        if (isNaN(chunkLength) || chunkLength > length + 4) {
          this.remaining_code_ = data;
          break;
        }
        const chunk = data.substr(4, chunkLength);
        data = data.substr(4 + chunkLength);
        if (chunk.length > 0) {
          try {
            // this.client_.emit('event', JSON.parse(chunk));
            this.last_state_ = JSON.parse(chunk) as protocol.Event;
            switch (this.last_state_.reason) {
              case protocol.EventReason.ON_ENTRY:
                this.emit('stopOnEntry', this.last_state_);
                break;
              case protocol.EventReason.ON_STEP:
                this.emit('stopOnStep', this.last_state_);
                break;
              case protocol.EventReason.ON_BREAKPOINT:
                this.emit('stopOnBreakpoint', this.last_state_);
                break;
              case protocol.EventReason.ON_EXCEPTION:
                this.emit('stopOnException', this.last_state_);
                break;
            }
          } catch (e) {
            this.emit('error', `Could not parse event Json '${chunk}': ${e}`);
          }
        }
      }
    });
  }

  public continue() {
    this.sendCommand({type: protocol.Type.RUN});
  }
  public step() {
    this.sendCommand({type: protocol.Type.NEXT_STEP});
  }

  public setBreakPoint(file: string, line: number, column?: number) {
    this.sendCommand({type: protocol.Type.SET_BREAKPOINT, position: {filename: file, line, column: column || 0}});
    return {verified: true, line, id: 1};
  }
  public clearBreakPoint(file: string, line: number) {
    this.sendCommand({type: protocol.Type.CLEAR_BREAKPOINT, position: {filename: file, line, column: 0}});
  }
  public clearBreakpoints(file: string): void {
    this.sendCommand({type: protocol.Type.CLEAR_BREAKPOINTS, position: {filename: file, line: 0, column: 0}});
  }

  public stack(startFrame: number, endFrame: number) {
    const ls = this.last_state_;
    const frames = ls.state.callstackItem.map(
        (item, index) => ({name: item, index, file: ls.position.filename, line: ls.position.line}));

    return {
      frames, count: frames.length
    }
  }

  public scope() {
    // return this.last_state_.state.variable.map(varaible => ({s: varaible.})
  }

  public variables() {
    return this.last_state_.state.variable;
  }

  private sendCommand(command: protocol.Command) {
    const buffer = JSON.stringify(command);
    const send_command = buffer + '\0';
    this.client_.write(send_command, 'utf8', obj => {
      this.emit('error', `Send command: '${send_command}'`);
    });
  }
}