const zmq = require("zeromq")

async function run() {
    const sock = new zmq.Subscriber

    sock.connect("tcp://127.0.0.1:3000")

    let subs = Uint8Array.from([3, 0, 0])

    console.log("subscribe to: " + subs)
    sock.subscribe(subs)

    for await (const [msg] of sock) {
        console.log("received a message: " + Uint8Array.from(msg))
        console.log("the value is: " + msg[3]);
    }
}

run()