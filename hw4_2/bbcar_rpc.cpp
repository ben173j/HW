#include "bbcar_rpc.h"
#include <stdlib.h>
RPCFunction rpcStop(&RPC_stop, "stop");
RPCFunction rpcCtrl(&RPC_goStraight, "goStraight");
RPCFunction rpcTurn(&RPC_turn, "turn");
RPCFunction rpcReceive(&RPC_receive_LENGTH,"RECEIVE");
RPCFunction rpcCombine(&RPC_COMBINATION,"Combine");
//RPCFunction rpcApriltag(&RPC_apriltag,"TAG");



extern BBCar car;
extern int CAR_DIRECTION ;
extern int DISTANCE_X;
extern int DISTANCE_Y;
extern DigitalOut led1;
float DISTANCE_OFFSET_X;
float DISTANCE_OFFSET_Y;
char* car_dir ;

extern BufferedSerial xbee;
extern DigitalInOut PING;
extern BufferedSerial PC_OPENCV; 
extern BufferedSerial UART_OPENCV;
extern DigitalOut led1;

EventQueue queue(32 * EVENTS_EVENT_SIZE);
EventQueue queue2(32 * EVENTS_EVENT_SIZE);
EventQueue queue3(32 * EVENTS_EVENT_SIZE);

Thread t1,t2,t3;

extern parallax_ping  ping1;
int control_DP = 0;



int OPEN_MV_DETECT()
{
    car.goStraight(30);
    UART_OPENCV.set_baud(9600);
    while(1){
      
      if(UART_OPENCV.readable()){
            
            char recv[1];
            UART_OPENCV.read(recv, sizeof(recv));
            //if(recv[0]=='r') printf("RIGHT\n\r");
            //else if(recv[0]=='l') printf("LEFT\n\r");
            //else printf("STRAIGHT\n\r");
            printf("%c\n\r",recv[0]);
            if(recv[0]=='l'){
                printf("ENTER LEFT\n\r");
                car.turn(50,0.3);
                //ThisThread :: sleep_for(200ms);
                //while(recv[0]=='l');
                    
            }
            else if(recv[0] == 'r'){
                printf("ENTER RIGHT\n\r");
                car.turn(34,-0.3);
                //ThisThread :: sleep_for(150ms);
                //
                while(recv[0]=='r');
                
            }
            else{
                printf("ENTER STRAIGHT\n\r");
                car.goStraight(30);
                //ThisThread :: sleep_for(100ms);

            } 


            
            //PC_OPENCV.write(recv, sizeof(recv));
            
            
      }

   }
   return 1;
}


int  DISTANCE_PING()
{
   //EventQueue queue;
   //queue.call_every(1s,printf,"PINGI: %1.2f \n\r",(float)ping1); 
   while(1) {
       ThisThread:: sleep_for(1s);
       printf("PING: %1.2f\r\n",(float)ping1);
        if((float)ping1<10){
            control_DP=1;
            break;
        }
   }
   printf("LAST PING: %1.2f \r\n",(float)ping1);
   printf("control_DP= %d\r\n",control_DP);
   car.stop();

   return 1;
}

/*
void RPC_apriltag(Arguments* in, Reply* out)
{
  
    UART_OPENCV.set_baud(9600);
    int tag_ID;
    char buff[5];
    while(1){
      
      if(UART_OPENCV.readable()){
            
            char recv[1];
            UART_OPENCV.read(recv, sizeof(recv));
            
            int i=0;
            while(recv[0]!='\n'){
                buff[i]=recv[0];
                i++;
            }
            i=0;
            if(buff[i].isdigit()){
                tag_ID = atoi(buff);
                printf("Tag ID: %d\n\r",tag_ID);
            }
            else{
                pritnf("buff: %s\n\r",buff);
            } 
            

            
            //PC_OPENCV.write(recv, sizeof(recv));
            
            
      }

   }
   return 1;
}
*/

void RPC_COMBINATION(Arguments* in, Reply* out)
{
    //t1.start(callback(&queue, &EventQueue::dispatch_forever));
    //queue.call(DISTANCE_PING);
    
    t2.start(callback(&queue2, &EventQueue::dispatch_forever));
    queue2.call(OPEN_MV_DETECT);

    car.goStraight(70);
}


void RPC_receive_LENGTH(Arguments* in, Reply* out)
{
    
    CAR_DIRECTION = in->getArg<int>();
    DISTANCE_X = in->getArg<int>();
    DISTANCE_Y = in->getArg<int>();
    
    DISTANCE_OFFSET_X = DISTANCE_X/5;
    DISTANCE_OFFSET_Y = DISTANCE_Y/5;

    if(CAR_DIRECTION==1) car_dir= "WEST";
    else car_dir = "EAST";

    printf("DIRECTION: %s, X: %d, Y: %d \n\r",car_dir,DISTANCE_X,DISTANCE_Y);
    printf("DISTANCE_OFFSET(X,Y) = (%1.2f ,%1.2f)",DISTANCE_OFFSET_X,DISTANCE_OFFSET_Y);
    //printf("(RPC_receive) Distance: %1.2f\n\r\n\r",car.ENCODER.get_cm());
    car.ENCODER.reset();
    while(car.ENCODER.get_cm() < DISTANCE_OFFSET_X){
        if(car.ENCODER.get_cm() > DISTANCE_OFFSET_X){
            car.stop();
            break;
        }
        car.goStraight(-95);
        if(car.ENCODER.get_cm() > DISTANCE_OFFSET_X){
            car.stop();
            break;
        } 
    }
    car.stop();
    //printf("Steps: \r\n", car.ENCODER.get_steps());
    //printf("Distance_X: %1.2f \r\n",car.ENCODER.get_cm());
    //printf("FINISH\n\r");
    
    if(CAR_DIRECTION==1 && DISTANCE_OFFSET_Y>=0){
        car.reverseturn(100, -0.3); // reverse right
        ThisThread :: sleep_for(2770ms);
        car.stop();    
    
        car.ENCODER.reset();
        while(car.ENCODER.get_cm() < DISTANCE_OFFSET_Y){
            if(car.ENCODER.get_cm() > DISTANCE_OFFSET_Y){
                car.stop();
                break;
            }
            car.goStraight(-95);
            if(car.ENCODER.get_cm() > DISTANCE_OFFSET_Y){
                car.stop();
                break;
            } 
        }
    }

    //printf("Distance_Y: %1.2f \r\n",car.ENCODER.get_cm());
    

    //else if(car_dir =="EAST" && DISTANCE_OFFSET_Y>=0){
    //    
    //}
    
    car.stop();
    
}

void RPC_stop (Arguments *in, Reply *out)   {
    car.stop();
    return;
}

void RPC_goStraight (Arguments *in, Reply *out)   {
    int speed = in->getArg<double>();
    car.goStraight(speed);
    //printf("(RPC) Distance: %1.2f\n\r",car.ENCODER.get_cm());
    printf("(RPC) steps: %d\n\r",car.ENCODER.get_steps());
    printf("distance: %1.2f\n\r",car.ENCODER.get_cm());
    
    return;
}

void RPC_turn (Arguments *in, Reply *out)   {
    int speed = in->getArg<double>();
    double turn = in->getArg<double>();
    car.turn(speed,turn);
    return;
}
