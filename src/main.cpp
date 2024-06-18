#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <GyverMotor2.h>
#include <GParser.h>

const char* ssid = "Сrane";             // Ваш SSID
const char* password = "12Qwerty";       // Ваш пароль
const int udpServerPort = 1234;            // Порт получателя

WiFiUDP udp;

GMotor2<DRIVER2WIRE> motor1(12, 14); //D1, D2 - Гусеница левая
GMotor2<DRIVER2WIRE> motor2(2, 13); //D4, D7 - Гусеника правая
GMotor2<DRIVER2WIRE> motor3(5, 4); //D6, D5 - Вращение крана

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Подключение к WiFi...");
   
  }
  Serial.println("ПОДКЛЮЧИЛСЯ");
  Serial.println(WiFi.localIP());

  udp.begin(udpServerPort); // Инициализация UDP для приема данных

  motor1.setMinDuty(100);   // мин. ШИМ
  motor1.reverse(1);     // реверс

  motor2.setMinDuty(100);   
  motor2.reverse(1);     

  motor3.setMinDuty(120);   
  motor3.reverse(1);    
}

void loop() {
  motor1.tick();
  motor2.tick();
  motor3.tick();
  int speed;
  int mode;
  
  if (WiFi.status() == WL_CONNECTED) {
    // Прием данных по UDP
    char incomingPacket[255];
    int packetSize = udp.parsePacket();
    if (packetSize) {
      int len = udp.read(incomingPacket, 255);
      if (len > 0) {
        incomingPacket[len] = 0;
        GParser data(incomingPacket, ',');
        int am = data.split(); 
        //Serial.printf("%s\n",incomingPacket);
        mode=data.getInt(0);
        speed=map(data.getInt(1),172,1811,-255,255);
        speed = (speed>=-2 && speed<=2) ? 0 : speed;   //условие Тернарный оператор
        Serial.println(speed);
      }
    }
    if (mode==0){
      speed=0;
    }else if (mode==1){
      motor1.setSpeed(speed);
      motor2.setSpeed(speed);
    }else if (mode==2){
      motor1.setSpeed(speed);
      motor2.setSpeed((-1)*speed);
    }
  }else{speed=0;}

  delay(10);
}