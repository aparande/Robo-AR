Robo-AR System ID Code
================

# Code Structure

The code here is structured as an FSM. There are __ main files, each of which handle its own part of the FSM. 


# How To Run

To run this code, you need to run 

> `make flash` 

Which will flash the code to the robot. 

For compilation, simply running: 

> `make` 

Is good enough. 

For debugging, writing print statements and running: 

> `make rtt` 

will allow you to view print statements directly to the computer. Note the robot has to be plugged in for this to work.


# Testing

We have also included a Python script `robot_control.py` that can be used to collect system-id data from the Romi. Before running this script, make sure that your computer has BLE capability and that you have the [Bluepy](https://github.com/IanHarvey/bluepy) python package installed. To use the script, turn on your Romi and run

> `sudo ./robot_control.py`

The script should now do something, but only Jay knows what that is.



