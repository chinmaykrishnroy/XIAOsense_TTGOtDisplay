#include <SPI.h>
#include <ArduinoWebsockets.h>
#include <WiFi.h>
#include <TJpg_Decoder.h>
#include <TFT_eSPI.h>
#include <TFT_eSPI_Widgets.h>
#include "Button2.h"

#define RIGHT_BUTTON_PIN  35
#define LEFT_BUTTON_PIN  0
#define YPOS_MAX 0
#define YPOS_MIN -105
#define MOVE_FORCE 20

const char* ssid = "Prefect SocketCam";
const char* password = "11223344";

const char *simple_message = "Init...";

unsigned long start_cycle;

using namespace TFT_eSPI_Widgets;
using namespace websockets;
WebsocketsServer server;
WebsocketsClient client;
TFT_eSPI tft = TFT_eSPI();
Button2 rButton = Button2(RIGHT_BUTTON_PIN);
Button2 lButton = Button2(LEFT_BUTTON_PIN);
Canvas canvas;

int yPos = -52;

void setup() {
  Serial.begin(115200);
  buttonInit();
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.setTextFont(1);
  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(tft_output);
  tft.setTextColor(TFT_BLUE,TFT_BLACK); 
  tft.print("@esp32:~ ");
  tft.setTextColor(TFT_GREEN,TFT_BLACK); 
  tft.println("Setting up the Access Point...");
  delay(100);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  tft.setTextColor(TFT_BLUE,TFT_BLACK); 
  tft.print("@esp32:~ ");
  tft.setTextColor(TFT_GREEN,TFT_BLACK); 
  tft.println("Access Point IP Address:");
  delay(250);
  tft.setTextColor(TFT_BLUE,TFT_BLACK); 
  tft.print("@esp32:~ ");
  tft.setTextColor(TFT_DARKCYAN,TFT_BLACK); 
  tft.println(IP);
  delay(150);
  tft.setTextColor(TFT_BLUE,TFT_BLACK); 
  tft.print("@esp32:~ ");
  tft.setTextColor(TFT_GREEN,TFT_BLACK); 
  tft.println("Waiting for Prefect XIAO Sense");
  tft.setTextColor(TFT_GREEN,TFT_BLACK); 
  delay(150);
  for(int i=0; i<5; i++){
    tft.print(".");
    delay(25);
  }
  delay(500);
  canvas.init(tft, GraphicalProperties(TFT_DARKGREY, TFT_NAVY, TFT_WHITE, 2));
  Area area = canvas.getArea();
  area *= 0.9;
  new GenericWidget(canvas, area);
  canvas.getChild().setPosition(50, 50);
  canvas.getChild().setDefaultGraphicalProperties(GraphicalProperties(TFT_WHITE, TFT_DARKCYAN, TFT_BLACK, 4, 2));
  new MessageWidget(canvas.getChild(), simple_message);
  GraphicalProperties p = canvas.getChild().getChild().getDefaultGraphicalProperties();
  p.setBorderColor(p.getBackgroundColor());
  canvas.getChild().getChild().setDefaultGraphicalProperties(p);
  p.setBackgroundColor(TFT_BLACK);
  p.setBorderColor(TFT_BLACK);
  p.setFontColor(TFT_WHITE);
  canvas.getChild().getChild().setFocusGraphicalProperties(p);
  canvas.getChild().getChild().setAcceptFocus(true);
  canvas.touch();
  canvas.print();
  MessageWidget &w = canvas.getChild().getChild<MessageWidget>();
  w.setMessage("Waiting for XIAO ESP32S3 Sense at [" + IP.toString() + "] to connect to the access point...");
  w.setWrap(true);
  w.touch();
  //canvas.loop(false);
  canvas.refresh();
  server.listen(8888);
}

void loop() {
  rButton.loop();
  lButton.loop();

  if(server.poll()){
    client = server.accept();
    }

  if(client.available()){
    client.poll();
    WebsocketsMessage msg = client.readBlocking();
    uint32_t t = millis();
    
    TJpgDec.drawJpg(-40, yPos, (const uint8_t*)msg.c_str(), msg.length());
    
    t = millis() - t;
    Serial.print(t); Serial.println(" ms");
  }
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap){
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void buttonInit(){
   rButton.setReleasedHandler(released);
   lButton.setReleasedHandler(released);
}

void released(Button2& btn) {
  if(btn == rButton){
    Serial.println("right click> ");
    moveYOffset(1);
  }else{
    Serial.println("left click> "); 
    moveYOffset(-1);
  }
}

void moveYOffset(int dir){
   yPos += (dir * MOVE_FORCE);

  if(yPos >= YPOS_MAX){
    yPos = YPOS_MAX;
  }else if(yPos <= YPOS_MIN){    
    yPos = YPOS_MIN; 
  }
  
  Serial.printf("yPos : %d\n", yPos);
}
