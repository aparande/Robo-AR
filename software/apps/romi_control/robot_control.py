#!/usr/bin/env python3
import struct
import time
from getpass import getpass
from bluepy.btle import Peripheral, DefaultDelegate
import binascii

parser = argparse.ArgumentParser(description='Print advertisement data from a BLE device')
# BLE Address of the Romi. Change if your Romi has a different address
addr = "c0:98:e5:46:ee:c5"
if len(addr) != 17:
    raise ValueError("Invalid address supplied")

# Define relevant UUIDs for the Romi
SERVICE_UUID = "4607eda0-f65e-4d59-a9ff-84420d87a4ca"
WAYPOINT_CMD_UUID = "4607eda1-f65e-4d59-a9ff-84420d87a4ca"
ACK_UUID = "4607eda2-f65e-4d59-a9ff-84420d87a4ca"

class RobotController():

    def __init__(self, address):

        # Connect to the Romi over BLE
        self.robot = Peripheral(addr)
        print("connected")

        # Get service from robot
        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)
        # Get characteristic handles from service/robot
        self.waypoint_cmd = self.sv.getCharacteristics(WAYPOINT_CMD_UUID)[0]
        self.ack = self.sv.getCharacteristics(ACK_UUID)[0]

        while True:
            # Check to see if the robot is ready to recieve a command
            ack, = struct.unpack("i", self.ack.read())
            if ack==0:
                # The robot is ready to recieve a command
                # Get command as input from the user
                r = float(input("Distance (m): "))
                t = float(input("Angle (degrees): "))
                # Write to the appropriate characteristic
                # The waypoint command is a sequence of two floats, representing a polar coordiantes format
                self.waypoint_cmd.write(struct.pack('ff', *[r, t]));
                # Tell the robot that we want it to start executing our command
                self.ack.write(struct.pack('Bxxx', *[ack+1]))
                # Give the robot some time to update the characteristic value before we read it again
                time.sleep(1)

    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr) as robot:
    getpass('Input waypoints for car')
