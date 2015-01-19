

// The maximum message line length.
#define MAX_LINE_LENGTH 80

// The maximum number of tokens in a single message.
#define MAX_TOKENS 10
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
      analogWrite( 3, value );
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
      
void setup()
{
  // initialize the Serial port
  Serial.begin( 9600);

}    
    
    
    
void loop()
{
  serial_input_poll();
  // other polled tasks can go here
}

/****************************************************************/
/****************************************************************/
