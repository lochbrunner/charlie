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
  const length = `0000${buffer.length}`.slice(-4);
  client.write(length + buffer, 'utf8', obj => {
    console.log(`Send command: '${length + buffer}'`);
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
    client.emit('event', JSON.parse(chunk));
  }
});

client.on('event', obj => {
  console.log(`Received: ${JSON.stringify(obj)}`);
});

process.stdin.pipe(require('split')()).on('data', line => {
  client.write(line, 'utf8', obj => {
    send_command({type: protocol.Type.NEXT_STEP});
  });
})

client.on('close', () => {
  console.log('Connection closed');
  process.exit(-1);
});
