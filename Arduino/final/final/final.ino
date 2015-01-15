#define DATA_SIZE 4 // as a test, define 10 as the maximum number of data
#define NUM_OF_RANDOM 400 // small number to increase chance of number not changing
#include <Encoder.h>
Encoder knobLeft(2, 6);
Encoder knobRight(3, 7);
// the buffer that transfers data should be as big as the maximum number of data * 4 
// for the actual values to be transferred and their indices
// + 3 for 0xc0 and the bytes that goes into [repack]
const int total_size = (DATA_SIZE * 4) + 3;

byte data_buffer[total_size]; // set a maximum size for the array
byte stored_data[DATA_SIZE]; // array for value comparison
byte val[DATA_SIZE];
int pulsador1 = 10;
int pulsador2 = 11;

void setup() {
  Serial.begin(9600);
  pinMode(pulsador1, INPUT);
  pinMode(pulsador2, INPUT);
  // initialize values in array that stores data for comparison
  
}
long positionLeft  = -999;
long positionRight = -999;

void loop() {
  int repack_size = 0;
  int final_size;
  data_buffer[0] = 0xc0;
  // give an offset of 3, cause during the process you calculate the size of data
  // that needs to be the second value in the list and it's split in two
  int index = 3;
  // set random values and check whether changed
  // if changed stored them in data_buffer and afterwards store its index
  long newLeft, newRight;
  newLeft = knobLeft.read();
  newRight = knobRight.read();
  if (newLeft != positionLeft || newRight != positionRight)
  {
      val[0] = map(newLeft,0,1023,0,255);
      val[1] = map(newRight,0,1023,0,255);
      val[2] = digitalRead(pulsador1);
      val[3] = digitalRead(pulsador2);
      
    for(int i = 0; i < DATA_SIZE; i++){
      if(val[i] != stored_data[i]){
        data_buffer[index++] = val[i] & 0x007f;
        data_buffer[index++] = val[i] >> 7;
        data_buffer[index++] = i & 0x007f;
        data_buffer[index++] = i >> 7;
        stored_data[i] = val[i];
        // increment the size of the data that will be transferred
        repack_size++;
      }
    }
    // quadraple repack_size cause bytes are being split in two
    // and we send a value index along with that value, also split in two
    repack_size *= 4;
    // second (and third) element in array is the number of values to transfer
    // excluding itself, sent to [repack]
    data_buffer[1] = repack_size & 0x007f;
    data_buffer[2] = repack_size >> 7;
    // set transfer bytes size
    // + 3 is for the number that goes into [repack] and 0xc0
    final_size = repack_size + 3;
    // in case any number has changed write it to the serial line
    if(repack_size) Serial.write(data_buffer, final_size);
    // wait for a second
    //delay(1000);
  }
}
