const SerialPort = require("serialport");
const Readline = require("@serialport/parser-readline");

const port = new SerialPort("/dev/cu.SLAB_USBtoUART", {
  baudRate: 115200,
});

const parser = port.pipe(new Readline({ delimiter: "\n" }));
// Read the port data
port.on("open", () => {
  console.log("Serial Port open");
});
parser.on("data", (data) => {
  console.log(data);
});
