import time
import serial
import sys,tty,termios

Direction_car = input("Direction: ")
while ((Direction_car.strip().isalpha()) ):
    Direction_car=input("INVALID: Direction: ")
Direction_car = int(Direction_car)
Direction_car = str(Direction_car)
Distance_x = input("X: ")
while Distance_x.strip().isalpha():
    Distance_x=input("INVALID! X: ")
Distance_x = int(Distance_x)
Distance_x = str(Distance_x)
Distance_y= input("Y: ")
while Distance_y.strip().isalpha():
    Distance_y=input("INVALID! Y: ")
Distance_y = int(Distance_y)
Distance_y = str(Distance_y)
print("Direction: ",Direction_car ," X:",Distance_x ," Y:",Distance_y)

s = serial.Serial(sys.argv[1])

#print(type(Distance_x))
#print(type(Distance_y))
#print(type(Direction_car))

#s.write(str("/RECEIVE/run"+" "+Direction_car+" "+Distance_x+" "+Distance_y+" \n").encode())



class _Getch:
    def __call__(self):
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

def get():
    inkey = _Getch()
    while(1):
        k=inkey()
        if k!='':break
    if k=='\x1b':
        k2 = inkey()
        k3 = inkey()
        if k3=='A':
            print ("up")
            s.write(str("/goStraight/run 100 \n").encode())
        if k3=='B':
            print ("down")
            s.write("/goStraight/run -100 \n".encode())
        if k3=='C':
            print ("right")
            s.write("/turn/run 100 -0.3 \n".encode())
        if k3=='D':
            print ("left")
            s.write("/turn/run 100 0.3 \n".encode())
        time.sleep(1)
        s.write("/stop/run \n".encode())
    elif k.lower() == 'r':
        print("Direction: ",Direction_car ," X:",Distance_x ," Y:",Distance_y)
        #print("REPEAT")
        s.write(str("/RECEIVE/run"+" "+Direction_car+" "+Distance_x+" "+Distance_y+" \n").encode())
    elif k.lower() == 'c':
        print("test")
        s.write(str("/Combine/run \n").encode())
    elif k=='q':
        print ("quit")
        return 0
    else:
        print ("not an arrow key!")
    return 1

if len(sys.argv) < 1:
    print ("No port input")
s = serial.Serial(sys.argv[1])
while get():
    i = 0