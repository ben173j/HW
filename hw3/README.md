Note: Only the setup and result of the codes will be explained here.
      The explanation of codes are in the .py and .cpp files 

SETUP:

To setup the codes, make sure that the following libraries have been 
imported in the same directory:
- tensorflowlite (mbed add https://gitlab.larc-nthu.net/ee2405_2021/         tensorflowlite_mbed)
- uLCD 
- accelerometer
- MQTT
- etc
(details are in main.cpp file in https://github.com/ben173j/HW/blob/main/hw3/model_deploy/main.cpp )

python3 and gcc compilers are necessary to sucessfully run the programs.
There are some changes in the config.h in this project (a little bit different
from the one given in lab)
model.cpp contains the coordinates recorded during training.
mbed_app.json needs to be reconfigured when the IP address changes.



FUNCTION AND RESULT:

There is a mqtt_client.py file in https://github.com/ben173j/HW/tree/main/hw3 which is used as a client in this project.

USER has to first run the main.cpp file and check in sudo screen/dev/ttyACM*
that "Sucessfully Connected" is printed in screen.

Then, user can run sudo python3 mqtt_client.py to allow python to connect to the server.
If succesful, the Python will send a RPC serial command to mbed to enter into UI mode. 

main.cpp file is in the model_deploy folder (https://github.com/ben173j/HW/blob/main/hw3/model_deploy/main.cpp).

main.cpp file does two main jobs. 
1. Display UI interface. User is asked to select threshold angle of interest.
   This selection will be sent through MQTT to python/PC. PYthon/PC will then send a RPC serial command to mbed to enter into angle detection mode. (some LEDs will light)

2. In angle detection mode, some LEDs will light. The uLCD will keep on
   showing "INITIALIZING" if the board is not correctly placed on a table.
   When it is placed correctly on a table, its current coordinate will be
   taken as the reference vectors(This is measured using accelerometer).
   User can now tilt the board. (Different LEDs effect will begin) uLCD and screen will let the user know if the tilted angle is greater or smaller than the threshold angle. If tilted angle > threshold angle, this angle will be sent to client(pthon/PC) through MQTT. It will not be sent if the angle is smaller. User will have around 1s in each tilting event. After 14 angles have been recorded, all the measured angles will be sent to client(python/PC) through MQTT and FINISH will be shown in uLCD and screen.

   Note: different variation of LEDs will light on each phase

