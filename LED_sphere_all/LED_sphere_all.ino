/* 
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-web-server-websocket-sliders/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>

#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>
#define MAT_TYPE NEOPIXEL   /* leds LED type; see FastLED docs for others */
#define NUM_LEDS 128
#define DATA_PIN 3
int BRIGHT=50;

// Define the array of leds
CRGB leds[NUM_LEDS];

// Replace with your network credentials
const char *ssid = "QubitDJ";
const char *password = "password";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object

AsyncWebSocket ws("/ws");

String message = "";
String sliderValue1 = "0";
String sliderValue2 = "0";
String sliderValue3 = "0";

String dirigent = "default";
String temp_dirigent = "";

int sliderValue1_int = 20;
int sliderValue1_int_temp = 20;


long ret=0x60C000;
long temp_ret =0x60C000;

uint8_t rgb[3];

void init_color(){
for (int i = 0; i < 3; i++) { // Set default color
    rgb[i] = 100;
  }
}


//Json Variable to Hold Slider Values
JSONVar sliderValues;




//Get Slider Value
String getSliderValues(){
  sliderValues["sliderValue1"] = String(sliderValue1);

  String jsonString = JSON.stringify(sliderValues);
  return jsonString;
}

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
   Serial.println("LittleFS mounted successfully");
  }
}



IPAddress local_IP(192,168,4,6);
IPAddress gateway(192,168,4,3);
IPAddress subnet(255,255,255,0);

// Initialize WiFi
void initWiFi() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
  
  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");
  
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

void notifyClients(String sliderValues) {
  ws.textAll(sliderValues);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;

    Serial.println(message);

    // This gets the string which should determine which function should run in the loop
    if (message.indexOf("Func") >= 0){
       dirigent = message.substring(4);
      }
    
    // This converts string to hex if message contains the identfier from updatePickedColor in js file, which is "pi"
    if (message.indexOf("pi") >= 0) {
        char hex[7] = {'\0'};
        String mes;
        mes = message.substring(2);
        mes.toCharArray(hex,7);
    
        char *ptr;
        
    
        ret = strtoul(hex, &ptr, 16);
    
        // Get the bytes for each color
        for (int i = 0; i < 3; i++) {
          rgb[i] = (ret >> (16 - 8 * i)) & 0xff;
        }

    }

   // Change the actual LEDs color
//   int i = 0;
//    for(i=0;i<NUM_LEDS;i++){
//    
//    leds[i].setRGB(rgb[1],rgb[0],rgb[2]);
//    FastLED.show();
//    delay(1);
//  }
  
    
  
    // This translates the message in case the slider was moved
    if (message.indexOf("1s") >= 0) {
      sliderValue1 = message.substring(2);
      sliderValue1_int =  map(sliderValue1.toInt(), 0, 500, 0, 30);
      Serial.println(sliderValue1_int);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    // This translates the message in case the slider was moved
    if (message.indexOf("2s") >= 0) {
      sliderValue2 = message.substring(2);
      BRIGHT =  map(sliderValue2.toInt(), 0, 180, 0, 180); 
      FastLED.setBrightness(BRIGHT); // Change the brightness
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }

//    if (strcmp((char*)data, "getValues") == 0) {
//      notifyClients(getSliderValues());
//    }
  }
}


void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void Fire_setup(); // Function declaration

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.setBrightness(BRIGHT);
  init_color();
  Fire_setup();
  initFS();
  initWiFi();

  initWebSocket();
  
  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/pickerhtml.html", "text/html");
  });
  
  server.serveStatic("/", LittleFS, "/");

  // Start server
  //server.begin();
}





int pos(int x, int y){
  // Function for converting row/column indexes into linear LED array indexes
  
  const uint8_t phy_h = 8; // Height of the leds
  const uint8_t phy_w = 16; // Width of the leds
  
  int ind;
  x = x%16;
  ind = y*phy_w + x + 9;
  
  return ind;
  }






//////////// For qDART

int xr4 = 0; 

void darty(){
  int i = 0;
  int j = 0;
  const int l = 7;

  leds[0] = CRGB::Black;

  xr4 = (xr4 + 1)%16;
  
  FastLED.clear();
  for (i=0; i<l; i++){

      if(i==0){
      leds[pos((i+xr4),2)].setRGB(rgb[1],rgb[0],rgb[2]);
      leds[pos((i+xr4),4)].setRGB(rgb[1],rgb[0],rgb[2]);
      leds[pos((i+xr4),6)].setRGB(rgb[1],rgb[0],rgb[2]);
      }
      else if(i==1 or i==2){
        leds[pos((i+xr4),3)].setRGB(rgb[1],rgb[0],rgb[2]);
        leds[pos((i+xr4),4)].setRGB(rgb[1],rgb[0],rgb[2]);
        leds[pos((i+xr4),5)].setRGB(rgb[1],rgb[0],rgb[2]);
        }
      else if(i>2){
        leds[pos((i+xr4),4)].setRGB(rgb[1],rgb[0],rgb[2]);
        }
      leds[0] = CRGB::Black; // Trying to extinguish this blinking of the first LED
      
  
      }
      FastLED.show();
 
     
      delay(pow(30-sliderValue1_int,2));
  }


////// For qGLOW
void qGLOW_f(){
    //int temp_r = rgb[1] + rgb[0] + rgb[2];
    int i;
    for(i=0;i<NUM_LEDS;i++){
    if(dirigent!=temp_dirigent){  // If there is a cahnge of function during the loop, break it
      break;
    }
    //if(temp_r!=(rgb[1] + rgb[0] + rgb[2])){ //Uncomment this if you want that every change starts from the first LED
    //  break;
    //}
    
    
    leds[i].setRGB(rgb[1],rgb[0],rgb[2]);
    FastLED.show();
    delay(pow(30-sliderValue1_int,2)/10);
  }
}


void qGLOW_default(){
  int i = 0;
  int escape = 0;
  for(i=0;i<NUM_LEDS;i++){
    if((dirigent!=temp_dirigent) or (escape ==1)){  // If there is a cahnge of function during the loop, break it
      escape = 1;
      break;
    }
    
    leds[i] = CRGB::Green; 
    FastLED.show();
    delay(pow(30-sliderValue1_int,2)/10);
  }
  for(i=0;i<NUM_LEDS;i++){
    if((dirigent!=temp_dirigent) or (escape ==1)){  // If there is a cahnge of function during the loop, break it
      escape = 1;
      break;
    }
    
    leds[i] = CRGB::Red;
    FastLED.show();
    delay(pow(30-sliderValue1_int,2)/10);
  }
  for(i=0;i<NUM_LEDS;i++){
    if((dirigent!=temp_dirigent) or (escape ==1)){  // If there is a cahnge of function during the loop, break it
      escape = 1;
      break;
    }
    
    leds[i] = CRGB::Blue;
    FastLED.show();
    delay(pow(30-sliderValue1_int,2)/10);
  }
}


//////// For qFIRE

uint32_t colors[] = {
  0x000000,
  0x001000,
  0x003000,
  0x006000,
  0x008000,
  0x00A000,
  0x20C000,
  0x40C000,
  0x60C000,
  0x80C000,
  0x708080
};
const uint8_t NCOLORS = (sizeof(colors)/sizeof(colors[0]));

unsigned long createRGB(int r, int g, int b)
{   
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

uint8_t flarerows = 6;    /* number of rows (from bottom) allowed to flare */
const uint8_t maxflare = 5;     /* max number of simultaneous flares */
uint8_t flarechance = 20; /* chance (%) of a new flare (if there's room) */

void fire_loop();

void qFIRE_f(){
  fire_loop();
  int i;
  int r;
  int g;
  int b;
  if(ret!=temp_ret){ // If the color was not changed, use the default palette but if changed calculate new palette
    temp_ret = ret;
  for(i=0;i<NCOLORS;i++){
    r = i*rgb[0]/(NCOLORS-1);
    g = i*rgb[1]/(NCOLORS-1);
    b = i*rgb[2]/(NCOLORS-1);
    colors[i] = createRGB(g, r, b);

    }
  }
  if(sliderValue1_int!=sliderValue1_int_temp){
    sliderValue1_int_temp =sliderValue1_int;
    flarerows = sliderValue1_int/2;
    flarechance = 5 + sliderValue1_int*3;

    
  }

}
  
unsigned long def_wait = random(5,10)*1000;
unsigned long time_start = 0;
unsigned long myTime = 0;

void loop() {
  
 // If there was a change of function, clear all leads 
 if(dirigent!=temp_dirigent){
  FastLED.clear();
  temp_dirigent = dirigent;
  def_wait = random(5,20)*1000; /* Determine new time for the default mode in milliseconds*/
  }

 if (dirigent == "qDART"){
  darty();
     }

 if (dirigent == "qGLOW"){
  FastLED.setBrightness(BRIGHT/2);
  qGLOW_f();
  FastLED.setBrightness(BRIGHT);
  }

 if (dirigent == "qFIRE"){
  qFIRE_f();
 }

 if(dirigent == "default"){
  time_start = millis();
  FastLED.clear();
  FastLED.setBrightness(BRIGHT/2);
  while(true){    
    myTime = millis();
    if((myTime-time_start)>def_wait){
      break;
    }

    qGLOW_default();
        if((dirigent!=temp_dirigent)){  // If there is a cahnge of function during the loop, break it
      break;
    }
    }
  FastLED.setBrightness(BRIGHT);

  FastLED.clear();
  time_start = millis();
  while(true){
    myTime = millis();
    if((myTime-time_start)>def_wait){
      break;
    }
    darty();
    if((dirigent!=temp_dirigent)){  // If there is a cahnge of function during the loop, break it
      break;
    }
    }

  FastLED.clear();
  time_start = millis();  
  while(true){
    myTime = millis();
    if((myTime-time_start)>def_wait){
      break;
    }
    qFIRE_f();
    if((dirigent!=temp_dirigent)){  // If there is a cahnge of function during the loop, break it
      break;
    }
    }
   
    
 }
 

  

  //ws.cleanupClients();
}
