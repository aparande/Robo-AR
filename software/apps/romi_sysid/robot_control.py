#!/usr/bin/env python3
import struct
import time
import keyboard
import argparse
from getpass import getpass
from bluepy.btle import Peripheral, DefaultDelegate

parser = argparse.ArgumentParser(description='Print advertisement data from a BLE device')
args = parser.parse_args()
#addr = args.addr.lower()
addr = 'C0:98:E5:51:EE:C5'
if len(addr) != 17:
    raise ValueError("Invalid address supplied")

SERVICE_UUID = "4607eda0-f65e-4d59-a9ff-84420d87a4ca"
CHAR_UUID = "4607eda1-f65e-4d59-a9ff-84420d87a4ca"# add your characteristics
DATA_UUID = "4607eda2-f65e-4d59-a9ff-84420d87a4ca"# add your characteristics
ACK_UUID = "4607eda3-f65e-4d59-a9ff-84420d87a4ca"# add your characteristics
DATA_READY_UUID = "4607eda4-f65e-4d59-a9ff-84420d87a4ca"# add your characteristics

class RobotDelegate(DefaultDelegate):
    def __init__(self, controller):
        self.robot_controller = controller
        DefaultDelegate.__init__(self)
    def handleNotification(self, cHandle, data):
        ack = struct.unpack("B", self.ack.read())[0]
        print(cHandle);
        print(data)


class RobotController():

    def __init__(self, address):

        self.robot = Peripheral(addr)
        self.robot.setDelegate(RobotDelegate(self))
        print("connected")

        # keep state for keypresses
        self.pressed = {"up": False, "down": False, "right": False, "left": False}
        # get service from robot
        # get characteristic handles from service/robot
        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)
        self.ch = self.sv.getCharacteristics(CHAR_UUID)[0]
        self.data = self.sv.getCharacteristics(DATA_UUID)[0]
        self.ack = self.sv.getCharacteristics(ACK_UUID)[0]
        self.data_ready = self.sv.getCharacteristics(DATA_READY_UUID)[0]
        while True:
            ack = struct.unpack("B", self.ack.read())[0]
           # if self.robot.waitForNotifications(1):
           #     print("I got it!")
           #     continue
            print("Ack val: {}".format(ack))
            if ack==0:
               ls = float(input("Left Speed: "))
               rs = float(input("Right Speed: "))
               t = float(input("Time (float seconds): "))
               self.ch.write(struct.pack('fff', *[ls, rs, t]))
               ack = struct.unpack("B", self.ack.read())[0]
               while ack != 1:
                   continue
            else:
                data_ready = struct.unpack("B", self.data_ready.read())[0]
                print("Data ready?: {}".format(data_ready))
                if data_ready:
                    print("Waiting...")
                    data = struct.unpack("f" * 30, self.data.read())
                    print(data)
                    self.data_ready.write(struct.pack('B', *[False]))
                else:
                    print("Waiting...")
    def send_command(self, ls, rs, t):
        self.ch.write(struct.pack('fff', *[ls, rs, t]))

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
