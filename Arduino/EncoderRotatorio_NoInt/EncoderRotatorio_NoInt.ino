
//these pins can not be changed 2/3 are special pins
int encoderPin1 = 4;
int encoderPin2 = 5;

int lastEncoded = 0;
long encoderValue = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;
long t;

void setup() {
  Serial.begin (9600);

  pinMode(encoderPin1, INPUT); 
  pinMode(encoderPin2, INPUT);

  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on

  
}

void loop(){ 
  t = micros();
  updateEncoder();
  t= micros()-t;
  Serial.println(encoderValue);
  //Serial.write(t);
  delay(100);
}


void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded; //store this value for next time
}
