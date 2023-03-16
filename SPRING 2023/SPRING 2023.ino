#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <math.h>
#include <time.h>

#define WIFI_SSID "****"
#define WIFI_PASSWORD "****"
#define BOT_TOKEN "****"

const unsigned long BOT_MTBS = 1000;
unsigned long bot_lasttime;
float Po;
float pH_Step;
float PH7 = ;
float PH4 = ;
int dst;
int pH_Value;
int timezone = 7*3600;
double voltpH;

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void setup()
{
  pinMode(A0, INPUT);
  Serial.begin(115200);
  Serial.println();
  configTime(0, 0, "pool.ntp.org");
  secured_client.setTrustAnchors(&cert);
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for local time");
  while(!time(nullptr))
  {
    Serial.print("*");
    delay(1000); 
  }
  Serial.println("\nTime response...");
  bot_setup();
}

void loop()
{
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  int h = p_tm->tm_hour;
  int m = p_tm->tm_min;
  int s = p_tm->tm_sec;
  for(int i = 0; i < 800; i++)
  {
    pH_Value = analogRead(A0);
    /*pH Sensor Input 5V*/
    voltpH = (5 / 1024.0) * pH_Value;
    pH_Step = (PH4 - PH7) / 3;
    Po += 7.00 + ((PH7 - voltpH) / pH_Step);
  }
  Po = Po / 800;
  timeClock(h, m, s);
  Serial.print(" ");
  Serial.println(Po);
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      timeClock(h, m, s);
      Serial.println(" Got response");
      command(numNewMessages, Po, h, m, s);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
  Po = 0;
  delay(3000);
}

void command(int numNewMessages, float Po, int h, int m, int s)
{
  timeClock(h, m, s);
  Serial.println(" Handle ");
  timeClock(h, m, s);
  Serial.print(" ");
  Serial.println(numNewMessages);
  timeClock(h, m, s);
  Serial.println(" message");
  String answer;
  for (int i = 0; i < numNewMessages; i++)
  {
    telegramMessage &msg = bot.messages[i];
    timeClock(h, m, s);
    Serial.println(" Received " + msg.text);
    if (msg.text == "/help")
      answer = "Need help? use /start or /status";
    else if (msg.text == "/start")
      answer = "Welcome *" + msg.from_name + "*.";
    else if (msg.text == "/status")
      answer = "Online...";
    else if (msg.text == "/data")
      if (Po < 6.5 or Po > 8.5)
      {
        answer = "pH = *" + String(Po, 2) + "*\nClean the pool immediately!";
      }
      else
      {
        answer = "pH = *" + String(Po, 2) + "*";
      }
    bot.sendMessage(msg.chat_id, answer, "Markdown");
  }
}

void bot_setup()
{
  const String commands = F("["
                            "{\"command\":\"help\",  \"description\":\"Get bot usage help\"},"
                            "{\"command\":\"start\", \"description\":\"Message sent when you open a chat with a bot\"},"
                            "{\"command\":\"status\",\"description\":\"Answer device current status\"},"
                            "{\"command\":\"data\",\"description\":\"Send output data\"}"
                            "]");
  bot.setMyCommands(commands);
}

void timeClock(int h, int m, int s)
{
  /*Time output*/
  Serial.print(h);
  Serial.print(":");
  Serial.print(m);
  Serial.print(":");
  Serial.print(s);
}

float round_to_dp(float in_value, int decimal_place)
{
  float multiplier = powf(10.0f, decimal_place);
  in_value = roundf(in_value * multiplier) / multiplier;
  return in_value;
}
