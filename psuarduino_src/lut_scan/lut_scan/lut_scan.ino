void setup() {
  analogWrite(3, 0);
  Serial.begin(9600);
}

void loop() {
  for(int iter = 0; iter < 256; iter++)
  {
    Serial.println(iter);
    analogWrite(3, iter);
    delay(3500);
  }
  while(1)
  {
    analogWrite(3, 0);
    delay(1000);
  }
}
