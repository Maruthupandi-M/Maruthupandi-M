#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <RTClib.h>
#include <time.h>

const char* ssid = "TEAM";
const char* password = "";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

#define BOT_TOKEN "8670129488:AAH5THhd1r83LjgqJj7yh12WArgR9_bp130"
#define CHAT_ID "6385119473"

const int vibrationPin = 12;
const int ledPin = 2;
const int stopButtonPin = 13;
const int snoozeButtonPin = 14;

RTC_DS3231 rtc;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

int alarmHour = 0;
int alarmMinute = 0;
int alarmSecond = 0;
bool alarmSet = false;

bool alarmRinging = false;
unsigned long alarmStartTime = 0;
const unsigned long alarmDuration = 60000;

bool snoozeActive = false;
unsigned long snoozeStartTime = 0;
const unsigned long snoozeDuration = 30000;

unsigned long lastTimeBotRan = 0;
const unsigned long botRequestDelay = 2000;

unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 300;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  printHeader();
  
  pinMode(vibrationPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(stopButtonPin, INPUT_PULLUP);
  pinMode(snoozeButtonPin, INPUT_PULLUP);
  
  digitalWrite(vibrationPin, LOW);
  digitalWrite(ledPin, LOW);
  
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("❌ RTC not found! Check wiring.");
    while (1);
  }
  Serial.println("✅ RTC initialized!");
  
  if (rtc.lostPower()) {
    Serial.println("⚠️ RTC lost power! Setting default time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  connectWiFi();
  
  if (WiFi.status() == WL_CONNECTED) {
    syncRTCfromNTP();
  }
  
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  bot.sendMessage(CHAT_ID, "🛏️ Smart Pillow Alarm System ONLINE!\nUse /help for commands", "");
  
  printCurrentTime();
  printInstructions();
}

void loop() {
  handleButtons();
  handleSerialInput();
  handleTelegramMessages();
  checkAlarm();
  handleAlarmRinging();
  handleSnooze();
  
  delay(50);
}

void connectWiFi() {
  Serial.print("📡 Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Wi-Fi Connected!");
  } else {
    Serial.println("\n❌ Wi-Fi Failed!");
  }
}

void syncRTCfromNTP() {
  Serial.println("\n🕐 Syncing RTC with NTP server...");
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (getLocalTime(&timeinfo)) {
    rtc.adjust(DateTime(timeinfo.tm_year + 1900, 
                        timeinfo.tm_mon + 1, 
                        timeinfo.tm_mday, 
                        timeinfo.tm_hour, 
                        timeinfo.tm_min, 
                        timeinfo.tm_sec));
    Serial.println("\n✅ RTC synchronized with NTP server!");
    printCurrentTime();
  } else {
    Serial.println("\n❌ Failed to get time from NTP!");
  }
}

DateTime getCurrentTime() {
  return rtc.now();
}

void printCurrentTime() {
  DateTime now = rtc.now();
  Serial.print("\n🕒 Current Time: ");
  Serial.print(now.hour());
  Serial.print(":");
  if (now.minute() < 10) Serial.print("0");
  Serial.print(now.minute());
  Serial.print(":");
  if (now.second() < 10) Serial.print("0");
  Serial.println(now.second());
}

String getCurrentTimeString() {
  DateTime now = rtc.now();
  char buffer[9];
  sprintf(buffer, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  return String(buffer);
}

void setRTCtime(int hour, int minute, int second) {
  DateTime now = rtc.now();
  rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, second));
  Serial.println("✅ RTC time updated!");
  printCurrentTime();
}

void checkAlarm() {
  if (alarmRinging) return;
  if (snoozeActive) return;
  if (!alarmSet) return;
  
  DateTime now = rtc.now();
  int currentHour = now.hour();
  int currentMinute = now.minute();
  int currentSecond = now.second();
  
  if (currentHour == alarmHour && currentMinute == alarmMinute && currentSecond >= alarmSecond) {
    triggerAlarm();
  }
}

void triggerAlarm() {
  Serial.println("\n");
  Serial.println("╔════════════════════════════════════════════╗");
  Serial.println("║     🔔 ALARM TRIGGERED! 🔔                 ║");
  Serial.println("║     📳 VIBRATION MOTOR ACTIVE              ║");
  Serial.println("╚════════════════════════════════════════════╝");
  
  alarmRinging = true;
  alarmStartTime = millis();
  digitalWrite(vibrationPin, HIGH);
  digitalWrite(ledPin, HIGH);
  
  bot.sendMessage(CHAT_ID, "🔔 ALARM TRIGGERED!\nTime: " + getCurrentTimeString(), "");
  
  Serial.println("📳 Vibration motor ACTIVE!");
  Serial.println("📌 Press STOP button (GPIO13) to cancel");
  Serial.println("📌 Press SNOOZE button (GPIO14) for 30 seconds");
}

void handleAlarmRinging() {
  if (alarmRinging) {
    if (millis() - alarmStartTime >= alarmDuration) {
      stopAlarm();
      Serial.println("⏰ Alarm stopped automatically after 1 minute");
    }
  }
}

void stopAlarm() {
  digitalWrite(vibrationPin, LOW);
  digitalWrite(ledPin, LOW);
  alarmRinging = false;
  alarmSet = false;
  snoozeActive = false;
  
  Serial.println("\n✅ Alarm STOPPED");
  bot.sendMessage(CHAT_ID, "✅ Alarm stopped", "");
}

void snoozeAlarm() {
  if (!alarmRinging) {
    Serial.println("ℹ️ No active alarm to snooze");
    bot.sendMessage(CHAT_ID, "ℹ️ No active alarm to snooze", "");
    return;
  }
  
  digitalWrite(vibrationPin, LOW);
  digitalWrite(ledPin, LOW);
  alarmRinging = false;
  
  snoozeActive = true;
  snoozeStartTime = millis();
  
  Serial.println("\n╔════════════════════════════════════════════╗");
  Serial.println("║     😴 SNOOZE ACTIVATED!                   ║");
  Serial.println("║     ⏰ Alarm will ring again in 30 seconds ║");
  Serial.println("╚════════════════════════════════════════════╝");
  
  bot.sendMessage(CHAT_ID, "😴 Snooze activated! Alarm will ring again in 30 seconds.", "");
}

void handleSnooze() {
  if (!snoozeActive) return;
  
  if (millis() - snoozeStartTime >= snoozeDuration) {
    snoozeActive = false;
    
    Serial.println("\n⏰ SNOOZE TIME OVER! Ringing again...");
    bot.sendMessage(CHAT_ID, "⏰ Snooze time over! Alarm ringing again...", "");
    
    alarmRinging = true;
    alarmStartTime = millis();
    digitalWrite(vibrationPin, HIGH);
    digitalWrite(ledPin, HIGH);
    
    Serial.println("📳 Vibration motor ACTIVE again!");
  }
}

void setAlarm(int hour, int minute, int second) {
  if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59) {
    alarmHour = hour;
    alarmMinute = minute;
    alarmSecond = second;
    alarmSet = true;
    
    snoozeActive = false;
    if (alarmRinging) {
      stopAlarm();
    }
    
    Serial.println("\n✅ ALARM SET SUCCESSFULLY!");
    Serial.print("⏰ Alarm Time: ");
    Serial.print(alarmHour);
    Serial.print(":");
    if (alarmMinute < 10) Serial.print("0");
    Serial.print(alarmMinute);
    Serial.print(":");
    if (alarmSecond < 10) Serial.print("0");
    Serial.println(alarmSecond);
    
    printTimeUntilAlarm();
    
    String message = "✅ Alarm set successfully!\n";
    message += "⏰ Time: " + String(alarmHour) + ":" + String(alarmMinute) + ":" + String(alarmSecond);
    bot.sendMessage(CHAT_ID, message, "");
  } else {
    Serial.println("\n❌ Invalid time!");
    bot.sendMessage(CHAT_ID, "❌ Invalid time! Use: /set HH:MM:SS", "");
  }
}

void setAlarm(int hour, int minute) {
  setAlarm(hour, minute, 0);
}

void disableAlarm() {
  alarmSet = false;
  if (alarmRinging) {
    stopAlarm();
  }
  snoozeActive = false;
  Serial.println("\n🔕 Alarm DISABLED!");
  bot.sendMessage(CHAT_ID, "🔕 Alarm disabled!", "");
}

void printTimeUntilAlarm() {
  if (!alarmSet) return;
  
  DateTime now = rtc.now();
  int totalCurrentSeconds = now.hour() * 3600 + now.minute() * 60 + now.second();
  int totalAlarmSeconds = alarmHour * 3600 + alarmMinute * 60 + alarmSecond;
  
  int secondsUntil = totalAlarmSeconds - totalCurrentSeconds;
  if (secondsUntil < 0) {
    secondsUntil += 86400;
  }
  
  Serial.print("⏳ Time until alarm: ");
  Serial.print(secondsUntil / 3600);
  Serial.print("h ");
  Serial.print((secondsUntil % 3600) / 60);
  Serial.print("m ");
  Serial.print(secondsUntil % 60);
  Serial.println("s");
}

void handleButtons() {
  unsigned long currentTime = millis();
  
  if (digitalRead(snoozeButtonPin) == LOW) {
    if (currentTime - lastButtonPressTime > debounceDelay) {
      lastButtonPressTime = currentTime;
      
      Serial.println("\n🔘 SNOOZE button pressed!");
      snoozeAlarm();
      
      while(digitalRead(snoozeButtonPin) == LOW) {
        delay(50);
      }
    }
  }
  
  if (digitalRead(stopButtonPin) == LOW) {
    if (currentTime - lastButtonPressTime > debounceDelay) {
      lastButtonPressTime = currentTime;
      
      Serial.println("\n🔘 STOP button pressed!");
      if (alarmRinging || snoozeActive) {
        stopAlarm();
        Serial.println("🛑 Alarm stopped by STOP button");
      } else {
        Serial.println("ℹ️ No active alarm to stop");
      }
      
      while(digitalRead(stopButtonPin) == LOW) {
        delay(50);
      }
    }
  }
}

void handleTelegramMessages() {
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    while (numNewMessages) {
      for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        
        if (chat_id != CHAT_ID) {
          bot.sendMessage(chat_id, "❌ Unauthorized user!", "");
          continue;
        }
        
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;
        
        Serial.println("\n📨 Telegram: " + text);
        
        if (text == "/start") {
          String welcome = "🛏️ Smart Pillow Alarm System!\n\n";
          welcome += "/set HH:MM:SS - Set alarm\n";
          welcome += "/settime HH:MM:SS - Set time\n";
          welcome += "/off - Disable alarm\n";
          welcome += "/status - Check status\n";
          welcome += "/time - Show time\n";
          welcome += "/stop - Stop alarm\n";
          welcome += "/snooze - Snooze 30 sec\n";
          welcome += "/sync - Sync with NTP\n";
          welcome += "/help - Commands";
          bot.sendMessage(CHAT_ID, welcome, "");
        }
        
        else if (text == "/sync") {
          if (WiFi.status() == WL_CONNECTED) {
            syncRTCfromNTP();
          } else {
            bot.sendMessage(CHAT_ID, "❌ Wi-Fi not connected!", "");
          }
        }
        
        else if (text.startsWith("/settime ")) {
          String timeStr = text.substring(9);
          int h, m, s;
          if (sscanf(timeStr.c_str(), "%d:%d:%d", &h, &m, &s) == 3) {
            setRTCtime(h, m, s);
            bot.sendMessage(CHAT_ID, "✅ Time: " + getCurrentTimeString(), "");
          } else {
            bot.sendMessage(CHAT_ID, "❌ Use: /settime HH:MM:SS", "");
          }
        }
        
        else if (text.startsWith("/set ")) {
          String timeStr = text.substring(5);
          int firstColon = timeStr.indexOf(':');
          int secondColon = timeStr.indexOf(':', firstColon + 1);
          
          if (firstColon > 0) {
            int hour = timeStr.substring(0, firstColon).toInt();
            
            if (secondColon > 0) {
              int minute = timeStr.substring(firstColon + 1, secondColon).toInt();
              int second = timeStr.substring(secondColon + 1).toInt();
              setAlarm(hour, minute, second);
            } else {
              int minute = timeStr.substring(firstColon + 1).toInt();
              setAlarm(hour, minute, 0);
            }
          } else {
            bot.sendMessage(CHAT_ID, "❌ Use: /set HH:MM or /set HH:MM:SS", "");
          }
        }
        
        else if (text == "/off") {
          disableAlarm();
        }
        
        else if (text == "/status") {
          String status = "📊 Status:\n";
          status += "🕒 Time: " + getCurrentTimeString() + "\n";
          status += alarmSet ? "⏰ Alarm: SET at " + String(alarmHour) + ":" + String(alarmMinute) : "⏰ Alarm: NOT SET";
          if (alarmRinging) status += "\n🔔 RINGING!";
          if (snoozeActive) status += "\n😴 Snooze active";
          bot.sendMessage(CHAT_ID, status, "");
        }
        
        else if (text == "/time") {
          bot.sendMessage(CHAT_ID, "🕒 " + getCurrentTimeString(), "");
        }
        
        else if (text == "/stop") {
          if (alarmRinging) {
            stopAlarm();
            bot.sendMessage(CHAT_ID, "🛑 Stopped!", "");
          } else {
            bot.sendMessage(CHAT_ID, "ℹ️ No alarm ringing", "");
          }
        }
        
        else if (text == "/snooze") {
          snoozeAlarm();
        }
        
        else if (text == "/help") {
          String help = "🔧 Commands:\n";
          help += "/set HH:MM:SS\n/settime HH:MM:SS\n/off\n/status\n/time\n/stop\n/snooze\n/sync";
          bot.sendMessage(CHAT_ID, help, "");
        }
      }
      
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    
    lastTimeBotRan = millis();
  }
}

void handleSerialInput() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toLowerCase();
    
    if (input.startsWith("set ")) {
      String timeStr = input.substring(4);
      int firstColon = timeStr.indexOf(':');
      int secondColon = timeStr.indexOf(':', firstColon + 1);
      
      if (firstColon > 0) {
        int hour = timeStr.substring(0, firstColon).toInt();
        if (secondColon > 0) {
          int minute = timeStr.substring(firstColon + 1, secondColon).toInt();
          int second = timeStr.substring(secondColon + 1).toInt();
          setAlarm(hour, minute, second);
        } else {
          int minute = timeStr.substring(firstColon + 1).toInt();
          setAlarm(hour, minute, 0);
        }
      }
    }
    else if (input.startsWith("settime ")) {
      String timeStr = input.substring(8);
      int h, m, s;
      if (sscanf(timeStr.c_str(), "%d:%d:%d", &h, &m, &s) == 3) {
        setRTCtime(h, m, s);
      }
    }
    else if (input == "sync") {
      if (WiFi.status() == WL_CONNECTED) {
        syncRTCfromNTP();
      }
    }
    else if (input == "off") {
      disableAlarm();
    }
    else if (input == "status") {
      showStatus();
    }
    else if (input == "time") {
      printCurrentTime();
    }
    else if (input == "stop") {
      if (alarmRinging) stopAlarm();
    }
    else if (input == "snooze") {
      snoozeAlarm();
    }
    else if (input == "test") {
      Serial.println("🧪 Testing vibration...");
      digitalWrite(vibrationPin, HIGH);
      digitalWrite(ledPin, HIGH);
      delay(2000);
      digitalWrite(vibrationPin, LOW);
      digitalWrite(ledPin, LOW);
      Serial.println("✅ Test complete");
    }
    else if (input == "help") {
      printInstructions();
    }
  }
}

void showStatus() {
  Serial.println("\n╔════════════════════════════════════════════╗");
  Serial.println("║        SYSTEM STATUS                       ║");
  Serial.println("╚════════════════════════════════════════════╝");
  
  DateTime now = rtc.now();
  Serial.print("🕒 Time: ");
  Serial.print(now.hour());
  Serial.print(":");
  if (now.minute() < 10) Serial.print("0");
  Serial.print(now.minute());
  Serial.print(":");
  if (now.second() < 10) Serial.print("0");
  Serial.println(now.second());
  
  Serial.print("📶 Wi-Fi: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  
  Serial.print("⏰ Alarm: ");
  Serial.print(alarmSet ? "SET at " : "NOT SET");
  if (alarmSet) {
    Serial.print(alarmHour);
    Serial.print(":");
    if (alarmMinute < 10) Serial.print("0");
    Serial.print(alarmMinute);
    Serial.print(":");
    if (alarmSecond < 10) Serial.print("0");
    Serial.print(alarmSecond);
    Serial.println();
    printTimeUntilAlarm();
  } else {
    Serial.println();
  }
  
  Serial.print("🔔 Ringing: ");
  Serial.println(alarmRinging ? "YES" : "NO");
  
  Serial.print("😴 Snooze: ");
  Serial.println(snoozeActive ? "ACTIVE" : "INACTIVE");
  if (snoozeActive) {
    Serial.print("   Remaining: ");
    Serial.print((snoozeDuration - (millis() - snoozeStartTime)) / 1000);
    Serial.println(" seconds");
  }
  
  Serial.println("════════════════════════════════════════════\n");
}

void printHeader() {
  Serial.println("\n");
  Serial.println("╔══════════════════════════════════════════════╗");
  Serial.println("║     SMART PILLOW - SILENT WAKE-UP ALARM      ║");
  Serial.println("║     ESP32 + RTC DS3231 + TELEGRAM BOT        ║");
  Serial.println("╚══════════════════════════════════════════════╝");
}

void printInstructions() {
  Serial.println("\n📌 Commands:");
  Serial.println("   set HH:MM:SS  - Set alarm");
  Serial.println("   settime HH:MM:SS - Set RTC time");
  Serial.println("   off - Disable alarm");
  Serial.println("   status - Show status");
  Serial.println("   time - Show time");
  Serial.println("   stop - Stop alarm");
  Serial.println("   snooze - Snooze 30 sec");
  Serial.println("   test - Test motor");
  Serial.println("\n🔘 Buttons:");
  Serial.println("   GPIO13 (STOP) - Stop alarm");
  Serial.println("   GPIO14 (SNOOZE) - Snooze 30 sec");
  Serial.println();
}
