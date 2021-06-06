#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

Ticker servo_ticker;
Ticker en_ticker;
Ticker display_ticker;
DigitalIn encoder_in(D6);
PwmOut pin5(D10), pin6(D11);
BufferedSerial xbee(D1, D0);

EventQueue queue;
//parallax_encoder ENCODER( DigitalIn& input, Ticker &encoder_ticker );

//BBCar car(pin5, pin6, servo_ticker);
BBCar car(pin5, pin6, servo_ticker, encoder_in, en_ticker);

volatile int steps;
volatile int last;


void encoder_control() {
   int value = encoder_in;
   if (!last && value) steps++;
   last = value;
}

void display()
{
   printf("steps: %d\r\n",steps);
}

int main() {
   
   char buf[256], outbuf[256];
   FILE *devin = fdopen(&xbee, "r");
   FILE *devout = fdopen(&xbee, "w");
   //en_ticker.attach(&encoder_control, .01);
   //display_ticker.attach(queue.event(&display),500ms);
   while (1) {

      memset(buf, 0, 256);
      for( int i = 0; ; i++ ) {
         char recv = fgetc(devin);
         if(recv == '\n') {
            printf("\r\n");
            break;
         }
         buf[i] = fputc(recv, devout);
      }
   RPC::call(buf, outbuf);
   }
}


