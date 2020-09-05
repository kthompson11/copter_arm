var cobs = require('./cobs');
var crc = require('crc');

const BAUD_RATE = 9600;
const SERIAL_PORT_PATH = '/dev/ttyACM0';

const SerialPort = require('serialport');
const Delimiter = require('@serialport/parser-delimiter')
const port = new SerialPort(SERIAL_PORT_PATH, { baudRate: BAUD_RATE });

const parser = new Delimiter({delimiter: '\0', includeDelimiter: true})
port.pipe(parser)

parser.on('data', data => parsePacket(data));

var totalPacketErrors = 0;

function parsePacket(data) {
  var unstuffed = cobs.cobs_unstuff(data);
  if (unstuffed === false) {
    console.log("Bad packet");
    totalPacketErrors += 1;
    return;
  }

  packetCRC = unstuffed.readUInt32BE(unstuffed.length - 4);
  computedCRC = crc.crc32(unstuffed.slice(0, unstuffed.length - 4));
  
  if (packetCRC !== computedCRC) {
    totalPacketErrors += 1;
  }
  console.log(packetCRC.toString(16) + " , " + computedCRC.toString(16) + " , packet errors = " + totalPacketErrors.toString() + ' , data = ' + unstuffed.readUInt16BE().toString());
}
