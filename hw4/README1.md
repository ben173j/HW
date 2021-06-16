!!!IMPORTANT!!!
Since the google form only received one github link,
Only the link to the first part of HW4 was given, which is here.

The link for HW4_2 is
https://github.com/ben173j/HW/tree/main/hw4_2

and the link for HW4_3 is
https://github.com/ben173j/HW/tree/main/hw4_2

They will have their respective README.md 
Here, only the first part of HW4 will be explained.

To setup the program, make sure that the bbcar_rpc.cpp,
bbcar_rpc.h, bbcar.h, bbcar.cpp are replaced from the 
original bbcar folder.

Remember to use the main.cpp in here and wire XBEE to the
car and a laptop/host.
Now, the program can be flashed to the board.

car_control.py is used to control the car.
It will first prompt the user to enter a valid direction of
the car, and a valid 
x_distance and y_distance of the parking space.

For car_direction, 1 indicates WEST and other numbers
indicate EAST.

X_direction has to be positive

Y_direction can be both positive and negative.
If it is positive, the car park (P) is placed one row above the car(C), like 
the pict below,
//
//          | P |
//   C

if it is negative, it becomes like this
//
//   C
//          | P |


After the correct input has been given, user can enter 'r' from
the keyboard and 
the car will reverse to the correct parking slot
