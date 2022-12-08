/* Get tilt angles on X and Y, and rotation angle on Z
 * Angles are given in degrees
 *
 * License: MIT
 */
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "Wire.h"
#include <MPU6050_light.h>

// MPU6050 //
MPU6050 mpu(Wire);
float data_yaw;

// HCSR-04 //
unsigned long timer = 0;

const int trigPin = D4;
const int echoPin = D3;

// define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

// Serial //
uint8_t val;
char dataYawSend[4];
char dataSensorSend[4];
char sendData[10];

char auth[] = "4SNdyI6J-5xjTmfmvQO7BM_kKLLrIHpR";

char ssid[] = "JTE-WIFI";
char pass[] = "";

const char *state;

const char *header_1 = "s";
const char *header_2 = "e";
const char *header_3 = "t";

char dataYawKirim[8];
char dataHCSRKirim[8];

char *data_kirim;

BLYNK_WRITE(V0)
{
    val = param.asInt();
    if (val == 0)
    {
        state = "0";
    }
    else
    {
        state = "1";
    }
    digitalWrite(D0, val);
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    // HCSR //
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input

    // Gyro //

    byte status = mpu.begin();
    while (status != 0)
    {
    } // stop everything if could not connect to MPU6050

    delay(3000);
    // mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
    mpu.calcOffsets(); // gyro and accelero

    // Serial //
    pinMode(D0, OUTPUT);
    digitalWrite(D0, 1);
    delay(200);
    digitalWrite(D0, 0);
    delay(200);
    digitalWrite(D0, 1);
    delay(200);
    digitalWrite(D0, 0);
    delay(200);
    digitalWrite(D0, 1);
    delay(200);
    digitalWrite(D0, 0);
    Blynk.begin(auth, ssid, pass, "blynk.cloud");
}

void loop()
{
    Blynk.run();

    mpu.update();

    if ((millis() - timer) > 10)
    { // print data every 10ms
        data_yaw = mpu.getAngleZ();
        timer = millis();
    }
    if(data_yaw <= -180)
      data_yaw += 360;
    if(data_yaw >= 180)
      data_yaw -= 360;

    

    //  HCSR  //

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);

    distanceCm = duration * SOUND_VELOCITY / 2;

    distanceInch = distanceCm * CM_TO_INCH;

    dataYawKirim[0] = ((*(long int*)&data_yaw)>>0)&0xFF;
    dataYawKirim[1] = ((*(long int*)&data_yaw)>>8)&0xFF;
    dataYawKirim[2] = ((*(long int*)&data_yaw)>>16)&0xFF;
    dataYawKirim[3] = ((*(long int*)&data_yaw)>>24)&0xFF;
    
    dataHCSRKirim[0] = ((*(long int*)&distanceCm)>>0)&0xFF;
    dataHCSRKirim[1] = ((*(long int*)&distanceCm)>>8)&0xFF;
    dataHCSRKirim[2] = ((*(long int*)&distanceCm)>>16)&0xFF;
    dataHCSRKirim[3] = ((*(long int*)&distanceCm)>>24)&0xFF;

    // Serial //
    Serial.write(val);
    Serial.write(dataYawKirim[0]);
    Serial.write(dataYawKirim[1]);
    Serial.write(dataYawKirim[2]);
    Serial.write(dataYawKirim[3]);
    Serial.write(dataHCSRKirim[0]);
    Serial.write(dataHCSRKirim[1]);
    Serial.write(dataHCSRKirim[2]);
    Serial.write(dataHCSRKirim[3]);
}