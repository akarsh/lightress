#include <SPI.h>
#include <Ethernet.h>
 
//  Hue constants
const char hueHubIP[] = "10.0.0.7";  // Hue hub IP
const char hueUsername[] = "GZTlFVlMbJPC2ItnQMdheAGt82W5LGac6HJBcLGd";  // Hue username
const int hueHubPort = 80;

//  Hue variables
boolean hueOn;  // on/off
int hueBri;  // brightness value
long hueHue;  // hue value
String hueCmd;  // Hue command

unsigned long buffer=0;  //buffer for received data storage
unsigned long addr;
 
//  Ethernet
byte mac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };  // W5100 MAC address
IPAddress ip(130,149,222,33);  // Arduino IP
EthernetClient client;

// Hear rate
const int HR_RX = 2;
long hr = 0;
long last_hr_ts = 0;
long last_report_ts = 0;
int report_delay = 2000; //Must be > 1000 miliseconds
long samples[8] = {0};  //size was chosen since it's optimized in division and modulus
byte last_sample = 0;

void setup()
{
 Serial.begin(9600);
 pinMode (HR_RX, INPUT);  //Signal pin to input 
 
 Ethernet.begin(mac,ip); // Hue connection
 Serial.println("Hue Ready.");
 
   Serial.println("Waiting for heart beat...");
  //Wait until a heart beat is detected  
  while (!digitalRead(HR_RX)) {};
  Serial.println ("Heart beat detected!"); 
}

void loop()
{    
 
  long current_time = millis();
  if (digitalRead(HR_RX) == HIGH)  {
        //First sample will be garbage, since last_hr_ts = 0
        //dt < 10 means the same sample. Otherwise it means hr of 6000bpm...
        //That's fast even for a colibri
        if (current_time - last_hr_ts > 10) {
            byte next_sample = ((last_sample + 1) % 8);
            samples[next_sample] = current_time - last_hr_ts;
            last_sample = next_sample;
        }
        last_hr_ts = current_time;
      }

  if (current_time - last_report_ts > report_delay)   {
    if (current_time - last_hr_ts < report_delay) {
      {
        long sum_samples = 0;
        byte i;
        for (i = 1; i < 9; ++i) {
          sum_samples += samples[((last_sample + i) % 8)];  //optimizes to (last_sample + 1)&7
      }
        sum_samples /= 8;  //optimizes to sum_samples >>= 3
          /*To be accurate (2 digits), we multiply the sample  by 100, and finally divide result */
        //hr = ((100000/(sum_samples))*60)/100;
        hr = (6000000/(sum_samples))/100;
//        Serial.println("Heart beat: ");
        Serial.println(hr);
        // Hue setting
  if (hr < 60) {
//    Serial.println("hr < 60 value");
      String command1 = "{\"on\": true,\"hue\": 0,\"sat\":100,\"bri\":50,\"transitiontime\":"+String(random(15,25))+"}";
  setHue(4,command1);
  setHue(5,command1);
  setHue(6,command1);
  }
  else   if (hr >= 60 && hr < 70 ) {
//      Serial.println("hr >70 value");
      String command1 = "{\"on\": true,\"hue\": 0,\"sat\":100,\"bri\":65,\"transitiontime\":"+String(random(15,25))+"}";
  setHue(4,command1);
  setHue(5,command1);
  setHue(6,command1);
  }
  else   if (hr >= 70 && hr < 80 ) {
//      Serial.println("hr >70 value");
      String command1 = "{\"on\": true,\"hue\": 0,\"sat\":0,\"bri\":100,\"transitiontime\":"+String(random(15,25))+"}";
  setHue(4,command1);
  setHue(5,command1);
  setHue(6,command1);
  }
  else if (hr >= 80 && hr < 90 ) {
//          Serial.println("hr <90 value");
      String command1 = "{\"on\": true,\"hue\": 0,\"sat\":0,\"bri\":100,\"transitiontime\":"+String(random(15,25))+"}";
  setHue(4,command1);
  setHue(5,command1);
  setHue(6,command1);
  }
  else if(hr >= 90 && hr < 100){
    String command1 = "{\"on\": true,\"hue\": 43680,\"sat\":100,\"bri\":65,\"transitiontime\":"+String(random(15,25))+"}";
  setHue(4,command1);
  setHue(5,command1);
  setHue(6,command1);
  }
  else
  {
//    Serial.println("hr >90 value");
//  Hue
    String command1 = "{\"on\": true,\"hue\": 43680,\"sat\":100,\"bri\":50,\"transitiontime\":"+String(random(15,25))+"}";
//    String command2 = "{\"on\": false,\"hue\": 65280,\"sat\":200,\"bri\":255,\"transitiontime\":"+String(random(15,25))+"}";
    setHue(4,command1);
    setHue(5,command1);
    setHue(6,command1);
  }
  
      }
    } else {
//      Serial.println("Out of range, greater then 8 samples");
      Serial.println(hr);
    }
    last_report_ts = current_time;
  }  
}

// setting Hue
boolean setHue(int lightNum,String command)
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    while (client.connected())
    {
      client.print("PUT /api/");
      client.print(hueUsername);
      client.print("/lights/");
      client.print(lightNum);  // hueLight zero based, add 1
      client.println("/state HTTP/1.1");
      client.println("keep-alive");
      client.print("Host: ");
      client.println(hueHubIP);
      client.print("Content-Length: ");
      client.println(command.length());
      client.println("Content-Type: text/plain;charset=UTF-8");
      client.println();  // blank line before body
      client.println(command);  // Hue command
      break;
    }
    client.stop();
    return true;  // command executed
  }
  else
    return false;  // command failed
}

// Hue to know about the lights state
boolean getHue(int lightNum)
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    client.print("GET /api/");
    client.print(hueUsername);
    client.print("/lights/");
    client.print(lightNum);  
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(hueHubIP);
    client.println("Content-type: application/json");
    client.println("keep-alive");
    client.println();
    while (client.connected())
    {
      if (client.available())
      {
        client.findUntil("\"on\":", "\0");
        hueOn = (client.readStringUntil(',') == "true");  // if light is on, set variable to true
 
        client.findUntil("\"bri\":", "\0");
        hueBri = client.readStringUntil(',').toInt();  // set variable to brightness value
 
        client.findUntil("\"hue\":", "\0");
        hueHue = client.readStringUntil(',').toInt();  // set variable to hue value
        
        break;  // not capturing other light attributes yet
      }
    }
    client.stop();
    return true;  // captured on,bri,hue
  }
  else
    return false;  // error reading on,bri,hue
}
