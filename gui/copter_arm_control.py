import sys

from PyQt5.QtWidgets import (
    QApplication, QDialog, QMainWindow, QMessageBox
)

from PyQt5.uic import loadUi
from main_window import Ui_MainWindow

import time
import math
import cobs.cobs as cobs
import crcmod
import struct
import serial
from serial.tools.list_ports import comports

import asyncio
from qasync import QEventLoop
from periodic import Periodic

RX_FORMAT = '<f'
IO_INTERVAL = 0.01
SETPOINT_INTERVAL = 0.10

class Window(QMainWindow, Ui_MainWindow):

    def __init__(self, parent=None):
        self.armed = False

        self.serial = serial.Serial(timeout=0)
        self.serial_data: bytearray = bytearray()
        self.last_transmit = 0

        super().__init__(parent)
        self.setupUi(self)
        self.connect_signals()

        for port_info in comports():
            self.list_serial_path.addItem(port_info.device)

    def connect_signals(self):
        self.btn_connect_serial.clicked.connect(self.connect_serial)

        self.btn_arm.clicked.connect(self.arm)
        self.btn_disarm.clicked.connect(self.disarm)

    def connect_serial(self):
        if self.serial.is_open:
            self.serial.close()
            self.label_serial_status.setText("status: disconnected")
        
        self.serial.port = self.list_serial_path.currentText()
        self.serial.baudrate = 57600
        self.serial.open()
        self.label_serial_status.setText("status: connected")

    def update_armed_state(self, armed: bool):
        self.armed = armed
        if armed:
            armed_string = "status: armed"
        else:
            armed_string = "status: disarmed"
        self.label_armed_status.setText(armed_string)

    def arm(self):
        self.update_armed_state(True)

    def disarm(self):
        self.update_armed_state(False)

    async def io_loop(self):
        io = Periodic(self.do_io, IO_INTERVAL)
        await io.start()

    def do_io(self):
        if not self.serial.is_open:
            return

        self.receive_state()
        self.transmit_setpoint()

    def receive_state(self):
        self.serial_data += self.serial.read_all()

        i_delimiter = self.serial_data.find(b'\x00')
        while i_delimiter >= 0:
            try:
                data = self.serial_data[:i_delimiter]
                self.serial_data = self.serial_data[i_delimiter + 1:]
                unstuffed_data = cobs.decode(data)
                if len(unstuffed_data) == struct.calcsize(RX_FORMAT):
                    pos, = struct.unpack(RX_FORMAT, unstuffed_data)
                    pos_deg = math.degrees(pos)
                    self.label_position.setText(format(pos_deg, '.1f'))
            except cobs.DecodeError:
                pass

            i_delimiter = self.serial_data.find(b'\x00')

    def transmit_setpoint(self):
        now = time.time()
        if time.time() > self.last_transmit + SETPOINT_INTERVAL:
            self.last_transmit = now
            setpoint = math.radians(self.slider_setpoint.value())
            data = struct.pack('<?f', self.armed, setpoint)
            stuffed_data = cobs.encode(data) + b'\x00'
            self.serial.write(stuffed_data)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = Window()
    win.show()

    loop = QEventLoop()
    asyncio.set_event_loop(loop)

    with loop:
        loop.create_task(win.io_loop())
        loop.run_forever()