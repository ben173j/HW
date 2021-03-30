There are two files written with different code languages( c++ and python) in this directory. Having both compilers are compulsory to succesfully run the programs.

main.cpp program essentially produces an analog signal, receive it back and sample it to the digital ones. User can choose which frequency(displayed through uLCD) to be produced. 

FTT.py sketches the ampltude vs time wave and FFT vs Freq wvae from the sampled points of the analog signal.

Note: Codes are explained in the main.cpp file. Only the setup and results will be explained here.

The .cpp file will first be explained.

To setup the program, user has to first make sure that all the pins of the mbed
board are connected properly according to the variable declarations in main.cpp. 
Please make sure that the mbed and uLCD-144G2-AR libraries exist in the same folder.
Then, user can compile and run the file by typing 
"sudo mbed compile --source . --source ~/ee2405/mbed-os-build/ -m B_L4S5I_IOT01A -t GCC_ARM -f"
in linux terminal.

There are 3 bottons and 1 uLCD used in this project.
The middle button(connected to D10) is used to confirm selection / reselect the frequency of interest. The other two are for moving up and down.
When the program starts, the uLCD will prompt the user to select which frequency of signal to be produced. After user selects the frequency of interest, the uLCD will tell the user which frequency has been 'selected'.
If user wants to reselect the frequency, user can press the middle button again and the uLCD will reprompt the user.

The .py file will now be explained.
To run the .py file, make sure that the screen mode is clear. This can be done by checking "sudo screen /dev/ttyACM*" in linux terminal (* represents num of ttyACM). Then, user can run the .py file by typing "sudo python3 FFT.py" in the linux terminal.
After typing the command, user can now select the frequency of interest through the button.
A graph will appear after a short time.
