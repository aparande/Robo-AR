#!/usr/bin/env python3
import struct
import time
import keyboard
import argparse
from getpass import getpass
from bluepy.btle import Peripheral, DefaultDelegate
import binascii

parser = argparse.ArgumentParser(description='Print advertisement data from a BLE device')
parser.add_argument('addr', metavar='address', type=str, help='Address of the form XX:XX:XX:XX:XX:XX')
args = parser.parse_args()
addr = args.addr.lower()
if len(addr) != 17:
    raise ValueError("Invalid address supplied")

SERVICE_UUID = "4607eda0-f65e-4d59-a9ff-84420d87a4ca"
CHAR_UUIDS = "4607eda1-f65e-4d59-a9ff-84420d87a4ca"# add your characteristics
ACK_UUID = "4607eda2-f65e-4d59-a9ff-84420d87a4ca"# add your characteristics

class RobotController():

    def __init__(self, address):

        self.robot = Peripheral(addr)
        print("connected")

        # keep state for keypresses
        self.pressed = {"up": False, "down": False, "right": False, "left": False}
        # get service from robot
        # get characteristic handles from service/robot
        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)
        self.ch = self.sv.getCharacteristics(CHAR_UUIDS)[0]
        self.ack = self.sv.getCharacteristics(ACK_UUID)[0]
        #keyboard.hook(self.on_key_event)

        while True:
            #ack = int(binascii.b2a_hex(self.ack.read())[:2], 16)
            ack, = struct.unpack("Bxxx", self.ack.read())
            if ack==0:
                r = float(input("Distance (m): "))
                t = float(input("Angle (degrees): "))
                self.ch.write(struct.pack('ff', *[r, t]));
                self.ack.write(struct.pack('Bxxx', *[ack+1]))
                

#    def on_key_event(self, event):
#        # print key name
#        print(event.name)
#        # if a key unrelated to direction keys is pressed, ignore
#        if event.name not in self.pressed: return
#        # if a key is pressed down
#        if event.event_type == keyboard.KEY_DOWN:
#            # if that key is already pressed down, ignore
#            if self.pressed[event.name]: return
#            # set state of key to pressed
#            self.pressed[event.name] = True
#            self.ch.write(bytes([0x01, 0x01]));
#        else:
#            # set state of key to released
#            self.pressed[event.name] = False

    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr) as robot:
    getpass('Input waypoints for car')
