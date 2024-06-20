#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <GyverMotor2.h>
#include <GParser.h>

const char* ssid = "Сrane";             // Ваш SSID
const char* password = "12Qwerty";       // Ваш пароль
const int udpServerPort = 1234;            // Порт получателя

WiFiUDP udp;

GMotor2<DRIVER2WIRE> motor1(2, 13); //D6, D5 - Гусеница левая
GMotor2<DRIVER2WIRE> motor2(12, 14); //D4, D7 - Гусеника правая
GMotor2<DRIVER2WIRE> motor3(5, 4); //D1, D2 - Вращение крана

struct Speeds{
  int speed1; //Скорость 1 и 2 мотора
  int speed2;
};
Speeds operation;
int speed_mode[3];

Speeds Speed_value(int sp1,int sp2){
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
            operation.speed1=sp1;
            operation.speed2=sp1+sp2;
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
            if (speed_mode[2]<=-150){
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
        GParser data(incomingPacket, ',');
        int am = data.split(); 
        //Serial.printf("%s\n",incomingPacket);
        for (int i=0;i<3;i++){
          speed_mode[i]= (i==0) ? data.getInt(i): map(data.getInt(i),172,1811,-255,255);
        }
        speed_mode[1] = (speed_mode[1]>=-10 && speed_mode[1]<=10) ? 0 : speed_mode[1];
        speed_mode[2] = (speed_mode[2]>=-10 && speed_mode[2]<=10) ? 0 : speed_mode[2];
      }
      if (speed_mode[0]==0){
        operation.speed1=0;
        operation.speed2=0;
      }else{
        operation = Speed_value(speed_mode[1],speed_mode[2]);
      }

      motor1.setSpeed(operation.speed1);
      motor2.setSpeed(operation.speed2);
    }
  }else{
      operation.speed1=0;
      operation.speed2=0;
  }

  delay(10);
}