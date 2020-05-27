const zmq = require("zeromq")

async function run() {
  const sock = new zmq.Request

  sock.connect("tcp://127.0.0.1:3001")
  console.log("Producer bound to port 3001")

  await sock.send("4")
  const [result] = await sock.receive()

  console.log(result)
}

run()