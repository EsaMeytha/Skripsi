#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <CTBot.h>

// Fingerprint for demo URL
const uint8_t fingerprint[20] = {0xcc, 0x92, 0xd5, 0xfd, 0x41, 0x33, 0xcc, 0xc4, 0x9f, 0x7d, 0xa1, 0x5a, 0x04, 0xf1, 0x11, 0x20, 0x04, 0x3d, 0xfb, 0x48};

// cc 92 d5 fd 41 33 cc c4 9f 7d a1 5a 04 f1 11 20 04 3d fb 48

#define pinreed 5 //D1
#define pinled 4 //D2
#define pinbuzzer 15 //D8
#define SS_PIN D4
#define RST_PIN D3
#define LED_G D1 //define green LED pin
#define LED_R D2 //define red LED
#define RELAY D0 //relay pin
#define BUZZER D8 //buzzer pin
#define ACCESS_DELAY 2000
#define DENIED_DELAY 1000
#define trig D2
#define echo D8
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

//  read sensor  //
const int sensorPin = A0;
int speakerPin = D8;
int state = 0;
int pirPin = D2;
int statusPIR =0;


//object CTBot
CTBot myBot;

//konfigurasi WiFi
String ssid = "meta"; //nama WiFi
String pass = "meta12345"; //password WiFi

//konfigurasi telegram
String token = "5854484613:AAFlVvH0V1SVFOIr59kMEk54PZVKxWlPGik";
const int id = 1201585531;

WiFiClient client;

int blockNum = 2;  
long durasi;
int jarak;
/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];

String data2;
const String data1 = "https://script.google.com/macros/s/AKfycbyPELgAKjnAzUgCQNj3HYLjUFKDLR4Hs4Nkfb_9urNh/dev?name=";

//variabel untuk status sensor
int aktif = 1;  //nilai awal (1=aktif, 0=tidak aktif)

void setup() {
 pinMode(LED_BUILTIN, OUTPUT);
  pinMode(trig, OUTPUT); // Sets the trigPin as an Output
 pinMode(echo, INPUT); // Sets the echoPin as an Input

  Serial.begin(115200);  
  WiFi.disconnect();
  delay(10);
  WiFi.begin(ssid, pass);
 
  Serial.println();
  Serial.println();
  Serial.print("Terhubung dengan ");
  Serial.println(ssid);
 
  //koneksi nodemcu ke telegram melalui WiFi
  myBot.wifiConnect(ssid, pass);
  myBot.setTelegramToken(token);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
  }
  Serial.println("");
  Serial.print("NodeMcu Terhubung dengan WiFi ");
  Serial.println(ssid);
  Serial.println();
  digitalWrite(LED_BUILTIN, LOW); 

  SPI.begin();          // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  noTone(BUZZER);
  digitalWrite(RELAY, LOW);
  Serial.println("Put your card to the reader...");
  Serial.println();
}

void loop() {

  digitalWrite(trig, LOW);
  delayMicroseconds(8);
  digitalWrite(trig, HIGH);
  delayMicroseconds(8);
  digitalWrite(trig, LOW);
  delayMicroseconds(8);
  durasi = pulseIn(echo, HIGH); // menerima suara ultrasonic
  jarak = (durasi / 2) / 29.1;  // mengubah durasi menjadi jarak (cm)
  Serial.print("Distance: ");
  Serial.println(jarak);
  
  state = analogRead(sensorPin);
  statusPIR = digitalRead(pirPin);

  //  baca nilai sensor reed switch
  int reed = digitalRead(pinreed);
  Serial.println(reed);

  //baca pesan telegram yang dikirim dari hp
  TBMessage msg;

  //uji apakah ada pesan baru yang masuk ke nodemcu?
  if(myBot.getNewMessage(msg))
  {
    //baca pesan masuk
    String pesan = msg.text; //cara mengambil pesan

    //uji pesan apakah ON atau OFF

    if(pesan == "ON")
    {
      aktif = 1;
      myBot.sendMessage(msg.sender.id, "Sensor sudah diaktifkan"); //ambil id telegram yang mengirim pesan
    }
    else if(pesan == "OFF")
    {
      aktif = 0;
       myBot.sendMessage(msg.sender.id, "Sensor sudah dimatikan"); //ambil id telegram yang mengirim pesan
    }    
  }
// uji apakah variabel aktif bernilai 1
if(aktif == 1) // 
  {
    //  uji nilai sensor untuk mengaktifkan buzzer
         if (reed == 0)  //magnet menjauh
        {
          //aktif
          digitalWrite(pinbuzzer, HIGH);
          //kirim notifikasi telegram
          myBot.sendMessage(id, "Pintu Terbuka"); 
        } 
        else
        {
          digitalWrite(pinbuzzer,LOW);
        }  
  }
  delay(500); 

  {  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
  } 
   digitalWrite(LED_BUILTIN, LOW); 

   // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "22 72 09 21") //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println();
    delay(500);
    delay(ACCESS_DELAY);
    digitalWrite(RELAY, HIGH);
    
  }
 
 else   {
    Serial.println(" Access denied");
    digitalWrite(LED_R, HIGH);
    tone(BUZZER, 300);
    delay(DENIED_DELAY);
    digitalWrite(LED_R, LOW);
    digitalWrite(RELAY, LOW);
    noTone(BUZZER);
  }


// Code for Meng-aktifkan Speaker  //
if (myBot.getNewMessage(msg)) {
    Serial.println("Pesan Masuk : " + msg.text);
    String pesan = msg.text;
    if (pesan == "Test speaker") {
      myBot.sendMessage(id, "Speaker Telah Menyala");
      digitalWrite(speakerPin, HIGH);
    }
    else if (pesan == "Matikan speaker") {
      myBot.sendMessage(id, "Speaker Telah Dimatikan");
      digitalWrite(speakerPin, LOW);
    }
    else {
      myBot.sendMessage(id, "Pesan Salah, silakan coba lagi");
    }
}
    
//  Code for Sensor PIR   //
  else if (statusPIR == HIGH){
    myBot.sendMessage (id, "Waspada, ada yang mencoba membuka pintu");
    delay(1000);
    digitalWrite(speakerPin, HIGH);
    delay(5000);
    digitalWrite(speakerPin, LOW);
    delay(1000);
  }
  }
}  
