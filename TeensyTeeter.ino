/*
* TITLE:	Teensy Teeter Controller
* AUTHOR:	Zack Goyetche
* DATE:		11/10/2015
*/

// TODO: Add modes for no control, open loop, closed loop, Bridged T.

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

boolean boolOLMode;
boolean boolBTMode;

/*Variables for difference equation (CL Lead)*/
//----------------------------------------
float CL_K = 276;
float zero = 0.934;

boolean boolCLMode;
boolean boolSetControl;

/*Variables for DAC/ADC*/
//-----------------------
float refVolt = 0.0;	// reference signal in volts
float refPos = 0.0;		// reference signal in millimeters

float senseVolt = 0.0;	// sensor voltage from ADC
float error = 0.0;		// error from feedback
int analogValue;
float scaledAnalog;

void setup() 
{
	analogWriteResolution(12);
	Serial.begin(9600);
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin,HIGH);
	pinMode(A0,INPUT);
	boolBTMode = false;
	boolCLMode = true;
	boolOLMode = false;
}

void loop() 
{
	USB();	
	if(boolOLMode == true)
		OLDiffEqn(refPos);
	if(boolCLMode == true)
		CLDiffEqn(refPos);
	delayMicroseconds(18000); 
}

void OLDiffEqn(float ref)
{
	float y_new;
	refVolt = -ref/2.7399 + 1.65; //---------------------------------------------this may need to be fixed?
	y_new = (unityK*(a*refVolt + a1*x_1 + a2*x_2) + b1*y_1 + b2*y_2);
	if(y_new >= 0 && y_new <= 3.3)
		analogWrite(A14, y_new*4096/3.3); // write to DAC -- set output voltage
	x_2 = x_1;		// x(n-2)	shift x(n-1)	to		x(n-2)
	x_1 = refVolt;	// x(n-1)	shift x(n)		to		x(n-1)
	y_2 = y_1;		// y(n-2)	shift y(n-1)	to		y(n-2)
	y_1 = y_new;		// y(n-1)	shift y(n)		to		y(n-1)
}

void CLDiffEqn(float ref)
{
	float y_new1;
	refVolt = -ref*0.06339 + 1.65;
	analogValue = analogRead(A0);
	scaledAnalog = (float)analogValue/1023*3.3;
	senseVolt = scaledAnalog;
	error = refVolt - senseVolt;
	y_new1 = CL_K*(error-zero*x_1);
	y_new1 = (3.3/20)*y_new1+1.65;
	analogWrite(A14, y_new1/3.3*4096);
	x_1 = error;	// x(n-1)	shift x(n)		to		x(n-1)
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
				Serial.print("Enter a position between -4.7mm and 4.3mm: ");
				reset_serial();
			}
		}
	}
}

void USB(void)
{
	//serial_menu();
	while (Serial.available() > 0)		// Scan serial port for activity
	{ 
		serial_accum();
	}     
	if (stringComplete == true)			// Checks to see if we have completed text
	{
		serial_commands();				// Go To serial commands sub
		reset_serial();					// Reset data once I am done doing something with it
	}
}

void serial_accum(void) // This function accumulated Serial input data for reading back
{
	char inChar = (char)Serial.read();// Flush serial data into variable inChar
  
	if (inChar == 8 && inputString.length() > 0)								// Check for backspace
	{
		Serial.write(inChar);													// Echo Data Back to serial port
		Serial.write(" ");														// Echo Blank Data Back To Serial Port
		Serial.write(8);														// Echo backspace Back To Serial Port
		inputString = inputString.substring(0, inputString.length() - 1);		// Subtract character from string object     
	}
	if (inChar == 13)															// Check for line feed
	{
		Serial.write(inChar);													// Echo Data Back to serial port
		Serial.write(char(13));													// Write a line feed and set stringComplete to true
		stringComplete = true;													// stringComplete becomes True
	}
	if (inChar == 10)															// Check for line feed
	{
		stringComplete = true;													// stringComplete becomes True
	}
	if (inChar != 10 && inChar != 8 && inChar != 13)							// If both carriage return and backspace weren't pressed then do the following
	{
		Serial.write(inChar);													// Echo Data Back to serial port
		inputString += inChar;													// Accumulate Data into inputString
	}                         
} 

void reset_serial(void)						// clear serial output with prompt
{
	inputString = "";						// clear the string
	stringComplete = false;					// reset 
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
	Serial.println("| Ctrl         = Set control mode.                       |");
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
	Serial.println("");
	Serial.print("The DAC is set to: "); 
	Serial.println(refVolt);
	Serial.print("Analog Voltage: ");Serial.println(scaledAnalog);Serial.print("Compensated DAC voltage: ");Serial.println(y_new);     
	Serial.println("");            
	} 
	if(inputString == "setref")
	{
	reset_serial();
	Serial.println("");
	Serial.print("Enter a position between -4.7mm and 4.3mm: ");
	setDacRoutine();
	} 
	if(inputString == "ctrl")
	{
		reset_serial();
		Serial.println("");
		Serial.println("Enter a control mode... ");
		Serial.println("OL:		Open loop controller");
		Serial.println("Lead:		CL lead compensator");
		Serial.println("T:		CL Bridged-T");
		Serial.println("None:		No control");
		Serial.println("");
		boolSetControl = false;
		while(boolSetControl == false)
		{
			while (Serial.available() > 0)	// Scan serial port for activity
				serial_accum();
			if(stringComplete == true)		// Checks to see if we have completed text
			{
				inputString.toLowerCase();	// Converts the inputString to lowercase - easier to handle in if then statements
				if(inputString == "ol")
				{
					boolOLMode = true;
					boolCLMode = false;
					boolBTMode = false;
					boolSetControl = true;
					Serial.println("");
					Serial.println("Set to Open Loop Control!");
					reset_serial();
				}
				else if(inputString == "lead")
				{
					boolCLMode = true;
					boolBTMode = false;
					boolOLMode = false;
					boolSetControl = true;
					Serial.println("");
					Serial.println("Set to Lead Compensated CL Control!");
					reset_serial();
				}
				else if(inputString == "t")
				{
				  
					boolBTMode = true;
					boolOLMode = false;
					boolCLMode = false;
					boolSetControl = true;
					Serial.println("");
					Serial.println("Set to Bridged-T CL Control!");
					reset_serial();
				}
				else if(inputString == "none")
				{
					boolCLMode = false;
					boolOLMode = false;
					boolBTMode = false;
					boolSetControl = true;
					Serial.println("");
					Serial.println("All control has been disabled!");
					reset_serial();
				}
				else
				{
					Serial.println("");
					Serial.println("Invalid input...!");
					Serial.println("");
					Serial.println("Enter a control mode... ");
					Serial.println("OL:		Open loop controller");
					Serial.println("Lead:		CL lead compensator");
					Serial.println("T:		CL Bridged-T");
					Serial.println("None:		No control");
					reset_serial();
				}
			}
		}
	}
}


