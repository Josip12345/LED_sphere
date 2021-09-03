# LED_sphere
Code for controlling RGB LED half sphere via web server on ESP8266 (NodeMCU 1.0 (ESP-12E Module)).
In principle the half sphere is 9x16 matrix, with the exception that the first row has only 1 and the second row has only 8 leds.

**Video: https://youtu.be/-h-K3bPTNTU**

This project is a combination of:
* https://github.com/toggledbits/MatrixFireFast
* https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/
* https://randomnerdtutorials.com/esp32-esp8266-rgb-led-strip-web-server/
* https://randomnerdtutorials.com/esp8266-nodemcu-web-server-websocket-sliders/#arduino

Thanks to the authors of above mentioned projects!

## Prerequisites:


  For the full description of how to install needed software modules check:
  https://randomnerdtutorials.com/esp8266-nodemcu-web-server-websocket-sliders/#arduino
  
  In a nutshell you will need:
   * Arduino IDE with ESP8266 Boards Add-on (in this project NodeMCU 1.0 (ESP-12E Module) was used)
   * LittleFS Filesystem uploader, plungin for Arduino IDE
   * Arduino_JSON library by Arduino version 0.1.0 (Arduino Library Manager)
   * ESPAsyncWebServer (.zip folder)
   * ESPAsyncTCP (.zip folder)

The code consists of the arduino sketch (splitted into "LED_sphere_all" and "qFire") and html, css and java files in the "data" folder. Just copy all of those into your arduino
sketch folder. Then load the sketch and upload the content of the "data" folder to ESP using LittleFS plugin. That should be it.
  
