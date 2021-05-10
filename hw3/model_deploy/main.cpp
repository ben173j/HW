//GENERAL
#include "mbed.h"
#include <cmath>
#include <string.h>

//FOR ACCELEROMETER
#include "accelerometer_handler.h"
#include "stm32l475e_iot01_accelero.h"

//FOR uLCD
#include "uLCD_4DGL.h"

//FOR MQTT
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

//FOR NN
#include "config.h"
#include "magic_wand_model_data.h"
#include "mbed_rpc.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"


//GENERAL
uLCD_4DGL uLCD(D1, D0, D2);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

InterruptIn button(USER_BUTTON);
//DigitalIn button(USER_BUTTON);

//For RPC
BufferedSerial pc(USBTX, USBRX);
void UI(Arguments *in, Reply *out);
void Angle(Arguments *in, Reply *out);
RPCFunction rpcUI(&UI, "UI");
RPCFunction rpcAngle(&Angle, "Angle");
double x, y;

//FOR Threshold Angle and storing angles
int ThresholdAngle=30;//Used to store the selected angle
float ANGLE_COLLECTION[14];



//for moving lines in uLCD
int GIndex=0; //used to track the selection of user
int Lindex=42;//Used for determining the position of white line
int unit=28; // The unit added/subtracted to Lindex when we move the line


//for accelerometer; the coordinates of board
int16_t initDataXYZ[3] = {0};
int idR[32] = {0};
int indexR = 0;

//FOR CALLING RPC
EventQueue RPCqueue(32 * EVENTS_EVENT_SIZE);
Thread RPCthread;

//FOR handling UI AND ANGLE THREADS
EventQueue queue(32 * EVENTS_EVENT_SIZE);
EventQueue queue2(32 * EVENTS_EVENT_SIZE);
Thread t1,t2,t3;


//FOR MQTT AND WIFI
// GLOBAL VARIABLES
WiFiInterface *wifi;
volatile int message_num = 0;
volatile int arrivedcount = 0;
volatile bool closed = false;
const char* topic = "Mbed";
Thread mqtt_thread(osPriorityHigh);
EventQueue mqtt_queue;

Thread mqtt_threadPublish(osPriorityHigh);
EventQueue mqtt_queuePublish;

//EventQueue PUBLISH_ANGLE_queue;

//FOR FLOW_CONTROL; 1 means allowed to execute, 0 means not allowed
bool moveline_enable = 1; // to control void moveLine(int,int)
bool model_enable=1;//to control int Model(void)
bool decision_enable=1;//to control Decision
bool publish_enable=1; //to control Publish
//FOR NN
// Create an area of memory to use for input, output, and intermediate arrays.
// The size of this will depend on the model you're using, and may need to be
// determined by experimentation.
constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

//Config datatype is used to represent the gesture trained before
Config config;

//global client used to store the client address
MQTT::Client<MQTTNetwork,Countdown>* CLIENT;


/*
  ALL THE SUBROUTINES IN THIS .cpp
*/
//USED TO PREDICT THE MODEL
int Model(void);
//Used to measure the angle of inclination 
int angle_detection(void);
//USED for display in uLCD
int Display(void);
//Used to move line in uLCD
void moveLine(int Nindex,int Pindex);
//Used to determine the direction of line movement in uLCD
void receive_shape(int Gindex);

//Used to send threshold angle to client when USER_BUTTON is pressed
void Decision(MQTT::Client<MQTTNetwork, Countdown>* client);

//Used to publish the angle when measured angle> threshold angle and publish
//all the angles when the preset tilting numbers have been reached
void PUBLISH_ANGLE(MQTT::Client<MQTTNetwork, Countdown>* client,float angle,int finish,int success);

//used to call RPC thread
void calling_RPC(); 

//UI is for interface, when /UI/run is entered, thread UI will be called
void UI (Arguments *in, Reply *out);   
//Angle is for detecting the angle of inclination, when /Angle/run is entered, thread Angle will be called
void Angle (Arguments *in, Reply *out);
//for closing mqtt
void close_mqtt();



// Return the result of the last prediction
int PredictGesture(float* output) {
  // How many times the most recent gesture has been matched in a row
  static int continuous_count = 0;
  // The result of the last prediction
  static int last_predict = -1;

  // Find whichever output has a probability > 0.8 (they sum to 1)
  int this_predict = -1;
  for (int i = 0; i < label_num; i++) {
    if (output[i] > 0.8) this_predict = i;
  }

  // No gesture was detected above the threshold
  if (this_predict == -1) {
    continuous_count = 0;
    last_predict = label_num;
    return label_num;
  }

  if (last_predict == this_predict) {
    continuous_count += 1;
  } else {
    continuous_count = 0;
  }
  last_predict = this_predict;

  // If we haven't yet had enough consecutive matches for this gesture,
  // report a negative result
  if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {
    return label_num;
  }
  // Otherwise, we've seen a positive result, so clear all our variables
  // and report it
  continuous_count = 0;
  last_predict = -1;

  return this_predict;
}

int Display()
{
    model_enable=1;
    uLCD.cls();//cls is to clear all the texts with the background color
    uLCD.textbackground_color(BLACK);
    uLCD.color(RED); //RED as text color

    uLCD.printf("\nPlease select the tilt angle:\n"); //Default Green on black text
    uLCD.locate(5,4);//position of the text
    uLCD.printf("30");
    uLCD.locate(5,7);
    uLCD.printf("40");
    uLCD.locate(5,10);
    uLCD.printf("50");
    uLCD.locate(5,13);
    uLCD.printf("60");
    
    uLCD.line(30, Lindex, 80, Lindex, WHITE);
    return Model();

}

void moveLine(int Nindex,int Pindex)
{
  if(moveline_enable==1){
    uLCD.line(30,Pindex,80,Pindex,BLACK);//This is to make the previous texts to appear black
                                        //The purpose is to make the previous texts invisible to the user
    uLCD.line(30,Nindex,80,Nindex,WHITE);//This is to make the current texts to appear white and allow user to see
  }
  else;
}

void Decision(MQTT::Client<MQTTNetwork, Countdown>* client)
{

   printf("Lindex: %d\n\r",Lindex);
   moveline_enable=0;
   model_enable=0;
   if(decision_enable==1){
      uLCD.cls();//cls is to clear all the texts with the background color
      uLCD.color(RED); //RED as text color
      //Lindex is the location of the white line. by knowing the location of the white line, we can know the corresponding thresholdAngle of intrest
      if(Lindex==42)ThresholdAngle=30; 
      else if(Lindex==70)ThresholdAngle=40;
      else if(Lindex==98)ThresholdAngle=50;
      else ThresholdAngle=60;

      uLCD.printf("\n %d degree was chosen\n",ThresholdAngle);
      

      message_num++;
      MQTT::Message message;
      char buff[100];
      sprintf(buff, "\r\nSelected Angle: %d\r\n", ThresholdAngle);
      message.qos = MQTT::QOS0;
      message.retained = false;
      message.dup = false;
      message.payload = (void*) buff;
      message.payloadlen = strlen(buff) + 1;
      int rc = client->publish(topic, message);//this is used to publish message to client; rc=0 means successful transmission of dataa, rc=-1 means fail
      //ThisThread :: sleep_for(1s);

      printf("rc:  %d\r\n", rc);
      printf("Publish message: %s\n\r", buff);
   
   }
   else return;
   
   // printf("\n\rrc:  %d\r\n", rc);
   // printf("\n\rPuslish message: %s\r\n", buff);
   //angle_detection();
   //return 1; // return 1 if successful
  

}

//accumulation for ANGLE_FOR_PYTHON
char ANGLE_FOR_PYTHON[1000]="\0";

void PUBLISH_ANGLE(MQTT::Client<MQTTNetwork, Countdown>* client,float angle,int finish,int success)
{
    if(publish_enable==1){

        int rc=0;
        message_num++;
        MQTT::Message message;
        char buff[100];
        if(finish==0){
            if(success==0){
                sprintf(buff,"FAIL! angle: %1.1f\r\n,angle");
                strcat(ANGLE_FOR_PYTHON,buff);
                return;     
            }
            else{
              sprintf(buff, "PASS! angle!: %1.1f\r\n", angle);
            }
            
            strcat(ANGLE_FOR_PYTHON,buff);
            message.qos = MQTT::QOS0;
            message.retained = false;
            message.dup = false;
            
              message.payload = (void*) buff;
              message.payloadlen = strlen(buff) + 1;
              rc = client->publish(topic, message);
          }
        else if(finish==1){
           for(int i=0;i<14;i++){
            sprintf(buff, "Measured Angle: %1.1f\r\n", ANGLE_COLLECTION[i]);
            message.qos = MQTT::QOS0;
            message.retained = false;
            message.dup = false;
            message.payload = (void*) buff;
            message.payloadlen = strlen(buff) + 1;
            rc = client->publish(topic, message);
            ThisThread :: sleep_for(200ms);
           }
            sprintf(buff, "-----FINISH-----");
            message.qos = MQTT::QOS0;
            message.retained = false;
            message.dup = false;
            message.payload = (void*) buff;
            message.payloadlen = strlen(buff) + 1;
            rc = client->publish(topic, message);
            ThisThread :: sleep_for(200ms);         
            
        }
        //ThisThread :: sleep_for(1s);

        printf("rc:  %d\r\n", rc);
        printf("Publish message: %s\n\r", buff);
      }
    else return;
   /*
   if(finish==0){
     printf("%s",buff);
   }
   else{
     for(int i=0;i<14;i++){
       printf("Angle tilted:%1.1f\n\r",ANGLE_COLLECTION[i]);
     }
   }
   */
}



void receive_shape(int Gindex)
{
 
  
  int Pindex=Lindex;
  
  if(Gindex==0){
    Lindex= Lindex - unit;
    if(Lindex>=42){
        moveLine(Lindex,Pindex);
    } 
    else{
      Lindex=42;
      uLCD.line(30,Lindex,80,Lindex,WHITE);
    }
  }
  else{
    Lindex = Lindex+unit;
    if(Lindex<=126){
      moveLine(Lindex,Pindex);
    }
    else{
      Lindex=126;
      uLCD.line(30,Lindex,80,Lindex,WHITE);
    }
  }
  
  
}


int Model() {

  //button.mode(PullNone);
  // Whether we should clear the buffer next time we fetch data
  moveline_enable=1;
  bool should_clear_buffer = false;
  bool got_data = false;

  // The gesture index of the prediction
  int gesture_index;

  // Set up logging.
  static tflite::MicroErrorReporter micro_error_reporter;
  tflite::ErrorReporter* error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return -1;
  }

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  static tflite::MicroOpResolver<6> micro_op_resolver;
  micro_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
      tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                               tflite::ops::micro::Register_MAX_POOL_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                               tflite::ops::micro::Register_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                               tflite::ops::micro::Register_FULLY_CONNECTED());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                               tflite::ops::micro::Register_SOFTMAX());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE,
                               tflite::ops::micro::Register_RESHAPE(), 1);

  // Build an interpreter to run the model with
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  tflite::MicroInterpreter* interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
  interpreter->AllocateTensors();

  // Obtain pointer to the model's input tensor
  TfLiteTensor* model_input = interpreter->input(0);
  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != config.seq_length) ||
      (model_input->dims->data[2] != kChannelNumber) ||
      (model_input->type != kTfLiteFloat32)) {
    error_reporter->Report("Bad input tensor parameters in model");
    return -1;
  }

  int input_length = model_input->bytes / sizeof(float);

  TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
  if (setup_status != kTfLiteOk) {
    error_reporter->Report("Set up failed\n");
    return -1;
  }

  error_reporter->Report("Set up successful...\n");

  
  //t2.start(callback(&queue2, &EventQueue::dispatch_forever));
  while (model_enable==1) {
    led3=!led3;
    // Attempt to read new data from the accelerometer
    got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                 input_length, should_clear_buffer);

    // If there was no new data,
    // don't try to clear the buffer again and wait until next time
    if (!got_data) {
      should_clear_buffer = false;
      continue;
    }

    // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      error_reporter->Report("Invoke failed on index: %d\n", begin_index);
      continue;
    }
    
    // Analyze the results to obtain a prediction
    //gesture_indesx is the array index in comfig.h (telling us right or left)
    if(moveline_enable==1)
    gesture_index = PredictGesture(interpreter->output(0)->data.f);
    //3 refers to no movement defined by us
    //button.rise(queue2.event(Decision,Lindex));
    if(gesture_index!=3){
      GIndex = gesture_index;
      receive_shape(GIndex); 
    } 
    //button.fall(queue2.event(&Decision,&client,Lindex));
    //if(button.read()==0)break;
     if(model_enable==0) break; 
    
    //ThisThread::sleep_for(1s);
    // Clear the buffer next time we read data
    should_clear_buffer = gesture_index < label_num;

    // Produce an output
    if (gesture_index < label_num) {
      error_reporter->Report(config.output_message[gesture_index]);
    }
  }
  return 1; //return 1 if successful
  
}


/*
int main()
{
  Display();
}
/*/

void calling_RPC() {
    //The mbed RPC classes are now wrapped to create an RPC enabled version - see RpcClasses.h so don't add to base class

    // receive commands, and send back the responses
    char buf[256], outbuf[256];

    FILE *devin = fdopen(&pc, "r");
    FILE *devout = fdopen(&pc, "w");

    while(1) {
        memset(buf, 0, 256);
        for (int i = 0; ; i++) {
            char recv = fgetc(devin);
            if (recv == '\n') {
                printf("\r\n");
                break;
            }
            buf[i] = fputc(recv, devout);
        }
        //Call the static call method on the RPC class
        RPC::call(buf, outbuf);
        printf("%s\r\n", outbuf);
    }
}
/*
int main()
{
  calling_RPC();
}
*/
void UI (Arguments *in, Reply *out)   {
    bool success = true;
    decision_enable=1;
    //char buffer[200], outbuf[256];
    //char strings[20];
    
    t1.start(callback(&queue, &EventQueue::dispatch_forever));
    queue.call(Display);
}

int angle_detection()
{
  decision_enable=0;
  //printf("\r\n----INITIALIZING----\r\n");
  BSP_ACCELERO_Init();
  BSP_ACCELERO_AccGetXYZ(initDataXYZ);
  led3=!led3;
  uLCD.cls();
  uLCD.printf("---INITIALIZING---");
  //printf("")
  int x , y , z;
  x = initDataXYZ[0];
  y = initDataXYZ[1];
  z = initDataXYZ[2];
  
  int ori_x=4, ori_y=-3 , ori_z= 1002;
  //D_x,D_y,D_z are used to check if the board is in the correct position eg:ON the table
  int ref_x=0,ref_y=0,ref_z=0;
  //while(1){
   
   int D_x = x-ori_x;
   int D_y = y-ori_y;
   int D_z = z-ori_z;
  //printf("\r\n%d, %d, %d\n", initDataXYZ[0], initDataXYZ[1], initDataXYZ[2]);
  //printf("\r\n%d, %d, %d\n", D_x, D_y, D_z);
  ThisThread::sleep_for(1s);
  //}
  printf("SUCCESFULLY CONNECTED!\n\r");
  
  while(abs(D_z)>10)
    {
      led1=!led1;
      led2=!led2;
      led3=!led3;
      BSP_ACCELERO_AccGetXYZ(initDataXYZ);
      //printf("%d, %d, %d\n", initDataXYZ[0], initDataXYZ[1], initDataXYZ[2]);
      //printf("\r----INITIALIZING----\r\n");
      //uLCD.cls();
      //uLCD.printf("---INITIALIZING---");
      
      x = initDataXYZ[0];
      y = initDataXYZ[1];
      z = initDataXYZ[2];
      printf("From initializing: %d, %d, %d\n\r", x, y, z);
      D_z = z-ori_z;
      /*
      if(!(D_x>7 && D_x<-7 &&
        D_y>7 && D_y<-7 &&
        D_z>7 && D_y<-7))
        {
          ref_x=initDataXYZ[0];
          ref_y=initDataXYZ[1];
          ref_z=initDataXYZ[2];
        }
        //uLCD.printf()
        */
      ThisThread::sleep_for(500ms);
    }
  ref_x=initDataXYZ[0];
  ref_y=initDataXYZ[1];
  ref_z=initDataXYZ[2];
  printf("\rReference vectors (x,y,z): (%d, %d, %d)\n\r",ref_x,ref_y,ref_z);
  uLCD.cls();
  uLCD.printf("Threshold: %d",ThresholdAngle);

  //printf("\rReference vectors (x,y,z): (%d, %d, %d)\n",ref_x,ref_y,ref_z);
  BSP_ACCELERO_AccGetXYZ(initDataXYZ);
  int Z_movement = initDataXYZ[2];
  //int control=0;
  float Measured_Angle = 0;
  int TILT_NUMBER =0;//Used to track the number of tracks by USER
  int MA_uindex=3;
  ThisThread::sleep_for(600ms);
  
   while(TILT_NUMBER<14){
      BSP_ACCELERO_AccGetXYZ(initDataXYZ);
      Z_movement = initDataXYZ[2];
      if(Z_movement>1000) Z_movement=1000; // if the ratio is >1, acos will return nan
      if(Z_movement<-1000) Z_movement=-1000;
      Measured_Angle = (acos(Z_movement/1000.0))*(180/3.142); // REMEMBER THE .0 else acos will not return the correct value
      printf("%1.1f\n\r",Measured_Angle);
      
      TILT_NUMBER++;
      led1=!led1;
      led2=!led2;
      //Measured_Angle = (acos(Z_movement/1000.0))*(180/3.142); // REMEMBER THE .0 else acos will not return the correct value
      
      if((Measured_Angle<ThresholdAngle)){
         if(MA_uindex==13){
            uLCD.cls();
            MA_uindex = 3;
            uLCD.printf("Threshold: %d",ThresholdAngle);
         }
         PUBLISH_ANGLE(CLIENT, Measured_Angle,0,0);
         uLCD.locate(0,MA_uindex);//position of the text
         uLCD.printf("INVALID! Angle:%1.1f",Measured_Angle);
         
         
      }
      else{
         if(MA_uindex==13){
            uLCD.cls();
            MA_uindex = 3;
            uLCD.printf("Threshold: %d",ThresholdAngle);
         }
         uLCD.locate(0,MA_uindex);
         uLCD.printf("SUCCESS! Angle:%1.1f",Measured_Angle);
         PUBLISH_ANGLE(CLIENT, Measured_Angle,0,1);
         ThisThread::sleep_for(300ms);

      }
      ANGLE_COLLECTION[TILT_NUMBER]=Measured_Angle;
      MA_uindex = MA_uindex+2;
      ThisThread::sleep_for(1s);
   }
   //if(control==0) PUBLISH_ANGLE(CLIENT,Measured_Angle,1,0);//FINISH;
   //control=1;
   //printf("\r\nAngle: (Angle Measured < Threshold Angle!)!\r\n");
   printf("\n\r-----FINISH-----\n\r");
   uLCD.locate(0,MA_uindex);
   uLCD.printf("\n\r------FINISH------\n\r");

  
    
  PUBLISH_ANGLE(CLIENT,Measured_Angle,1,0);
   //ThisThread::sleep_for(200ms);
   //BSP_ACCELERO_AccGetXYZ(initDataXYZ);
   //Z_movement = initDataXYZ[2];
   //Measured_Angle = (acos(Z_movement/1000.0))*(180/3.142); // REMEMBER THE .0 else acos will not return the correct value
   //printf("%1.1f\n\r",Measured_Angle);
  
  return 1;//if successful
 
 
 /* 
   i=0;
   MA_uindex = MA_uindex+2;
   printf("\r\nSUCCESFUL!\r\n");
   uLCD.locate(0,MA_uindex);
   uLCD.printf("SUCCESS! Angle:%d",Measured_Angle);
   
  //D_x,D_y,D_z will eventually be used to store the gravity vector
  //This part is to check the threshold angle
    

 /* 
 for(int i=0;i<20;i++){
   if(i==15)break;
   else printf("index: %d\r\n",i); 
  }
  */
}

void Angle (Arguments *in, Reply *out)   {
    moveline_enable=1;
    bool success = true;
    //char buffer[200], outbuf[256];
    //char strings[20];
    t3.start(callback(&queue, &EventQueue::dispatch_forever));
    queue.call(angle_detection);
}





void close_mqtt() {
    closed = true;
}




int main() 
{
    config.seq_length=64;
    
    config.consecutiveInferenceThresholds[0]=25;
    config.consecutiveInferenceThresholds[1]=10;
    config.consecutiveInferenceThresholds[2]=10;

    config.output_message[0]="UP";
    config.output_message[1]="DOWN";
    config.output_message[2]="DOWN"; 
 
   // t1.start(callback(&queue, &EventQueue::dispatch_forever));
    //queue.call(Display);

    RPCthread.start(callback(&RPCqueue, &EventQueue::dispatch_forever));
    RPCqueue.call(calling_RPC);

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
            printf("ERROR: No WiFiInterface found.\r\n");
            return -1;
    }


    printf("\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
            printf("\nConnection error: %d\r\n", ret);
            return -1;
    }

    
    NetworkInterface* net = wifi;
    MQTTNetwork mqttNetwork(net);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    //TODO: revise host to your IP
    const char* host = "172.20.10.2";
    printf("Connecting to TCP network...\r\n");

    SocketAddress sockAddr;
    sockAddr.set_ip_address(host);
    sockAddr.set_port(1883);

    printf("address is %s/%d\r\n", (sockAddr.get_ip_address() ? sockAddr.get_ip_address() : "None"),  (sockAddr.get_port() ? sockAddr.get_port() : 0) ); //check setting

    int rc = mqttNetwork.connect(sockAddr);//(host, 1883);
    if (rc != 0) {
            printf("Connection error.");
            return -1;
    }
    printf("Successfully connected!\r\n");

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "Mbed";

    if ((rc = client.connect(data)) != 0){
            printf("Fail to connect MQTT\r\n");
    }
   // if (client.subscribe(topic, MQTT::QOS0, messageArrived) != 0){
   //         printf("Fail to subscribe\r\n");
   // }
    CLIENT = &client;
    Thread mqtt_threadPublish(osPriorityHigh);
    EventQueue mqtt_queuePublish;
    
    mqtt_thread.start(callback(&mqtt_queue, &EventQueue::dispatch_forever));
    button.rise(mqtt_queue.event(&Decision, &client));

    //mqtt_threadPublish.start(callback(&mqtt_queuePublish, &EventQueue::dispatch_forever));
    //mqtt_queuePublish.call(&Decision, &client,Lindex,enable);

    //btn3.rise(&close_mqtt);
    //mqtt_thread    
    int num = 0;
    while (num != 5) {
            client.yield(100);
            ++num;
    }

    while (1) {
            if (closed) break;
            client.yield(500);
            ThisThread::sleep_for(500ms);
    }

    printf("Ready to close MQTT Network......\n");

    if ((rc = client.unsubscribe(topic)) != 0) {
            printf("Failed: rc from unsubscribe was %d\n", rc);
    }
    if ((rc = client.disconnect()) != 0) {
    printf("Failed: rc from disconnect was %d\n", rc);
    }

    mqttNetwork.disconnect();
    printf("Successfully closed!\n");

    return 0;
   
}







