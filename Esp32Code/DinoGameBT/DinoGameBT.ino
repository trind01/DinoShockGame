#include <Wire.h>
#include <BleKeyboard.h>
#include "BluetoothSerial.h"

BleKeyboard bleKeyboard;
BluetoothSerial ESP_BT; //Object for Bluetooth

const byte BOOT_BUTTON_PIN = 0;
bool ButtonPressed();
bool start_game = false;
void ButtonInterrupt();

const byte MPU_ADDR = 0x68;
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature; // variables for temperature data
char tmp_str[7]; // temporary variable used in convert function
char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(BOOT_BUTTON_PIN,INPUT);
  Serial.begin(115200);
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  ESP_BT.begin("ESP32_Dino_Game"); //Name of your Bluetooth Signal
  bleKeyboard.begin();

  attachInterrupt(BOOT_BUTTON_PIN, ButtonInterrupt, FALLING);
}

void loop() {
  try{
    if(start_game)
    {
      getMPUReadings();
    }
  if(start_game)
    {
      if(isDuck())
      {
        Duck();
      }
      else if(isJump())
      {
        Jump();
      }
    }
    if (ESP_BT.available()) //Check if we receive anything from Bluetooth
    {
      char incoming = ESP_BT.read(); //Read what we recevive
      if(incoming == 'F')
      {
        bleKeyboard.releaseAll();
        start_game = false;
      }
      Serial.println(incoming);
    }
  }
  catch(int e)
  {
    Serial.print("ERR: "); Serial.println(e);
  }
  delay(100);
}


bool ButtonPressed()
{
  return !digitalRead(BOOT_BUTTON_PIN);
}

void getMPUReadings()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
  
  // print out data
//  Serial.print("aX = "); Serial.print(convert_int16_to_str(accelerometer_x));
//  Serial.print(" | aY = "); Serial.print(convert_int16_to_str(accelerometer_y));
//  Serial.print(" | aZ = "); Serial.print(convert_int16_to_str(accelerometer_z));
//  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
//  Serial.print(" | tmp = "); Serial.print(temperature/340.00+36.53);
//  Serial.print(" | gX = "); Serial.print(convert_int16_to_str(gyro_x));
//  Serial.print(" | gY = "); Serial.print(convert_int16_to_str(gyro_y));
//  Serial.print(" | gZ = "); Serial.print(convert_int16_to_str(gyro_z));
//  Serial.println();
}

void ButtonInterrupt()
{
  start_game = !start_game;
}

bool isDuck()
{
  return (gyro_x < -15000 && accelerometer_z < 3000);
}

bool isJump()
{
  if(isDuck())
  {
    return false;
  }
  return (accelerometer_y < -25,000 || accelerometer_y > -7000);
}

void Jump()
{
  if(bleKeyboard.isConnected())
  {
    bleKeyboard.press(KEY_UP_ARROW);
    while(isJump())
    {
      getMPUReadings();
      delay(10);
    }
    bleKeyboard.release(KEY_UP_ARROW);
  }
}

void Duck()
{
    if(bleKeyboard.isConnected())
  {
    bleKeyboard.press(KEY_DOWN_ARROW);
    while(accelerometer_z < 1000)
    {
      getMPUReadings();
      delay(10);
    }
    bleKeyboard.release(KEY_DOWN_ARROW);
  }
}
