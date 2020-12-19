#!/usr/bin/env python3
import struct
import time
import numpy as np
import pandas as pd
from getpass import getpass
from bluepy.btle import Peripheral, DefaultDelegate

addr = 'C0:98:E5:51:EE:C5'
if len(addr) != 17:
    raise ValueError("Invalid address supplied")
    
# Define UUIDs for BLE connection
SERVICE_UUID = "4607eda0-f65e-4d59-a9ff-84420d87a4ca" # Romi BLE service
DRIVE_COMMAND_UUID = "4607eda1-f65e-4d59-a9ff-84420d87a4ca"# Romi drive command
DATA_UUID = "4607eda2-f65e-4d59-a9ff-84420d87a4ca"# Data recieved from Romi
ACK_UUID = "4607eda3-f65e-4d59-a9ff-84420d87a4ca"# Acknowledgement from Romi
DATA_READY_UUID = "4607eda4-f65e-4d59-a9ff-84420d87a4ca"# Data ready to be read from Romi

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

        # get service from robot
        # get characteristic handles from service/robot
        self.sv = self.robot.getServiceByUUID(SERVICE_UUID)
        self.drive_command = self.sv.getCharacteristics(CHAR_UUID)[0]
        self.data = self.sv.getCharacteristics(DATA_UUID)[0]
        self.ack = self.sv.getCharacteristics(ACK_UUID)[0]
        self.data_ready = self.sv.getCharacteristics(DATA_READY_UUID)[0]
        self.data_list = []
        
        # Set trial parameters
        ls = float(input("Left Speed: "))
        rs = float(input("Right Speed: "))
        t = float(input("Time (float seconds): "))
        name = "l_{}_r_{}".format(int(ls), int(rs))
        n = int(input("Num trials: "))
        
        for i in range(n):
            while True:
                ack = struct.unpack("B", self.ack.read())[0]
                # Send command when the robot is ready
                if ack==0:
                   if self.data_list:
                       self.write_data(ls, rs, "{}_{}".format(name, i))
                       self.data_list = []
                       break
                   input("Ready?") 
                   self.send_command(ls, rs, t);
                   ack = struct.unpack("B", self.ack.read())[0]
                
                   # Wait until robot acknowledges before proceding
                   while ack != 1:
                       continue
                else:
                    data_ready = struct.unpack("B", self.data_ready.read())[0]
                    # Read data when the robot is ready
                    if data_ready:
                        data = struct.unpack("f" * 30, self.data.read())
                        self.data_list.extend(data)
                        self.data_ready.write(struct.pack('B', *[False]))
                    else:
                        print("Waiting...")

    def send_command(self, ls, rs, t):
        # Tell Romi to drive specified left and right speeds for set amount of time
        self.ch.write(struct.pack('fff', *[ls, rs, t]))

    def write_data(self, ls, rs, name):
        # Write data to CSV file
        
        left_input = [ls] * (len(self.data_list)//3)
        right_input = [rs] * (len(self.data_list)//3)
        left_dists = self.data_list[::3]
        right_dists = self.data_list[1::3]
        times = self.data_list[2::3]
        header = ["left_input", "right_input", "left_distance", "right_distance", "time"]
        all_data = [left_input, right_input, left_dists, right_dists, times]
        df = pd.DataFrame(all_data).transpose()
        df.columns = header
        print(df)
        df.to_csv("data/{}.csv".format(name), index=False)

    def __enter__(self):
        return self
    def __exit__(self, exc_type, exc_value, traceback):
        self.robot.disconnect()

with RobotController(addr) as robot:
    getpass('Input waypoints for car')
