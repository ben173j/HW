import paho.mqtt.client as paho
import time
import serial

# https://os.mbed.com/teams/mqtt/wiki/Using-MQTT#python-client

# MQTT broker hosted on local machine
mqttc = paho.Client()

# Settings for connection
# TODO: revise host to your IP
host = "172.20.10.2"
topic = "Mbed"
serdev = '/dev/ttyACM0'
s = serial.Serial(serdev, 9600)
# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n");
    string_msg = str(msg.payload)
    string_msg = string_msg[6:14]
    #print(string_msg+"\n\r")
    if string_msg=="Selected":
        #s.write(bytes("\r\n/UI/delete \r\n",'UTF-8'))
        time.sleep(1)
        s.write(bytes("\r\n/Angle/run \r\n",'UTF-8'))
    

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=240)
mqttc.subscribe(topic, 0)
s.write(bytes("/UI/run\r\n",'UTF-8'))

# Publish messages from Python
num = 0
while num != 5:
    ret = mqttc.publish(topic, "Message from Python!\n", qos=0)
    
    if (ret[0] != 0):
            print("Publish failed")
    mqttc.loop()
    time.sleep(1.5)
    num += 1

# Loop forever, receiving messages
mqttc.loop_forever()