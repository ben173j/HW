#include "mbed.h"
#include "uLCD_4DGL.h"
uLCD_4DGL uLCD(D1, D0, D2);
DigitalIn button1(PA_6);//button1 used for up D12
DigitalIn button2(PA_7);//button2 used for confirm D11
DigitalIn button3(PA_2);//button3 used for down D10
AnalogOut Aout(PA_4);//D7
//AnalogOut Freq1(PA_5);//D13
AnalogIn Ain(A0);
//42,70,98
int Lindex=42;//Used for determining the position of white line
int unit=28; // The unit added/subtracted to Lindex when button is pressed
int sample=100000;
int sam_index=0;
float ADCdata[100000];


void receiveButton();


void Display()
{
    uLCD.cls();
    uLCD.textbackground_color(BLACK);
    uLCD.color(RED);

    uLCD.printf("\nPlease select the frequency:\n"); //Default Green on black text
    uLCD.locate(5,4);
    uLCD.printf("100 Hz");
    uLCD.locate(5,7);
    uLCD.printf("150 Hz");
    uLCD.locate(5,10);
    uLCD.printf("200 Hz");
    uLCD.line(30, Lindex, 80, Lindex, WHITE);
    receiveButton();

}

void freq(int index)
{
    float i;
    int selection=84;//The frequency of wave can be determined from wait_us time
                // Thus, int selection will be the input os wait_us
    uLCD.cls();
    uLCD.color(RED);
    if(index==42){
        uLCD.printf("100Hz is selected");
        selection=84;
    }
    else if(index==70){
        uLCD.printf("150Hz is selected");
        selection=50;
    }
    else{
        uLCD.printf("200Hz is selected");
        selection=30;
    }
    while(sam_index<sample)
    {   
        for(i = 0; i <= 0.9; i=i+0.01){
            Aout = i;
            ADCdata[sam_index]=Ain;
            wait_us(selection);
            if(button2.read()) Display();
            sam_index++;
        }
        
        if(button2.read()) Display();

        for(i = 0.9; i >= 0; i=i-0.1){
            Aout = i;
            ADCdata[sam_index]=Ain;
            wait_us(selection);
            if(button2.read()) Display();
            sam_index++;
        }  
    }
    
    for (int k = 0; k < sample; k++){
    printf("%f\r\n", ADCdata[k]);
    wait_us(100);
  } 
}
void moveLine(int Nindex,int Pindex)
{
    uLCD.line(30,Pindex,80,Pindex,BLACK);//This is to make the previous words to appear black
    uLCD.line(30,Nindex,80,Nindex,WHITE);//This is to make the current words appear white
}

void receiveButton()
{
    while(1){
    int Pindex;
     if(button1.read()==1){
            ThisThread :: sleep_for(200ms); // Without this, the selection will
                                            //move very fast when we push the button
            Pindex=Lindex;
            Lindex= Lindex - unit;
            if(Lindex<42){
                Lindex = 98;
            } 
            moveLine(Lindex,Pindex);
            //printf("button1 is pressed!\r\n");
            
        }
        if(button2.read()==1){
            ThisThread :: sleep_for(200ms);
            //printf("button2 is pressed!\r\n");
            freq(Lindex);
            
        }
        if(button3.read()==1){
            ThisThread :: sleep_for(200ms);
            Pindex=Lindex;
            Lindex=Lindex+unit;
            if(Lindex>98) Lindex =42;
            moveLine(Lindex,Pindex);
           // printf("button3 is pressed!\r\n");
            

        }
    }
}




int main()
{

    while(1){
        //uLCD.textbackground_color(BLACK);       
        Display();
    
    }
}