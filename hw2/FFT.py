import matplotlib.pyplot as plt
import numpy as np
import serial
import time

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev)

frequenc_f = s.readline()
Fs = float(frequenc_f)
#Fs=10000
Ts = 1.0/Fs; # sampling interval
t = np.arange(0,0.01,Ts) # time vector; create Fs samples between 0 and 1.0 sec.
y = np.arange(0,0.01,Ts) 

n = len(y) # length of the signal
#print(n)
k = np.arange(n)
T = n/Fs
frq = k/T # a vector of frequencies; two sides frequency range
frq = frq[range(int(n/2))] # one side frequency range


for x in range(0, n):
    line=s.readline() # Read an echo string from B_L4S5I_IOT01A terminated with '\n'
    # print line
    if(float(line)==Fs):
        continue
    y[x] = float(line)

Y = np.fft.fft(y)/n*2 # fft computing and normalization
Y = Y[range(int(n/2))] # remove the conjugate frequency parts

fig, ax = plt.subplots(2, 1)
ax[0].plot(t,y)
ax[0].set_xlabel('Time')
ax[0].set_ylabel('Amplitude')

#alist = list(frq)
#alist = alist[:1000]
#print(frq.len(),"\n")
#print(frq.ndim)
#print(frq[:100])
#print(frq.shape)
frq = frq[:15]
Y=Y[:15]
#frq = tuple(alist)
ax[1].plot(frq,abs(Y),'r') # plotting the spectrum
ax[1].set_xlabel('Freq (Hz)')
ax[1].set_ylabel('|Y(freq)|')
plt.show()
s.close()
