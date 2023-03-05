#include <Wire.h> 
#include <LiquidCrystal.h> 

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//  Variables
int pulsePin = 0;                 
int blinkPin = 13;               
int fadePin = 8;                 
int fadeRate = 0;                 



// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   
volatile int Signal;                
volatile int IBI = 600;            
volatile boolean Pulse = false;      
volatile boolean QS = false;        

// Regards Serial OutPut  -- Set This Up to your needs
static boolean serialVisual = true;   

volatile int rate[10];                      
volatile unsigned long sampleCounter = 0;          
volatile unsigned long lastBeatTime = 0;          
volatile int P = 512;                      
volatile int T = 512;                   
volatile int thresh = 525;                
volatile int amp = 100;                   
volatile boolean firstBeat = true;        
volatile boolean secondBeat = false;      

void setup()
{
  pinMode(blinkPin,OUTPUT);        
  pinMode(fadePin,OUTPUT);         
  Serial.begin(9600);
  
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
                                    // IF YOU ARE POWERING The Pulse Sensor AT VOLTAGE LESS THAN THE BOARD VOLTAGE, 
                                    // UN-COMMENT THE NEXT LINE AND APPLY THAT VOLTAGE TO THE A-REF PIN
                                    //   analogReference(EXTERNAL);   
}


//  Where the Magic Happens
void loop()
{
   serialOutput();  
   
  if (QS == true) // A Heartbeat Was Found
    {     
      // BPM and IBI have been Determined
      // Quantified Self "QS" true when arduino finds a heartbeat
      fadeRate = 255; // Makes the LED Fade Effect Happen, Set 'fadeRate' Variable to 255 to fade LED with pulse
      serialOutputWhenBeatHappens(); // A Beat Happened, Output that to serial.     
      QS = false; // reset the Quantified Self flag for next time    
    }
     
  ledFadeToBeat(); // Makes the LED Fade Effect Happen 
  delay(20); //  take a break
}

void ledFadeToBeat()
{
   fadeRate -= 15;                         //  set LED fade value
   fadeRate = constrain(fadeRate,0,255);   //  keep LED fade value from going into negative numbers!
   analogWrite(fadePin,fadeRate);          //  fade LED
}

void interruptSetup()
{     
  // Initializes Timer2 to throw an interrupt every 2mS.
  TCCR2A = 0x02;     // DISABLE PWM ON DIGITAL PINS 3 AND 11, AND GO INTO CTC MODE
  TCCR2B = 0x06;     // DON'T FORCE COMPARE, 256 PRESCALER 
  OCR2A = 0X7C;      // SET THE TOP OF THE COUNT TO 124 FOR 500Hz SAMPLE RATE
  TIMSK2 = 0x02;     // ENABLE INTERRUPT ON MATCH BETWEEN TIMER2 AND OCR2A
  sei();             // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED      
} 

void serialOutput()
{   // Decide How To Output Serial. 
 if (serialVisual == true)
  {  
     arduinoSerialMonitorVisual('-', Signal);   // goes to function that makes Serial Monitor Visualizer
  } 
 else
  {
      sendDataToSerial('S', Signal);     // goes to sendDataToSerial function
   }        
}

void serialOutputWhenBeatHappens()
{    
 if (serialVisual == true) //  Code to Make the Serial Monitor Visualizer Work
   {            
     Serial.print("*** Heart-Beat Happened *** ");  //ASCII Art Madness
     Serial.print("BPM: ");
     Serial.println(BPM);
     lcd.clear();
     lcd.print("Heart Beat: ");
     lcd.print(BPM);
   }
 else
   {
     sendDataToSerial('B',BPM);   // send heart rate with a 'B' prefix
     sendDataToSerial('Q',IBI);   // send time between beats with a 'Q' prefix
   }   
}

void arduinoSerialMonitorVisual(char symbol, int data )
{    
  const int sensorMin = 0;      // sensor minimum, discovered through experiment
  const int sensorMax = 1024;    // sensor maximum, discovered through experiment
  int sensorReading = data; // map the sensor range to a range of 12 options:
  int range = map(sensorReading, sensorMin, sensorMax, 0, 11);
  // do something different depending on the 
  // range value:
 
}


void sendDataToSerial(char symbol, int data )
{
   Serial.print(symbol);
   Serial.println(data);                
}

ISR(TIMER2_COMPA_vect) //triggered when Timer2 counts to 124
{  
  cli();                                      
  Signal = analogRead(pulsePin);             
  sampleCounter += 2;                         // keep track of the time in mS with this variable
  int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise
                                              //  find the peak and trough of the pulse wave
  if(Signal < thresh && N > (IBI/5)*3) // avoid dichrotic noise by waiting 3/5 of last IBI
    {      
      if (Signal < T) // T is the trough
      {                        
        T = Signal; // keep track of lowest point in pulse wave 
      }
    }

  if(Signal > thresh && Signal > P)
    {          // thresh condition helps avoid noise
      P = Signal;                             // P is the peak
    }                                        // keep track of highest point in pulse wave

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (N > 250)
  {                                   
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) )
      {        
        Pulse = true;                               
        digitalWrite(blinkPin,HIGH);                // turn on pin 13 LED
        IBI = sampleCounter - lastBeatTime;         
        lastBeatTime = sampleCounter;              
  
        if(secondBeat)
        {                        
          secondBeat = false;                  
          for(int i=0; i<=9; i++) 
          {             
            rate[i] = IBI;                      
          }
        }
  
        if(firstBeat) 
        {                         
          firstBeat = false;                  
          secondBeat = true;                   
          sei();                               
          return;                              
        }   
      // keep a running total of the last 10 IBI values
      word runningTotal = 0;                  

      for(int i=0; i<=8; i++)
        {               
          rate[i] = rate[i+1];                 
          runningTotal += rate[i];             
        }

      rate[9] = IBI;                         
      runningTotal += rate[9];              
      runningTotal /= 10;                    
      BPM = 60000/runningTotal;               
      QS = true;                               
      
    }                       
  }

  if (Signal < thresh && Pulse == true)
    {   
      digitalWrite(blinkPin,LOW);            
      Pulse = false;                         
      amp = P - T;                           
      thresh = amp/2 + T;                    
      P = thresh;                            
      T = thresh;
    }

  if (N > 2500)
    {                           
      thresh = 512;                       
      P = 512;                              
      T = 512;                              
      lastBeatTime = sampleCounter;               
      firstBeat = true;                      
      secondBeat = false;                    
    }

  sei();                                  
}// end isr
