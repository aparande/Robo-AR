Robo-AR Embedded Systems Code
================

# Code Structure

The code here is structured as an FSM. There are five main files, each of which handle its own part of the FSM. 

- `main.c`: Handles all control loop logic, as well as some setup. 
- `helpers.c` : Is an aggregation of external libraries and utility functions. All constants and sensor related variables are stored here.
- `inputs.c`: Defines structs and functions for getting and oeprating on inputs to the FSM. Is responsible for aggregating data across sensors. 
- `outputs.c`: Defines structs and functions for the outputs of the FSM. Is responsible for actually sending outputs to sensors. 
- `states.c`: Stores all of the logic involved in updating state for the FSM and determining ouputs givne the inputs. 


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

We have also included a Python script that can be used to send waypoints to the Romi. 





