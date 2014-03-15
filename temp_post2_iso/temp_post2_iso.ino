/* 
  Ocean Controls k-type board KTA259 TC Mux
  Neil Davey
  06-02-11  
*/

#include <Ethernet.h>
#include <SPI.h>
#include <string.h> //Use the string Library
#include <ctype.h>
#include <EEPROM.h>

#define CHANNEL_MAX 8 //set max cha 
#define CS_TEMP     9 // MAX6674/6675 /CS Line
#define MUX_EN      7 // ADG608 MUX Enable
#define MUX_A0      4 // ADG608 Addr0
#define MUX_A1      5 // ADG608 Arrd1
#define MUX_A2      6 // ADG608 Addr2
//added following defs
#define MISO 12 //TCAmp Slave Out pin (MISO) 
#define SCK 13 //TCAmp Serial Clock (SCK) 
#define ETH_SS 10  //Ethernet shield line

//from post sketch
byte mac[] = { 0x90,0xA2,0xDA,0x00,0xC7,0x36};  //Replace with your Ethernet shield MAC
EthernetClient client;
String data;
//IPAddress ip(192, 168, 7, 255); //ethernet ip address attatched to my ethernet shield

//-------------------------
//-------------------------
void Set_Mux_Channel(unsigned char chan)
{
  switch(chan)
  {
    //----------------------
    // MUX Channel 1
    case 1:
      digitalWrite(MUX_A0,LOW);
      digitalWrite(MUX_A1,LOW);
      digitalWrite(MUX_A2,LOW);
      break;
    //----------------------
    // MUX Channel 2
    case 2:
      digitalWrite(MUX_A0,HIGH);
      digitalWrite(MUX_A1,LOW);
      digitalWrite(MUX_A2,LOW);
      break;
    //----------------------
    // MUX Channel 3
    case 3:
      digitalWrite(MUX_A0,LOW);
      digitalWrite(MUX_A1,HIGH);
      digitalWrite(MUX_A2,LOW);
      break;
    //----------------------
    // MUX Channel 4
    case 4:
      digitalWrite(MUX_A0,HIGH);
      digitalWrite(MUX_A1,HIGH);
      digitalWrite(MUX_A2,LOW);
      break;
    //----------------------
    // MUX Channel 5
    case 5:
      digitalWrite(MUX_A0,LOW);
      digitalWrite(MUX_A1,LOW);
      digitalWrite(MUX_A2,HIGH);
      break;
    //----------------------
    // MUX Channel 6
    case 6:
      digitalWrite(MUX_A0,HIGH);
      digitalWrite(MUX_A1,LOW);
      digitalWrite(MUX_A2,HIGH);
      break;
    //----------------------
    // MUX Channel 7
    case 7:
      digitalWrite(MUX_A0,LOW);
      digitalWrite(MUX_A1,HIGH);
      digitalWrite(MUX_A2,HIGH);
      break;
    //----------------------
    // MUX Channel 8
    case 8:
      digitalWrite(MUX_A0,HIGH);
      digitalWrite(MUX_A1,HIGH);
      digitalWrite(MUX_A2,HIGH);
      break;
    //----------------------
    default:
      break;
  }
}

//-------------------------
//-------------------------
int Read_Temperature(void)
{
unsigned int temp_reading;

  // force conversion now...
  delay(5);
  digitalWrite(CS_TEMP,LOW); // Set MAX7765 /CS Low
  delay(5);
  digitalWrite(CS_TEMP,HIGH); // Set MAX7765 /CS High
  delay(250); // wait for conversion to finish..
  
  // read result
  digitalWrite(CS_TEMP,LOW); // Set MAX7765 /CS Low
  delay(1);
  
  temp_reading = SPI.transfer(0xff) << 8;  
  temp_reading += SPI.transfer(0xff);  

  digitalWrite(CS_TEMP,HIGH); // Set MAX7765 /CS High
  delay(1);
  
  // check result
  if(bitRead(temp_reading,2) == 1) // No Connection
    {
      return(-1); // Failed / NC Error
    }
    else
    {
      return((int)(temp_reading >> 5)); //Convert to Degc
    }
}

//-------------------------
//-------------------------
void setup()
{
  Serial.begin(9600);
  pinMode(ETH_SS,OUTPUT); // Hardware /SS, need to be output

  pinMode(CS_TEMP,OUTPUT); // MAX6675/6674 /CS Line
  pinMode(MUX_EN,OUTPUT); // Enable pin on ADG608
  pinMode(MUX_A0,OUTPUT); // A0 on ADG608
  pinMode(MUX_A1,OUTPUT); // A1 on ADG608
  pinMode(MUX_A2,OUTPUT); // A2 on ADG608

  pinMode(MISO, INPUT);        
  pinMode(SCK, OUTPUT);
 //When a device's Slave Select pin is low, it communicates with the master. 
 //When it's high, it ignores the master.
 
  
  digitalWrite(CS_TEMP,HIGH); // Set MAX7765 /CS High
  digitalWrite(MUX_EN,HIGH); // Enable on
  //digitalWrite(ETH_SS,HIGH);
  SPI.begin(); // Init SPI
}

//-------------------------
//-------------------------

void post(int temperature, int id){
   //let's try connecting to the web 
   delay(1000);
   digitalWrite(ETH_SS,HIGH);
   Serial.print("attempting to connect to thermabot w/ temp reading: ");
   Serial.print(temperature);
   Serial.print(" and an id of: ");
   Serial.println(id);

   Ethernet.begin(mac);
   Serial.println("we Fucking made it!");
  
   data="";
   data+=(String)id + ' ' + (String)temperature; // Use HTML encoding for comma's
   data+="&submit=Submit"; // Submitting data

   if (client.connect("www.thermabot.com",80)) {
     Serial.println("connected");
     client.println("POST /data/insert_msql.php HTTP/1.1");
     client.println("Host: www.thermabot.com");
     client.println("Content-Type: application/x-www-form-urlencoded");
     client.println("Connection: close");
     client.print("Content-Length: ");
     client.println(data.length());
     client.println();
     client.print(data);
     client.println();
    
     //Prints your post request out for debugging
     Serial.println("POST /data/insert_data.php HTTP/1.1");
     Serial.println("Host: www.thermabot.com");
     Serial.println("Content-Type: application/x-www-form-urlencoded");
     Serial.println("Connection: close");
     Serial.print("Content-Length: ");
     Serial.println(data.length());
     Serial.println();
     Serial.print(data);
     Serial.println();
  }
  if (client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  digitalWrite(ETH_SS,LOW);
  delay(10);
} 

void loop()
{
int temperature = 0;

while(1)
  {
    for(int i=1;i<=CHANNEL_MAX;i++){
      
      delay(1000);
      Set_Mux_Channel(i);
      temperature = Read_Temperature(); 
      if(temperature == -1)
      {
        Serial.print(i);
        Serial.println(": No Data...");
      }
      else
      {
        digitalWrite(CS_TEMP,HIGH); // Set MAX7765 /CS High
        digitalWrite(MUX_EN,HIGH); // Enable on
        delay(1000);
        post(temperature,i);
        digitalWrite(CS_TEMP,LOW); 
        digitalWrite(MUX_EN,LOW); 

        Serial.print(i);
        Serial.print(": ");
        Serial.print(temperature,DEC); 
        Serial.println(" DegC");
      }
    }
  }
}
