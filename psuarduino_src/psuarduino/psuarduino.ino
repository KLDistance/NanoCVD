#include <string.h>

String serial_number_buf;
char serial_response[64] = "[arduino_nanocvd_8392af01]\0";

void setup() {
  analogWrite(3, 0);
  Serial.begin(9600);
}

void loop() {
  if(Serial.available())
  {
    serial_number_buf = Serial.readStringUntil('\n');
    if(!strcmp(serial_number_buf.c_str(), "[host_nanocvd_8392af00]"))
    {
      Serial.println(serial_response);
      Serial.flush();
    }
    else
    {
      serial_number_buf.replace("\n", "");
      analogWrite(3, serial_number_buf.toInt());
    }
  }
}
