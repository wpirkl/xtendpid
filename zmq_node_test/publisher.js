const zmq = require("zeromq")

async function run() {
  const sock = new zmq.Publisher

  await sock.bind("tcp://127.0.0.1:3000")
  console.log("Publisher bound to port 3000")

  let topic = [0x03, 0x00, 0x00, 0x00]

  let i = 0;

  while (true) {

    let message;

    if((++i) % 2 == 0) {
        topic[3] = 0;
    } else {
        topic[3] = 1;
    }

    message = Uint8Array.from(topic)

    console.log("Sending to: " + message);

    await sock.send(message);
    await new Promise(resolve => setTimeout(resolve, 500))
  }
}

run()