HW4_2 readme

Like in the first part of HW4, make sure that bbcar_rpc.cpp,
bbcar_rpc.h, bbcar.h, bbcar.cpp are replaced from the 
original bbcar folder.

main.cpp is the main program for the mbed

openmv.py is used for line detection in openmv camera.

The car will follow a black line. It is advisable to use black tape
to make the line detection works smoother.

The car will turn to the left or right depending on the detected angle 
in openmv.

main.cpp only receives command from openmv and make a 
correct logical decision in driving the car.

Hence, after main.cpp has been flashed to the board and
openmv.py has been flashed to the OpenMV,
the car will start following a black line and if there is no
black line detected, the car will not move