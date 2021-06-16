HW4_3 readme

Like in the first part of HW4, make sure that bbcar_rpc.cpp,
bbcar_rpc.h, bbcar.h, bbcar.cpp are replaced from the 
original bbcar folder.

main.cpp is the main program for the mbed

apriltag.py is used to detect if an OpenMV camera 
is facing perpendicularly to an apriltag

When an apriltag is detected, apriltag.py will send
command to mbed so that the car will rotate to the
right/left depending on the angle.

After the car is perpendicular to the aprilag,
the car will stop rotating.
At this point, the distance between the car and the apriltag
will be measured with PING 
and displayed in the screen of mbed.

