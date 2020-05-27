const zmq = require("zeromq")

async function run() {
  const sock = new zmq.Request

  sock.connect("tcp://127.0.0.1:3001")
  console.log("Producer bound to port 3001")

  let sending = [4, 0, 1];
  console.log("Sending: " + sending);

  await sock.send(Uint8Array.from(sending))
  const [result] = await sock.receive()

  console.log("received: " + Uint8Array.from(result))
}

run()
