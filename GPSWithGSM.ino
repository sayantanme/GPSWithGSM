/*
  Web client

 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe, based on work by Adrian McEwen

 */
#include <stdlib.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)
//char server[] = "http://lppapproval.eu-gb.mybluemix.net";
//char server[] = "www.httpbin.org";
  char server[] = "http://mywebapps.mybluemix.net";

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
TinyGPS gps;
SoftwareSerial ss(4, 3);
bool conn;

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec,char buf[]);
static void print_int(unsigned long val, unsigned long invalid, int len,char buf[]);
static void print_date(TinyGPS &gps,char buf1[]);
//aJsonObject* formJSON(char sats[],char hdopVal[],char latt[],char lon[],char dateTime[],char alt[],char speedkmph[]);
void formJSON1(char sats[],char hdopVal[],char latt[],char lon[],char dateTime[],char alt[],char speedkmph[],char json[]);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  ss.begin(9600);
  
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  conn = false;
  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop()
{
  //*************GPS*********************
  float flat, flon;
  unsigned long age, date, time, chars = 0;
  //unsigned short sentences = 0, failed = 0;
  //static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
  
  char sats[6];
  print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5,sats);

  char hdopVAL[6];
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5,hdopVAL);
 
  gps.f_get_position(&flat, &flon, &age);
  char latt[20];
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6,latt);
  char lon[22];
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6,lon);

  //print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  char dateTime[32];
  print_date(gps,dateTime);
  char alt[20];
  print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2,alt);
  char speedkmph[12];
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2,speedkmph);
  
//  aJsonObject* json = formJSON(sats,hdopVAL,latt,lon,dateTime,alt,speedkmph);
//  char* string = aJson.print(json);
//  Serial.println(string);
//  Serial.println();
  char json[150];
  formJSON1(sats,hdopVAL,latt,lon,dateTime,alt,speedkmph,json);
  Serial.println(json);
  smartdelay(5000);
  //*************************************
  
  //**************HTTP*******************
  
  Serial.println("connecting..."); 
  
  // if you get a connection, report back via serial:
  if(!client.connected())
  {
      if (client.connect(server, 80)) {
        Serial.println("connected");
        conn = true;
      }
      else {
      // kf you didn't get a connection to the server:
      Serial.println("connection failed");
      conn = false;
    }
  }
  
    // Make a HTTP request:
//    client.println("GET /get text/html");
//    //client.println(" HTTP/1.1");
//    //client.println("Content-Type: application/json;charset=utf-8");
//    client.println("Host: www.httpbin.org");
//    client.println("Connection: close");
//    client.println();

    if(client.connected() && strlen(json) > 10){
      Serial.println("POSTing");
      // Make a HTTP PUT request:
      client.println("POST /api/Mydb/PushInfo HTTP/1.1");
      client.println("Host: mywebapps.mybluemix.net");
      client.println("Content-Length: ");
      client.print(strlen(json)) ; 
      client.println("Content-Type: application/json;charset=utf-8");
      client.println(json);
      //client.println("Arduino");  
      client.println("Keep-Alive: 10000"); 
      client.println("Connection: keep-alive");
      client.println();
    }
  
  

  delay(7000);
 // if there are incoming bytes available
 // from the server, read them and print them:
  while(true){
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
      //delay(100);
    }
    else{
      Serial.print("no reply");
      client.stop();
      break;
    }
  }
  delay(1000);
  //if the server's disconnected, stop the client:
//  if (!client.connected()) {
//    Serial.println();
//    Serial.println("disconnecting.");
//    client.stop();
//
//    // do nothing forevermore:
//    while (true);
//  }

  Serial.println("repeat.");
  smartdelay(5000);
}

void formJSON1(char sats[],char hdopVal[],char latt[],char lon[],char dateTime[],char alt[],char speedkmph[],char json[])
{
  Serial.println("Inside formJSON");
  if(sats[0] != '*' && hdopVal[0] != '*' && latt[0] != '*' && lon[0] != '*' && dateTime[0] != '*' && alt[0] != '*' && speedkmph[0] != '*' && json[0] != '*')
  {
    int len = 100 +strlen(speedkmph)+strlen(sats)+strlen(hdopVal)+strlen(latt)+strlen(lon)+strlen(dateTime)+strlen(alt);
    char str[len];
    Serial.println(len);
    
    strcpy(str,"{\"sats\":\"");
    strcat(str,sats);
    strcat(str,"\",");
    
  //  strcat(str,"\"hdop\":\"");
  //  strcat(str,hdopVal);
  //  strcat(str,"\",");
  
    strcat(str,"\"latt\":\"");
    strcat(str,latt);  
    strcat(str,"\",");
    
    strcat(str,"\"lon\":\"");
    strcat(str,lon);
    strcat(str,"\",");
  
    strcat(str,"\"dateTime\":\"");
    strcat(str,dateTime);
    strcat(str,"\",");
  
    strcat(str,"\"alt\":\"");
    strcat(str,alt);
    strcat(str,"\",");
  
    strcat(str,"\"speed\":\"");
    strcat(str,speedkmph);
    strcat(str,"\",");
    
    strcat(str,"\"DeviceName\":\"");
    strcat(str,"1");
    strcat(str,"\"}");
    
    Serial.println(strlen(str));  
    Serial.println(str);
    Serial.println("Leaving formJSON");
    strcpy(json,str);
  }
}
static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec,char buf[])
{
  char sz[len];  
  //Serial.println("Inside print_float");
  Serial.print("");
  if (val == invalid)
  {
    while (len-- > 1){
      //Serial.print('*');
      sz[len]= '*';
    }
    //Serial.print(' ');
    sz[len]= '*';
  }
  else
  {
    //Serial.println("Inside print_float");
    //floatval = String(val,prec);
    dtostrf(val,len+1,prec+1,sz); 
//    Serial.print('\'');
//    Serial.print(sz);
//    Serial.println('\'');
    //int pos = 0;
    for (int i=0; i<strlen(sz); i++)
    {
      //Serial.print(sz[i],DEC);  
      //delay(500);   
      if(sz[i] == ' ')
      {
        for(int j = i;j<(strlen(sz)-1); j++){
        sz[j]=sz[j+1];
        //pos++;
        //sz[i] = ' ';
        }
      }
    }
  }
  smartdelay(0);
//  Serial.print(sz);
  strcpy(buf,sz);
//  Serial.print('\'');
//  Serial.print(sz);
//  Serial.print('\'');
  Serial.print("");
 // Serial.println("Leaving print_float");
}

static void print_int(unsigned long val, unsigned long invalid, int len,char buf[])
{
  //Serial.println("Inside print_int");
  Serial.print("");
  char sz[6];  
  if (val == invalid)
    strcpy(sz, "*");
  else
    sprintf(sz, "%ld", val);
//  sz[len] = 0;
//  for (int i=strlen(sz); i<len; ++i)
//    sz[i] = ' ';
//  if (len > 0) 
//    sz[len-1] = '\0';
  //Serial.print(sz);
  //String number = String(sz);
  //Serial.print(String("number "+number));
  smartdelay(0);
//  Serial.println(sz);
  strcpy(buf,sz);
//  Serial.println(buf);
  Serial.print("");
  //Serial.println("leaving print_int");
}

static void print_date(TinyGPS &gps,char buf1[])
{
  //Serial.println("Inside print_date");
  Serial.print("");
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  char sz[32];
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE){
    //Serial.print("********** ******** ");
    strcpy(sz, "*");
  }
  else
  {    
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d",
        month, day, year, hour, minute, second);
    //Serial.print(sz);
  }
  //print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
//  Serial.print(sz);
  strcpy(buf1,sz);
//  Serial.println(buf1);
  Serial.print("");
  //Serial.println("leaving print_date");
}

