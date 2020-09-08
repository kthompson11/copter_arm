# Copter Arm

Copter Arm is a simple project that uses feedback control with a quadcopter motor and propellor to stabilize an arm at a desired angle. The angle is set at compile time, but in the future will be settable using a web interface. The firmware runs on a NUCLEO-F767ZI board.

## Components

- optimization - contains scripts used to optimize system parameters
- src - contains the stm32f7 firmware
- server - contains a simple Node.js server that receives information over a serial port and prints out debugging information

## Requirements

To install the firmware, the following is required:

- open source stlink tools (https://github.com/stlink-org/stlink)
- a NUCLEO-F767ZI board
- arm gcc toolchain

Running the Node.js server requires Node.js and the following npm packages:

- crc
- serialport

## Installation - Firmware

1. Install gnu ARM embedded toolchain

2. Install open source stlink tools

3. Compile the firmware with `make`

4. Flash the firmware to the device with `make flash`

## Node.js Server

1. Navigate to the `server` directory

2. Start the server by entering `node server.js`

## Testing

There are unit tests for some firmware modules. To run the tests:

1. Install cmocka

2. Install cmake

2. Run all tests using `make test`

3. Run individual tests by navigating to `tests/build/bin/` and run the test executable
