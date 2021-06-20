/*
 * This example shows how to publish messages from outside of the Mosquitto network loop.
 */
#include <wiringPi.h>
#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>




typedef unsigned char uint8;
typedef unsigned int  uint16;
typedef unsigned long uint32;
 
#define HIGH_TIME 32
 
int pinNumber = 7;
uint32 databuf;
  
bool readSensorData(void)
{
	databuf = 0;
    uint8 crc; 
    uint8 i;
  
    pinMode(pinNumber, OUTPUT); // set mode to output
    digitalWrite(pinNumber, 0); // output a high level 
    delay(25);
    digitalWrite(pinNumber, 1); // output a low level 
    pinMode(pinNumber, INPUT); // set mode to input
    pullUpDnControl(pinNumber, PUD_UP);
 
    delayMicroseconds(27);
    if (digitalRead(pinNumber) == 0) //SENSOR ANS
    {
        while (!digitalRead(pinNumber))
            ; //wait to high
 
        for (i = 0; i < 32; i++)
        {
            while (digitalRead(pinNumber))
                ; //data clock start
            while (!digitalRead(pinNumber))
                ; //data start
            delayMicroseconds(HIGH_TIME);
            databuf *= 2;
            if (digitalRead(pinNumber) == 1) //1
            {
                databuf++;
            }
        }
	
        for (i = 0; i < 8; i++)
        {
            while (digitalRead(pinNumber))
                ; //data clock start
            while (!digitalRead(pinNumber))
                ; //data start
            delayMicroseconds(HIGH_TIME);
            crc *= 2;  
            if (digitalRead(pinNumber) == 1) //1
            {
                crc++;
            }
        }
		int tmp = (databuf>>24& 0xff) +(databuf>>16 & 0xff )+(databuf>>8 & 0xff) + (databuf & 0xff);
		if(tmp != crc )
        {
            printf("validate errr\n");
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}



/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	/* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	//printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		mosquitto_disconnect(mosq);
	}

	/* You may wish to set a flag here to indicate to your application that the
	 * client is now connected. */
}


/* Callback called when the client knows to the best of its abilities that a
 * PUBLISH has been successfully sent. For QoS 0 this means the message has
 * been completely written to the operating system. For QoS 1 this means we
 * have received a PUBACK from the broker. For QoS 2 this means we have
 * received a PUBCOMP from the broker. */
void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
	printf("Message with mid %d has been published.\n", mid);
}


int get_temperature(void)
{
	sleep(1); /* Prevent a storm of messages - this pretend sensor works at 1Hz */
	char buf[20];

	if(readSensorData())
	{
		sprintf(buf,"TMP:%d.%d", (databuf >> 8) & 0xff, databuf & 0xff);
        databuf = 0;
	}

	if (readSensorData())
        {
           // printf("Sensor data read ok!\n");
           // printf("RH:%d.%d\n", (databuf >> 24) & 0xff, (databuf >> 16) & 0xff); 
           //  printf("TMP:%d.%d\n", (databuf >> 8) & 0xff, databuf & 0xff);
			sprintf(buf,"TMP:%d.%d", (databuf >> 8) & 0xff, databuf & 0xff);
            databuf = 0;
        }
        else
        {
            printf("Sensor dosent ans!\n");
            databuf = 0;
        }
	
	
	return random()%100;
}


bool  get_temperaturep(char* buf)
{
	sleep(1); /* Prevent a storm of messages - this pretend sensor works at 1Hz */
	
	pinMode(pinNumber, OUTPUT); // set mode to output
    digitalWrite(pinNumber, 1); // output a high level 
    delay(3000);
	bool b = readSensorData();
	while (/* condition */)
	{
		sprintf(buf,"{\"RH\":%d.%d,\"TMP\":%d.%d}", (databuf >> 24) & 0xff, (databuf >> 16) & 0xff, (databuf >> 8) & 0xff, databuf & 0xff);
        databuf = 0;
	}
	return true;

	/*
	if (readSensorData())
        {
			sprintf(buf,"{\"RH\":%d.%d,\"TMP\":%d.%d}", (databuf >> 24) & 0xff, (databuf >> 16) & 0xff, (databuf >> 8) & 0xff, databuf & 0xff);
            databuf = 0;
			return true;
        }
        else
        {
            printf("Sensor dosent ans!\n");
            databuf = 0;
			return false;
        }
	*/
	

}
/* This function pretends to read some data from a sensor and publish it.*/
void publish_sensor_data(struct mosquitto *mosq)
{
	char payload[20];
	
	//int temp;
	int rc;

	/* Get our pretend data */
	//char* temp = get_temperature();
	//payload =  get_temperaturep();
	/* Print it to a string for easy human reading - payload format is highly
	 * application dependent. */
	//snprintf(payload, sizeof(payload), "%d", temp);
	bool bget = get_temperaturep(payload);

	if (!bget)
	{
		return ;
		/* code */
	}
	
	/* Publish the message
	 * mosq - our client instance
	 * *mid = NULL - we don't want to know what the message id for this message is
	 * topic = "example/temperature" - the topic on which this message will be published
	 * payloadlen = strlen(payload) - the length of our payload in bytes
	 * payload - the actual payload
	 * qos = 2 - publish with QoS 2 for this example
	 * retain = false - do not use the retained message feature for this message
	 */
	//rc = mosquitto_publish(mosq, NULL, "example/temperature", strlen(payload), payload, 2, false);
	rc = mosquitto_publish(mosq, NULL, "test", strlen(payload), payload, 2, false);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
	}
}


int main(int argc, char *argv[])
{

   // 
    if (-1 == wiringPiSetup()) {
        printf("Setup wiringPi failed!");
        return 1;
    }
   pinMode(pinNumber, OUTPUT); // set mode to output
   digitalWrite(pinNumber, 1); // output a high level


	struct mosquitto *mosq;
	int rc;

	
	/* Required before calling other mosquitto functions */
	mosquitto_lib_init();

	/* Create a new client instance.
	 * id = NULL -> ask the broker to generate a client id for us
	 * clean session = true -> the broker should remove old sessions when we connect
	 * obj = NULL -> we aren't passing any of our private data for callbacks
	 */
	mosq = mosquitto_new(NULL, true, NULL);
	mosquitto_username_pw_set(mosq,"yang","yang@5896336");
	if(mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	
	/* Configure callbacks. This should be done before connecting ideally. */
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_publish_callback_set(mosq, on_publish);

	/* Connect to test.mosquitto.org on port 1883, with a keepalive of 60 seconds.
	 * This call makes the socket connection only, it does not complete the MQTT
	 * CONNECT/CONNACK flow, you should use mosquitto_loop_start() or
	 * mosquitto_loop_forever() for processing net traffic. */

	rc = mosquitto_connect(mosq, "127.0.0.1", 1883, 60);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	/* Run the network loop in a background thread, this call returns quickly. */
	rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	/* At this point the client is connected to the network socket, but may not
	 * have completed CONNECT/CONNACK.
	 * It is fairly safe to start queuing messages at this point, but if you
	 * want to be really sure you should wait until after a successful call to
	 * the connect callback.
	 * In this case we know it is 1 second before we start publishing.
	 */
	//while(1){
		publish_sensor_data(mosq);
		sleep(5);
	//}

	mosquitto_lib_cleanup();
	return 0;
}

