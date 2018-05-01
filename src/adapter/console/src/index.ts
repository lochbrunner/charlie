import * as net from 'net';
import * as readline from 'readline';

import * as protocol from './protocol';

const client = new net.Socket();

client.connect(3232, 'localhost', () => {
  console.log('Connected');
});

let remaining_code = '';

function send_command(command: protocol.Command) {
  const buffer = JSON.stringify(command);
  const send_command = buffer + '\0';
  client.write(send_command, 'utf8', obj => {
    console.log(`Send command: '${send_command}'`);
  });
}

client.on('data', (data: string) => {
  // First four characters indicate length
  data = remaining_code + data;
  const length = data.length;
  while (true) {
    const chunkLength = parseInt(data.substr(0, 4));
    if (isNaN(chunkLength) || chunkLength > length + 4) {
      remaining_code = data;
      break;
    }
    const chunk = data.substr(4, chunkLength);
    data = data.substr(4 + chunkLength);
    if (chunk.length > 0) {
      try {
        client.emit('event', JSON.parse(chunk));
      } catch (e) {
        console.log(`Could not parse event Json '${chunk}': ${e}`);
      }
    }
  }
});

client.on('event', (obj: Event) => {
  console.log(`Received: ${JSON.stringify(obj)}`);
});

function try_parse_b(line: string): protocol.Command|null {
  // Setting a breakpoint?
  // b <filename>:<linenumber>
  const reg = /b\W+([\w.]+):(\d+)\W*/g;
  const match = reg.exec(line);
  if (match) {
    const filename = match[1];
    const linenumber = parseInt(match[2]);
    return {type: protocol.Type.SET_BREAKPOINT, position: {filename, line: linenumber, column: 0}};
  } else {
    return null;
  }
}

function try_parse_clear(line: string): protocol.Command|null {
  // Removing a breakpoint?
  // clear <filename>:<linenumber>
  const reg = /clear\W+([\w.]+):(\d+)\W*/g;
  const match = reg.exec(line);
  if (match) {
    const filename = match[1];
    const linenumber = parseInt(match[2]);
    return {type: protocol.Type.CLEAR_BREAKPOINT, position: {filename, line: linenumber, column: 0}};
  } else {
    return null;
  }
}

process.stdin.pipe(require('split')()).on('data', line => {
  switch (line) {
    case 'r':
      send_command({type: protocol.Type.RUN});
      break;
    case 'n':
      send_command({type: protocol.Type.NEXT_STEP});
      break;
    case 'q':
      send_command({type: protocol.Type.QUIT});
      break;
    case 'list':
      send_command({type: protocol.Type.LIST_BREAKPOINTS});
      break;
    default:
      // Complex commands ?
      const command = try_parse_b(line) || try_parse_clear(line);
      if (command) {
        send_command(command);
      } else {
        console.error(`Unknown command '${line}' !`);
      }
      break;
  }
})

client.on('close', () => {
  console.log('Connection closed by debug server');
  process.exit(0);
});
