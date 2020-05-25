# Installation

Install ZeroMQ.js with prebuilt binaries:

```sh
npm install zeromq@6.0.0-beta.6
```

# Examples

**Note:** The examples assume the reader is familiar with ZeroMQ. If you are new to ZeroMQ, please start with the [ZeroMQ documentation](https://zeromq.org/get-started/).


## Push/Pull

This example demonstrates how a producer pushes information onto a
socket and how a worker pulls information from the socket.


### producer.js

```js
const zmq = require("zeromq")

async function run() {
  const sock = new zmq.Push

  await sock.bind("tcp://127.0.0.1:3000")
  console.log("Producer bound to port 3000")

  while (true) {
    await sock.send("some work")
    await new Promise(resolve => setTimeout(resolve, 500))
  }
}

run()
```

### worker.js

```js
const zmq = require("zeromq")

async function run() {
  const sock = new zmq.Pull

  sock.connect("tcp://127.0.0.1:3000")
  console.log("Worker connected to port 3000")

  for await (const [msg] of sock) {
    console.log("work: %s", msg.toString())
  }
}

run()
```

## Pub/Sub

This example demonstrates using `zeromq` in a classic Pub/Sub,
Publisher/Subscriber, application.

### publisher.js

```js
const zmq = require("zeromq")

async function run() {
  const sock = new zmq.Publisher

  await sock.bind("tcp://127.0.0.1:3000")
  console.log("Publisher bound to port 3000")

  while (true) {
    console.log("sending a multipart message envelope")
    await sock.send(["kitty cats", "meow!"])
    await new Promise(resolve => setTimeout(resolve, 500))
  }
}

run()
```

### subscriber.js

```js
const zmq = require("zeromq")

async function run() {
  const sock = new zmq.Subscriber

  sock.connect("tcp://127.0.0.1:3000")
  sock.subscribe("kitty cats")
  console.log("Subscriber connected to port 3000")

  for await (const [topic, msg] of sock) {
    console.log("received a message related to:", topic, "containing message:", msg)
  }
}

run()
```

## Req/Rep

### client.js

```js
const zmq = require("zeromq")

async function run() {
  const sock = new zmq.Request

  sock.connect("tcp://127.0.0.1:3000")
  console.log("Producer bound to port 3000")

  await sock.send("4")
  const [result] = await sock.receive()

  console.log(result)
}

run()
```

### server.js

```js
const zmq = require("zeromq")

async function run() {
  const sock = new zmq.Reply

  await sock.bind("tcp://127.0.0.1:3000")

  for await (const [msg] of sock) {
    await sock.send(2 * parseInt(msg, 10))
  }
}

run()
```
