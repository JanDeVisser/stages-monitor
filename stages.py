import logging
import queue
import random
import sys
import threading
import time

from typing import Union

import _bleio
from adafruit_ble.services import Service
from adafruit_ble.uuid import VendorUUID
from adafruit_ble.characteristics import Characteristic, ComplexCharacteristic

import adafruit_ble
from adafruit_ble.advertising.standard import ProvideServicesAdvertisement
from adafruit_ble.services.standard.device_info import DeviceInfoService
from adafruit_ble_cycling_speed_and_cadence import CyclingSpeedAndCadenceService

from PySide2.QtWidgets import (QApplication, QLabel, QPushButton,
                               QVBoxLayout, QWidget)
from PySide2.QtCore import QObject, Signal, Slot, Qt, QRect, QPoint
from PySide2.QtGui import QFont, QPaintEvent, QPainterPath, QPainter, QPen, QBrush

Buf = Union[bytes, bytearray, memoryview]

UUID_SB20_SERVICE = VendorUUID("0c46beaf-9c22-48ff-ae0e-c6eae1a2f4e5")
UUID_SB20_STATUS = VendorUUID("0c46beb0-9c22-48ff-ae0e-c6eae1a2f4e5")
UUID_SB20_COMMAND = VendorUUID("0c46beb1-9c22-48ff-ae0e-c6eae1a2f4e5")


class StatusQueue:
    """Accumulates a Characteristic's incoming values in a FIFO buffer."""

    def __init__(
        self,
        characteristic: Characteristic,
        *,
        timeout: float = -1,
        buffer_size: int = 64
    ):

        """Monitor the given Characteristic. Each time a new value is written to the Characteristic
        add the newly-written bytes to a FIFO buffer.

        :param Characteristic characteristic: The Characteristic to monitor.
          It may be a local Characteristic provided by a Peripheral Service,
          or a remote Characteristic in a remote Service that a Central has connected to.
        :param int timeout:  the timeout in seconds to wait for the first character
          and between subsequent characters.
        :param int buffer_size: Size of ring buffer that stores incoming data coming from client.
          Must be >= 1."""
        self._characteristic = characteristic
        self._timeout = timeout
        self._buffer_size = buffer_size
        self._queue = queue.Queue(buffer_size)
        characteristic._add_notify_callback(self._notify_callback)

    def _notify_callback(self, data: Buf) -> None:
        # Add data buffer to queue
        while self._queue.full():
            self._queue.get_nowait()

        try:
            self._queue.put_nowait(data)
        except queue.Full:
            return

    def get(self, wait=True):
        return self._queue.get(wait, None)


class _SB20Notification(ComplexCharacteristic):
    """Notify-only characteristic of SB20 status."""

    uuid = UUID_SB20_STATUS

    def __init__(self):
        super().__init__(properties=Characteristic.NOTIFY)

    def bind(self, service):
        """Bind to a SB20Service."""
        bound_characteristic = super().bind(service)
        bound_characteristic.set_cccd(notify=True)
        return StatusQueue(bound_characteristic, timeout=0.1)


def _hex(value):
    if value is None or len(value) == 0:
        return ''
    ret = "{0:02x}".format(value[0])
    for v in value[1:10]:
        ret = "{0} {1:02x}".format(ret, v)
    return ret


def hex(*values):
    return ", ".join([_hex(value) for value in values])


class GearNotification(QObject):
    gears_changed = Signal(str)
    status_message = Signal(str)


class SB20Service(Service):
    """
        Service for monitoring Stages SB20 status
    """

    uuid = UUID_SB20_SERVICE
    _status = _SB20Notification()
    command = Characteristic(uuid=UUID_SB20_COMMAND, properties=Characteristic.WRITE_NO_RESPONSE)

    def __init__(self, *args, **kwargs):
        super(SB20Service, self).__init__(*args, **kwargs)
        self.connection = None
        self.current_chainring = (1, 34)
        self.current_cog = (1, 33)

    def status(self):
        buf = self._status.get()
        if len(buf) > 0:
            if buf[:3] == b'\x0c\01\00' and len(buf) >= 8:
                self.current_chainring = (buf[3], buf[5])
                self.current_cog = (buf[4], buf[7])
                self.gears.gears_changed.emit("foo")
            return buf
        return None

    def status_message(self, msg):
        self.gears.status_message.emit(msg)

    def __call__(self):
        self.bootstrap()
        self.status_message("Service bootstrapped")
        self.suspend = False
        while not self.suspend:
            status = self.status()
            time.sleep(0.1)
        self.connection.disconnect()
        self.status_message("Disconnected")

    def run(self):
        t = threading.Thread(target=self, daemon=True)
        t.start()

    def disconnect(self):
        self.suspend = True

    def expect(self, *values):
        if len(values) == 0:
            return True
        # TODO Reset counter on b'\xfd\04'
        for count in range(0, 5):
            status = self.status()
            if status is not None:
                for v in values:
                    if status[:len(v)] == v:
                        return True
                self.status_message("Discarding {0}".format(hex(status)))
            time.sleep(1)
        return False

    def challenge(self, challenge, *resp):
        self.status_message("Sending {0} expecting {1}".format(hex(challenge), hex(*resp)))
        self.command = challenge
        if not self.expect(*resp):
            raise Exception("Expected %s after %s" % (hex(*resp), hex(challenge)))

    def display(self):
        return "Chainring {0} ({1}) Cog {2} ({3})".format(self.current_chainring[0], self.current_chainring[1],
                                                          self.current_cog[0], self.current_cog[1])

    @classmethod
    def connect(cls, notifier):

        def update_status(msg):
            notifier.status_message.emit(msg)

        # PyLint can't find BLERadio for some reason so special case it here.
        ble = adafruit_ble.BLERadio()  # pylint: disable=no-member

        sb20_connection = None

        update_status("Scanning...")
        for adv in ble.start_scan(ProvideServicesAdvertisement, timeout=5):
            if CyclingSpeedAndCadenceService in adv.services:
                update_status("Found a Cycling Speed and Cadence profile")
                sb20_connection = ble.connect(adv)
                if SB20Service not in sb20_connection:
                    update_status("Device is not a Stages SB20")
                    sb20_connection.disconnect()
                    sb20_connection = None
                else:
                    update_status("Connected")
                    break

        # Stop scanning whether or not we are connected.
        ble.stop_scan()

        if sb20_connection and sb20_connection.connected:
            if DeviceInfoService in sb20_connection:
                dis = sb20_connection[DeviceInfoService]
                try:
                    manufacturer = dis.manufacturer
                except AttributeError:
                    manufacturer = "(Manufacturer Not specified)"
                try:
                    model_number = dis.model_number
                except AttributeError:
                    model_number = "(Model number not specified)"
                update_status("Device: {0} {1}".format(manufacturer, model_number))
            else:
                update_status("No device information")

            sb20: SB20Service = sb20_connection[SB20Service]
            sb20.connection = sb20_connection
            sb20.gears = notifier
            return sb20
        return None

    def bootstrap(self):
        try:
            self._status.get(False)
        except queue.Empty:
            pass
        setup = [
            (b'\x08\0', b'\x08\0'),
            (b'\x0c\0\x01', b'\x0c\0\x01'),
            (b'\x0a\0\0\0', b'\x0a\x01'),
            (b'\x0d\x02', b'\x0d\x02'),
            (b'\x0d\x04', b'\x0d\x04'),
            (b'\x0e\x00', b'\x0e\x00'),
            (b'\x08\0', b'\x08\0'),
            (b'\x0c\0\x01', b'\x0c\0\x01'),
            (b'\x0b\00\04\04\02\03\03\01\01\02\03\04\01\02\03\04\00', b'\x0b\0', b'\x0c\01'),
            (b'\x10\0\x01', b'\x10\0\x01', b'\x0c\01'),
            (b'\x0c\00\02\05\01\xc8\00\01', b'\05\01', b'\x0c\01'),
            (b'\x0c\0\x02', b'\x0c\01'),
            (b'\x0c\02\0\0\02\x0c\x10\0\03\x0e\0', b'\x0c\02', b'\x0c\01'),
            (b'\x0c\0\x02', b'\x0c\01'),
            (b'\x0c\03\x22\x32\x21\x1c\x18\x15\x13\x11\x0f\x0e\x0d\x0c\x0b\x0a',),
            (b'\x0c\x02\x0b\x00\x02\x0c\x10\x00\x03\x0e\x00', b'\x0c\x02', b'\x0c\x01'),
            (b'\xfd\0', b'\xfd\x01', b'\x0c\01'),
            (b'\x03\x01\x4c\x1d\0\0', b'\x03\01', b'\x0c\01'),
            (b'\x0c\0\x02', b'\x0c\01'),
            (b'\x0c\0\x02', b'\x0c\01'),
            (b'\x0c\0\x02', b'\x0c\01'),
        ]
        for s in setup:
            self.challenge(*s)

    def chainrings(self):
        return (34, 50)

    def cogs(self):
        return (33, 28, 24, 21, 19, 17, 15, 14, 13, 12, 11, 10)

    def chainring_index(self):
        return self.current_chainring[0]

    def chainring_size(self):
        return self.current_chainring[1]

    def cog_index(self):
        return self.current_cog[0]

    def cog_size(self):
        return self.current_cog[1]


class GearsWidget(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.service = None
        self.pen = QPen()
        self.brush = QBrush()

    def set_service(self, service: SB20Service):
        self.service = service

    def unset_service(self):
        self.service = None

    def paintEvent(self, event: QPaintEvent) -> None:
        if self.service is None:
            return
        path = QPainterPath()
        path.moveTo(20, 80)
        path.lineTo(20, 30)
        path.cubicTo(80, 0, 50, 50, 80, 80)

        painter = QPainter(self)
        painter.setPen(self.pen)
        painter.setBrush(self.brush)
        painter.setRenderHint(QPainter.Antialiasing, True)

        x_inc = len(self.service.chainrings()) + len(self.service.cogs()) + 3
        w = int(2 * x_inc / 3)
        x = x_inc
        y_max = self.height() * 0.9
        y_0 = self.height() * 0.05
        factor = y_max / self.service.chainrings()[-1]
        for chainring in self.service.chainrings():
            y = chainring * factor
            r = QRect(x, int(y_0 + (y_max - y)/2), w, int(y))
            if chainring == self.service.chainring_size():
                painter.setBrush(Qt.SolidPattern)
            else:
                painter.setBrush(Qt.NoBrush)
            painter.drawRoundedRect(r, 25, 25, Qt.RelativeSize)
            x += x_inc

        x = self.width() - (len(self.service.cogs()) + 1) * x_inc
        factor = y_max / self.service.cogs()[0]
        for cog in self.service.cogs():
            y = cog * factor
            r = QRect(x, int(y_0 + (y_max - y)/2), w, int(y))
            if cog == self.service.cog_size():
                painter.setBrush(Qt.SolidPattern)
            else:
                painter.setBrush(Qt.NoBrush)
            painter.drawRoundedRect(r, 25, 25, Qt.RelativeSize)
            x += x_inc

        painter.setRenderHint(QPainter.Antialiasing, False)
        painter.setPen(self.palette().dark().color())
        painter.setBrush(Qt.NoBrush)
        painter.drawRect(QRect(0, 0, self.width() - 1, self.height() - 1))

    @Slot()
    def update_gears(self):
        self.update()


class MyWidget(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.button = QPushButton("Connect")
        self.disconnect_button = QPushButton("Disconnect")
        self.disconnect_button.hide()
        self.status = QLabel("")
        self.status.setAlignment(Qt.AlignLeft)
        self.text = QLabel("Hello World")
        self.text.setAlignment(Qt.AlignCenter)
        self.text.setFont(QFont("Courier", 100))
        self.gears = GearsWidget()

        self.layout = QVBoxLayout()
        self.layout.addWidget(self.text)
        self.layout.addWidget(self.button)
        self.layout.addWidget(self.disconnect_button)
        self.layout.addWidget(self.status)
        self.layout.addWidget(self.gears)
        self.setLayout(self.layout)
        self.service: SB20Service = None

        # Connecting the signal
        self.button.clicked.connect(self.connect_service)
        self.disconnect_button.clicked.connect(self.disconnect_service)

    @Slot()
    def connect_service(self):
        def connector():
            notifier = GearNotification()
            notifier.gears_changed.connect(self.show_gearing)
            notifier.gears_changed.connect(self.gears.update_gears)
            notifier.status_message.connect(self.update_status)
            self.service = SB20Service.connect(notifier)
            self.gears.set_service(self.service)
            if self.service is not None:
                self.service.run()

        self.button.hide()
        threading.Thread(target=connector).start()
        self.disconnect_button.show()

    @Slot()
    def disconnect_service(self):
        self.service.disconnect()
        self.service = None
        self.gears.unset_service()
        self.disconnect_button.hide()
        self.button.show()

    @Slot()
    def show_gearing(self, s):
        chainring = self.service.chainring_index()
        chainring_size = self.service.chainring_size()
        cog = self.service.cog_index()
        cog_size = self.service.cog_size()
        self.text.setText("{0} ({1}) : {2} ({3})".format(chainring, chainring_size, cog, cog_size))

    @Slot()
    def update_status(self, status):
        self.status.setText(status)


if __name__ == "__main__":
    # logging.basicConfig(level=logging.DEBUG)
    app = QApplication(sys.argv)

    widget = MyWidget()
    widget.resize(800, 600)
    widget.show()

    sys.exit(app.exec_())