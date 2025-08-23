void setup() {
  // put your setup code here, to run once:
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D4, OUTPUT);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.print("Start");
}

void loop() {
  digitalWrite(D4, HIGH);
  if(digitalRead(D1)==LOW){
    Serial.print(1);
    digitalWrite(D4, LOW);
    delay(500);
  }
  else if(digitalRead(D2)==LOW){
    Serial.print(2);  
    digitalWrite(D4, LOW);
    delay(500);
  }
  else if(digitalRead(D3)==LOW){
    Serial.print(3); 
    digitalWrite(D4, LOW);
    delay(500);
  }
  else if(digitalRead(D4)==LOW){
    Serial.print(4);
    digitalWrite(D4, LOW);
    delay(500);
  }
  else if(digitalRead(D5)==LOW){
    Serial.print(5);
    digitalWrite(D4, LOW);
    delay(500);
  }
  else if(digitalRead(D6)==LOW){
    Serial.print(6);
    digitalWrite(D4, LOW);
    delay(500);
  }
  else if(digitalRead(D7)==LOW){
    Serial.print(7);
    digitalWrite(D4, LOW);
    delay(500);
  }
}
