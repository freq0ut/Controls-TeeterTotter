/*
* TITLE:	Teensy Teeter Openloop Controller
* AUTHOR:	Zack Goyetche
* DATE:		11/10/2015
*/

/**********************************/
/*********GLOBAL VARIABLES*********/
/**********************************/

/*Variables to handle serial commands*/
//-------------------------------------
String inputString = "";          
char ar_userInput[32];
float fl_userInput = 0;
char incomingByte;
boolean stringComplete;
boolean boolSetDAC;
int dacWrite = 0;
int ledPin = 13;

/*Variables for difference equation (OL)*/
//----------------------------------------
float a = 1;		// coefficient for x(n)
float a1 = -1.892;	// coefficient for x(n-1)
float a2 = 0.9876;	// coefficient for x(n-2)
  
float x = 0;
float x_1 = 0;
float x_2 = 0;

float b1 = 1.38;	// coefficient for y(n-1)
float b2 = -0.4761;	// coefficient for y(n-2)

float y_new = 0.0;
float y_1 = 0.0;
float y_2 = 0.0;

float unityK = 1.0004; // gain factor for unity

/*Variables for DAC/ADC*/
//-----------------------
float refVolt = 0.0;	// reference signal in volts
float refPos = 0.0;		// reference signal in millimeters

void setup() 
{
  analogWriteResolution(12);
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,HIGH);
}

void loop() 
{
  USB();
  y_new = DiffEqn(refPos);
  if(y_new >= 0 && y_new <= 3.3)
    analogWrite(A14, y_new*4096/3.3);
  x_2 = x_1;		// x(n-2)
  x_1 = refVolt;	// x(n-1)
  y_2 = y_1;		// y(n-2)
  y_1 = y_new;		// y(n-1)
  delayMicroseconds(18000); 
}

float DiffEqn(float ref)
{
  float y_new1;
  refVolt = -ref/2.7399 + 1.65;
  y_new1 = (unityK*(a*refVolt + a1*x_1 + a2*x_2) + b1*y_1 + b2*y_2);
  return y_new1;
}

void USB(void)
{
   //serial_menu();
   while (Serial.available() > 0)              // Scan serial port for activity
   { 
     serial_accum();
   }     
   if (stringComplete == true)                 // Checks to see if we have completed text
   {
     serial_commands();                        // Go To serial commands sub
     reset_serial();                           // Reset data once I am done doing something with it
   }
}

void serial_accum(void) // This function accumulated Serial input data for reading back
{
  char inChar = (char)Serial.read();                                           // Flush serial data into variable inChar
  
  if (inChar == 8 && inputString.length() > 0)                                 // Check for backspace
  {
     Serial.write(inChar);                                                     // Echo Data Back to serial port
     Serial.write(" ");                                                        // Echo Blank Data Back To Serial Port
     Serial.write(8);                                                          // Echo backspace Back To Serial Port
     inputString = inputString.substring(0, inputString.length() - 1);         // Subtract character from string object     
  }
  if (inChar == 13)                                                            // Check for line feed
  {
    Serial.write(inChar);                                                      // Echo Data Back to serial port
    Serial.write(char(13));                                                    // Write a line feed and set stringComplete to true
    stringComplete = true;                                                     // stringComplete becomes True
  }
  if (inChar == 10)                                                            // Check for line feed
  {
    stringComplete = true;                                                     // stringComplete becomes True
  }
  if (inChar != 10 && inChar != 8 && inChar != 13)                             // If both carriage return and backspace weren't pressed then do the following
  {
    Serial.write(inChar);                                                      // Echo Data Back to serial port
    inputString += inChar;                                                     // Accumulate Data into inputString
  }                         
} 

void reset_serial(void) // Clear Serial output with prompt
{
   inputString = "";   // clear the string:
   stringComplete = false;                                                     // Reset 
}

void serial_menu(void)//Main Serial Menu
{ 
  Serial.println("");
  Serial.println(" ________________________________________________________  ");
  Serial.println("|********************************************************|"); 
  Serial.println("|******************** OL CONTROLLER! ********************|"); 
  Serial.println("|********************************************************|"); 
  Serial.println("| ?            = Display available commands.             |");  
  Serial.println("| ref?         = Display the current reference point.    |");
  Serial.println("| SetRef       = Set a new reference point.              |");
  Serial.println("|                                                        |");
  Serial.println("|________________________________________________________|");
  Serial.println("");
}

void serial_commands(void)		//Check serial command inputs
{
  inputString.toLowerCase();	// Converts the inputString to lowercase - easier to handle in if then statements

  char setpoint_string[32];
  inputString.toCharArray(setpoint_string,32);
   
  if(inputString == "?")
  {
    serial_menu();				// Displays Menu Commands
  }

  if(inputString == "ref?")
  {
    Serial.println("");
    Serial.print("The DAC is set to: "); 
    Serial.println(refVolt);                 
  } 

  if(inputString == "setref")
  {
    reset_serial();
    Serial.println("");
    Serial.print("Enter a position between -4.0mm and 4.3mm: ");
    setDacRoutine();  
  } 
}

void setDacRoutine(void)
{
  boolSetDAC = false;
    while(boolSetDAC == false)
    {
      while (Serial.available() > 0)	// Scan serial port for activity
      { 
        serial_accum();
      }     
      if(stringComplete == true)		// Checks to see if we have completed text
      {
        inputString.toCharArray(ar_userInput,32);
        fl_userInput = atof(ar_userInput);
        if(fl_userInput >= -4.7285 && fl_userInput <= 4.3131)
        {
          refPos = fl_userInput;
          Serial.println("");
          boolSetDAC = true;
        }
        else
        {
          Serial.println("");
          Serial.println("Invalid input...!");
          Serial.print("Enter a position between -4.0mm and 4.3mm: ");
          reset_serial();
        }
      }
   }  
}
