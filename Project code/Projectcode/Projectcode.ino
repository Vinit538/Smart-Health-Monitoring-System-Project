#include <SoftwareSerial.h>
SoftwareSerial gprsSerial(2,3);
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <String.h>
#include <DHT.h> 
#define DHTPIN A2
DHT dht(DHTPIN, DHT11);

int val;
int tempPin=1;





byte heart[] = {
  0x00,
  0x0A,
  0x1F,
  0x1F,
  0x0E,
  0x04,
  0x00,
  0x00
};

//  Variables
  

void setup()
{
  gprsSerial.begin(9600);               // the GPRS baud rate   
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("****WEL COME****");
  lcd.setCursor(0,1);
  lcd.print("*******TO*******");
  delay(2000);
  lcd.clear();
  lcd.createChar(1, heart);    
  lcd.setCursor(0, 0);         
  lcd.print("  ");           
  lcd.write(1);                
  lcd.print(" SHMS ");
  delay(3000);
  lcd.clear();
  Serial.begin(9600);      
  dht.begin();
                 
                                       
}


//  Where the Magic Happens
void loop()
{
  int t = dht.readTemperature();
  int h = dht.readHumidity();
     val=analogRead(tempPin);
     int mv=(val/1024.0)*500;
     int tempc=mv;
     lcd.setCursor(0,0);
     lcd.print(" BODY TEMP  :");
     lcd.print(tempc);
     lcd.setCursor(0,1);
     lcd.print("RT  : ");
     lcd.print(t);
     lcd.setCursor(8,1);
     lcd.print(" H  : " );
     lcd.print(h);  
   
   if (gprsSerial.available())
    Serial.write(gprsSerial.read());
 
  gprsSerial.println("AT");
  delay(1000);
 
  gprsSerial.println("AT+CPIN?");
  delay(1000);
 
  gprsSerial.println("AT+CREG?");
  delay(1000);
 
  gprsSerial.println("AT+CGATT?");
  delay(1000);
 
  gprsSerial.println("AT+CIPSHUT");
  delay(1000);
 
  gprsSerial.println("AT+CIPSTATUS");
  delay(2000);
 
  gprsSerial.println("AT+CIPMUX=0");
  delay(2000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CSTT=\"bsnlnet\"");//start task and setting the APN,
  delay(1000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIICR");//bring up wireless connection
  delay(3000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(2000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSPRT=0");
  delay(3000);
 
  ShowSerialData();
  
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(5000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  ShowSerialData();
  
  String str="GET https://api.thingspeak.com/update?api_key=MUM56GU8YPU0G0TV&field1=0"+String(tempc) +"&field2="+String(t) +"&field3="+String(h);
  Serial.println(str);
  gprsSerial.println(str);//begin send data to remote server
  
  delay(4000);
  ShowSerialData();
 
  gprsSerial.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  gprsSerial.println();
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData(); 
    
}
    
void ShowSerialData()
{
  while(gprsSerial.available()!=0)
  Serial.write(gprsSerial.read());
  delay(5000); 
  
}
