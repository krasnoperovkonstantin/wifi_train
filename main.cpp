#include <Arduino.h>
#include <LittleFS.h>
#include <EEPROM.h>
#include <GyverPortal.h>
 
#define EEPROM_POWER_ADDRESS 0 // 
#define WIDTH_PAGE 400
#define DEFAULT_AP_SSID "ESP"
#define DEFAULT_AP_PASS ""
#define DEFAULT_MDNS "train"
#define DEFAULT_SSID "router"
#define DEFAULT_PASSWORD "krasnoperovkonstantin"
#define DEFAULT_CONNECT_TIMEOUT 10

#define THROTTLE_PIN 4
#define LED_SOUND_PIN 5

GyverPortal portal(&LittleFS); // для проверки файлов

bool power;
int throttle = 0;

 static uint32_t tmr1;


void build() {
  GP.BUILD_BEGIN(WIDTH_PAGE); 
  GP.THEME(GP_DARK);
  GP.TITLE(DEFAULT_MDNS);
  GP.LABEL("Throttle"); 
  GP.BUTTON("btn1", "MAX");
  GP.BUTTON("btn2", "70%");
  GP.BUTTON("btn3", "30%");
  GP.BUTTON("btn4", "STOP");
  GP.BUILD_END();
}

void action() {
  if (portal.click("btn1")) throttle = 255;
  if (portal.click("btn2")) throttle = 250;
  if (portal.click("btn3")) throttle = 240;
  if (portal.click("btn4")) throttle = 0;

  analogWrite(THROTTLE_PIN, throttle);
  if (throttle == 0)
  {
    digitalWrite(LED_SOUND_PIN, 0);
  } else {
    digitalWrite(LED_SOUND_PIN, 1);
  }

  tmr1 = millis();
}


void setup() {
  Serial.begin(9600);
  
  EEPROM.begin(sizeof(power) + EEPROM_POWER_ADDRESS + 1);
  EEPROM.get(EEPROM_POWER_ADDRESS, power);

  EEPROM.put(EEPROM_POWER_ADDRESS, !power);
  EEPROM.commit();

  if (power)
  {
    Serial.println("power true");
    delay(100);
  } else {
    Serial.println("power false");
    delay(100);
    ESP.deepSleep(0); 
  }

  pinMode(THROTTLE_PIN, OUTPUT);
  pinMode(LED_SOUND_PIN, OUTPUT);
  analogWrite(THROTTLE_PIN, 0);
  digitalWrite(LED_SOUND_PIN, 1);
  delay(500);
  digitalWrite(LED_SOUND_PIN, 0);
  Serial.print("connect to ");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(DEFAULT_SSID, DEFAULT_PASSWORD);
  
  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < DEFAULT_CONNECT_TIMEOUT) {
    delay(980);
    digitalWrite(LED_SOUND_PIN, 1);
    delay(20);
    digitalWrite(LED_SOUND_PIN, 0);
    Serial.println(count);
    Serial.println(DEFAULT_SSID);
    Serial.println(DEFAULT_PASSWORD);
    count++;
  }
  Serial.println(WiFi.localIP());

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(DEFAULT_AP_SSID, DEFAULT_AP_PASS);
  }

  portal.attachBuild(build);
  portal.attach(action);
  portal.start(DEFAULT_MDNS);

  portal.enableOTA();   // без пароля

  if (!LittleFS.begin()) Serial.println("FS Error");
  portal.downloadAuto(true);

  digitalWrite(LED_SOUND_PIN, 1);

  for (throttle = 200; throttle <= 255; throttle++) {
    analogWrite(THROTTLE_PIN, throttle);
    delay(100);
  }
}

void loop() {
  portal.tick();
 
  if (millis() - tmr1 >= 300000) {
    tmr1 = millis();
      analogWrite(THROTTLE_PIN, 0);
      digitalWrite(LED_SOUND_PIN, 0);
      
      EEPROM.put(EEPROM_POWER_ADDRESS, power);
      EEPROM.commit();
      ESP.deepSleep(0); 
  }
}
