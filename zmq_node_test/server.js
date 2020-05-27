const zmq = require("zeromq")

async function run() {
  const sock = new zmq.Reply

  await sock.bind("tcp://127.0.0.1:3001")

  for await (const [msg] of sock) {

    console.log("Received: " + Uint8Array.from(msg));

    answer = [msg[0], 0x02];

    switch(msg[0]) {
        case 0x00:
            answer = [msg[0], 0x00, '2', 'S'];
            break;
        case 0x04:
            answer = [msg[0], 0x00];
            if(msg[1] >= 3) answer[1] = 3;
            break;
        case 0x05:
            answer = [msg[0], 0x00];
            if(msg[1] >= 3) answer[1] = 3;
            break;
    }

    console.log("Sending: " + Uint8Array.from(answer));

    await sock.send(Uint8Array.from(answer))
  }
}

run()
