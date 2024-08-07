#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <GyverMotor2.h>
#include <GParser.h>

const char* ssid = "Сrane";             // Ваш SSID
const char* password = "12Qwerty";       // Ваш пароль
const int udpServerPort = 1234;            // Порт получателя

WiFiUDP udp;

GMotor2<DRIVER2WIRE> motor1(2, 16); //D4, tx - Гусеница левая
GMotor2<DRIVER2WIRE> motor2(12, 14); //D5, D6 - Гусеника правая
GMotor2<DRIVER2WIRE> motor3(5, 4); //D1, D2 - Вращение крана

struct Speeds{
  int speed1;
  int speed2;
  int speed3;
};
Speeds operation;
int speed_mode[3];

Speeds Speed_value(int sp1,int sp2, int sp3){

  if (sp1>=0 && sp2>=0){
      if (sp1>=100){
        if (abs(sp1)>=abs(sp2)){
          operation.speed1=sp1;
        }else{
          operation.speed1=sp2;
        }
          operation.speed2=sp1-sp2;
      }else{
        if (sp2>=150){
          operation.speed1=sp2;
          operation.speed2=-sp2;
        }else{
          operation.speed1=0;
          operation.speed2=0;
        }
      }
  }else if (sp1<0 && sp2>=0){
      if (sp1<=-100){
        if (sp2>=245){
          operation.speed1=sp1;
          operation.speed2=0;
        }
        else{
          operation.speed1=sp1;
          operation.speed2=sp1+sp2;
        }
      }else{
        if (sp2>=150){
          operation.speed1=sp2;
          operation.speed2=-sp2;
          
        }else{
          operation.speed1=0;
          operation.speed2=0;
        }
      }
  }else if (sp1<=0 && sp2<0){
      if (sp1<=-100){
        if (abs(sp1)>=abs(sp2)){
          operation.speed2=sp1;
        }else{
          operation.speed2=sp2;
        }
          operation.speed1=sp1-sp2;
      }else{
        if (sp2<=-150){
          operation.speed1=sp2;
          operation.speed2=-sp2;
        }else{
          operation.speed1=0;
          operation.speed2=0;
        }
      }
  }else if (sp1>0 && sp2<0){
    if (sp1>=100){
      operation.speed1=sp1+sp2;
      operation.speed2=sp1;
    }else{
      if (sp2<=-150){
        operation.speed1=sp2;
        operation.speed2=-sp2;
      }else{
        operation.speed1=0;
        operation.speed2=0;
      }
    }
  }
  operation.speed3=sp3;
  return operation;
}

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

  if (WiFi.status() == WL_CONNECTED) {
    // Прием данных по UDP
    char incomingPacket[255];
    int packetSize = udp.parsePacket();
    if (packetSize) {
      int len = udp.read(incomingPacket, 255);
      if (len > 0) {
        incomingPacket[len] = 0;
        //Serial.printf("%s\n",incomingPacket);
        GParser data(incomingPacket, ',');
        int am = data.split(); 
        for (int i=0;i<3;i++){
          speed_mode[i]= data.getInt(i);
        }
        speed_mode[0] = (speed_mode[0]>=-10 && speed_mode[0]<=10) ? 0 : speed_mode[0];
        speed_mode[1] = (speed_mode[1]>=-10 && speed_mode[1]<=10) ? 0 : speed_mode[1];
        speed_mode[2] = (speed_mode[2]>=-10 && speed_mode[2]<=10) ? 0 : speed_mode[2];
      }

      operation = Speed_value(speed_mode[0],speed_mode[1],speed_mode[2]);
      
      Serial.print(operation.speed1);
      Serial.print(" ");
      Serial.print(operation.speed2);
      Serial.print(" ");
      Serial.print(operation.speed3);
      Serial.print(" ");
      Serial.println();

      motor1.setSpeed(operation.speed1);
      motor2.setSpeed(operation.speed2);
      motor3.setSpeed(operation.speed3);
    }
  }else{
      operation.speed1=0;
      operation.speed2=0;
      operation.speed3=0;
  }

  delay(10);
}