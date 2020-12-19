import io
import json

OPCODE_COMMAND = 0x52
OPCODE_NOTIFICATION = 0x1b

CHANNEL_COMMAND = 0x39
CHANNEL_NOTIFICATION = 0x36


class Command:
    def __init__(self, counter, value):
        self.counter = counter
        self.value = value

    def remarks(self):
        if self.value.startswith("0c 03"):
            b = [int(octet, 16) for octet in self.value[6:].split(" ")]
            values = []
            for ix in range(0, int(len(b)/2)):
                lsb = b[ix*2]
                msb = b[ix*2 + 1]
                v = msb*256 + lsb
                values.append(str(v))
            return " ".join(values)
        else:
            return ""


class Notification:
    def __init__(self, counter, value):
        self.counter = counter
        self.value = value


with open("connect-shimano.json") as fh:
    data = json.load(fh)

    prev_cmd: Command = None
    for record in data:
        att = record["_source"]["layers"]["btatt"]
        counter = int(record["_source"]["layers"]["nordic_ble"]["nordic_ble.event_counter"])
        opcode = int(att["btatt.opcode"], 16)
        handle = int(att.get("btatt.handle", "0x0"), 16)
        value = att.get("btatt.value", "00:00").replace(":", " ")
        if opcode == OPCODE_COMMAND and handle == CHANNEL_COMMAND:
            if not prev_cmd or prev_cmd.value != value:
                if prev_cmd:
                    print("| {0} | `{1}` | | {2} |".format(prev_cmd.counter, prev_cmd.value, prev_cmd.remarks()))
                cmd = Command(counter, value)
                prev_cmd = cmd
        elif opcode == OPCODE_NOTIFICATION and handle == CHANNEL_NOTIFICATION:
            notif = Notification(counter, value)
            if prev_cmd and prev_cmd.counter == notif.counter:
                print("| {0} | `{1}` | `{2}` | {3} |".format(counter, prev_cmd.value, notif.value, prev_cmd.remarks()))
            else:
                if prev_cmd:
                    print("| {0} | `{1}` | | {2} |".format(prev_cmd.counter, prev_cmd.value, prev_cmd.remarks()))
                print("| {0} | | `{1}` | |".format(counter, value))
                prev_cmd = None
