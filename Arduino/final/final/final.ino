//Quedan definidos los puertos 5 9 10 y 11 para el PWM de los leds


#define DATA_SIZE 9 // as a test, define 10 as the maximum number of data
#define NUM_OF_RANDOM 400 // small number to increase chance of number not changing
#define BAUDIOS 9600
// The maximum message line length.
#define MAX_LINE_LENGTH 80
// The maximum number of tokens in a single message.
#define MAX_TOKENS 10

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
int pulsador1 = 4;
int pulsador2 = 8;
int pulsador3 = 12;
int pulsador4 = 13;
int Pot1 = A0;
int Pot2 = A1;
int Pot3 = A2;


/****************************************************************/
// Wrapper on strcmp for clarity of code.  Returns true if strings are
// identical.
static int string_equal( char *str1, char *str2) 
{
  return !strcmp(str1, str2);
}
/****************************************************************/
// Standard Arduino polling function to handle all I/O and periodic processing.
// This loop should never be allowed to stall or block so that all tasks can be
// constantly serviced.
static void parse_input_message(int argc, char *argv[])
{
  // Interpret the first token as a command symbol.
  char *command = argv[0];
  
   if (argc == 2) {
    int value = atoi(argv[1] );
    
    if ( string_equal( command, "le1" )) {
      analogWrite( 5, value );
      return;
    }
    else if ( string_equal( command, "le2" )) {
      analogWrite( 9, value );
      return;
    }
    else if ( string_equal( command, "le3" )) {
      analogWrite( 10, value );
      return;
    }
    else if ( string_equal( command, "le4" )) {
      analogWrite( 11, value );
      return;
    }
   }
}

static void serial_input_poll(void)
{
  static char input_buffer[ MAX_LINE_LENGTH ];   // buffer for input characters
  static char *argv[MAX_TOKENS];                 // buffer for pointers to tokens
  static int chars_in_buffer = 0;  // counter for characters in buffer
  static int chars_in_token = 0;   // counter for characters in current partially-received token (the 'open' token)
  static int argc = 0;             // counter for tokens in argv
  static int error = 0;            // flag for any error condition in the current message

  // Check if at least one byte is available on the serial input.
  if (Serial.available()) {
    int input = Serial.read();

    // If the input is a whitespace character, end any currently open token.
    if ( isspace(input) ) {
      if ( !error && chars_in_token > 0) {
	if (chars_in_buffer == MAX_LINE_LENGTH) error = 1;
	else {
	  input_buffer[chars_in_buffer++] = 0;  // end the current token
	  argc++;                               // increase the argument count
	  chars_in_token = 0;                   // reset the token state
	}
      }

      // If the whitespace input is an end-of-line character, then pass the message buffer along for interpretation.
      if (input == '\r' || input == '\n') {

	

	if (argc > 0) parse_input_message( argc, argv ); 

	// reset the full input state
	error = chars_in_token = chars_in_buffer = argc = 0;                     
      }
    }

    // Else the input is a character to store in the buffer at the end of the current token.
    else {
      // if beginning a new token
      if (chars_in_token == 0) {

	// if the token array is full, set an error state
	if (argc == MAX_TOKENS) error = 1;

	// otherwise save a pointer to the start of the token
	else argv[ argc ] = &input_buffer[chars_in_buffer];
      }

      // the save the input and update the counters
      if (!error) {
	if (chars_in_buffer == MAX_LINE_LENGTH) error = 1;
	else {
	  input_buffer[chars_in_buffer++] = input;
	  chars_in_token++;
	}
      }
    }
  }
}


void setup() {
  Serial.begin(BAUDIOS);
  pinMode(pulsador1, INPUT);
  pinMode(pulsador2, INPUT);
  pinMode(pulsador3, INPUT);
  pinMode(pulsador4, INPUT);
  // initialize values in array that stores data for comparison
  
}
long positionLeft  = -999;
long positionRight = -999;

void loop() {
  serial_input_poll();
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
  int Potenc1 = analogRead(Pot1);
  int Potenc2 = analogRead(Pot2);
  int Potenc3 = analogRead(Pot3);
  if (newLeft != positionLeft || newRight != positionRight)
  {
      val[0] = map(newLeft,0,1023,0,255);
      val[1] = map(newRight,0,1023,0,255);
      val[2] = digitalRead(pulsador1);
      val[3] = digitalRead(pulsador2);
      val[4] = digitalRead(pulsador3);
      val[5] = map(Potenc1,0,1023,0,255);
      val[6] = digitalRead(pulsador4);
      val[7] = map(Potenc2,0,1023,0,255);
      val[8] = map(Potenc3,0,1023,0,255);
      
      
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
  serial_input_poll();
}
