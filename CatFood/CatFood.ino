#include <Process.h>
#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <ArduinoJson.h>

Process date;

int feedPin = 7;
int hours, minutes, seconds;
char status[2];
char dayStatus[2];

int mornStart = 6;
int mornEnd = 8;
int eveStart = 17;
int eveEnd = 20;

BridgeServer server;

void setup() {
  Bridge.begin();
  Serial.begin(9600);
  server.begin();

  Console.println(F("Server started. Awaiting requests"));
  
  Bridge.put("dayStatus","0");
  getFeedState();

  pinMode(feedPin, OUTPUT);
  digitalWrite(feedPin, LOW);
  date.begin(F("date"));
  date.addParameter(F("+%T"));
  date.run();
}

void sendFeedTime(bool alertHeader=false) {
  Process p;
  String cmd = "curl 'https://stevennic.me/home?p=history&part=add";
  if(alertHeader) {
    cmd += "&alert=true'";
  } else {
    cmd += "&alert=false'";
  }
  cmd += " -H 'User-Agent: Arduino Yun' -H 'Accept: application/json'";
  p.runShellCommand(cmd);
  Console.println(cmd);
  Console.flush();
  p.close();
}

void getFeedState() {
  Process p;
  String cmd = F("curl 'https://stevennic.me/home?p=state' -H 'User-Agent: Arduino Yun' -H 'Accept: application/json'");
  p.runShellCommand(cmd);
  Console.println(cmd);
  String body = "";
  bool gotResponse = false;

  while (p.available()>0) {
    char c = p.read();
    body=body+c;
    gotResponse = true;
  }
  if (gotResponse) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(body);
    if (root.success()) {
      if (root.containsKey(F("message"))) {
        body = root[F("message")].as<String>();
        Console.println(body);
        Bridge.put("status", body);
        
        if (body.toInt() < 1) {
          Bridge.put("dayStatus","0");
        }
      }
    } else {
      Console.println(F("failed to parse JSON"));
    }
  }
  Console.flush();
  p.close();
}

void motorized(int Multiples = 1) {
  digitalWrite(feedPin, HIGH);
  delay(2700 * Multiples);
  digitalWrite(feedPin, LOW);
}

void clientReply(BridgeClient &client, String message) {
  client.println(F("Status: 200"));
  client.println(F("Content-type: application/json; charset=utf-8"));
  client.println();
  client.println("{'message': '" + message + "'}");
  client.stop();
  client.flush();
  
  Console.println("{'message': '" + message + "'}");
}

void loop() {
  BridgeClient client = server.accept();

  if (!date.running()) {
    date.begin(F("date"));
    date.addParameter(F("+%T"));
    date.run();
  }

  while (date.available() > 0) {
    String timeString = date.readString();

    int firstColon = timeString.indexOf(F(":"));
    int secondColon = timeString.lastIndexOf(F(":"));

    hours = timeString.substring(0, firstColon).toInt();
    minutes = timeString.substring(firstColon + 1, secondColon).toInt();
    seconds = timeString.substring(secondColon + 1).toInt();
  }

  if (client) {
    String command = client.readStringUntil('/');
    command.trim();
    command.toLowerCase();

    if (command.indexOf("feed") > -1) {
      int spinVal = client.parseInt();
      if (!spinVal) {
        spinVal = 1;
      }
      clientReply(client, "Cat Fed");

      motorized(spinVal);
      Bridge.put("dayStatus", "0");
      //sendFeedTime(true);
    } else if (command.indexOf("auto") > -1) {
      Bridge.get("status", status, 2);
      Bridge.get("dayStatus", dayStatus, 2);

      if (atoi(status)) {
        if ((hours >= mornStart && hours < mornEnd) || (hours >= eveStart && hours < eveEnd)) {
          if (atoi(dayStatus)) {
            clientReply(client, "Auto Feed");
            motorized();
            //sendFeedTime(true);
            Bridge.put("dayStatus", "0");
          } else {
            clientReply(client, "Already fed");
          }
        } else {
          clientReply(client, "Out of time bounds");
        }
      } else {
        clientReply(client, "Automated feeding is currently disabled. Please run 'toggle' or 'feed' command.");
      }
    } else if (command.indexOf("toggle") > -1) {
      Bridge.get("status", status, 2);
      
      if (atoi(status)) {
        Bridge.put("status", 0);
        clientReply(client, "Auto feed disabled.");
      } else {
        Bridge.put("status", 1);
        clientReply(client, "Auto feed enabled.");
      }
    } else {
      clientReply(client, "Command not recognized");
    }
  } else {
    if ((hours == mornStart || hours == eveStart) && minutes == 0){

      Bridge.get("dayStatus", dayStatus, 2);

      if (!(atoi(dayStatus)) && seconds <= 1) {
        getFeedState();
        Bridge.get("status", status, 2);

        if (atoi(status)) {
          if (hours < 12) {
            Bridge.put("dayStatus", "1");
          } else {
            Bridge.put("dayStatus", "2");
          }
        } else {
          Bridge.put("dayStatus", "0");
        }
      }
    } else if ((hours == mornEnd || hours == eveEnd) && minutes == 0) {

      Bridge.get("dayStatus", dayStatus, 2);

      if (atoi(dayStatus) > 0 && seconds <= 2) {
        getFeedState();
        Bridge.get("status", status, 2);

        if (atoi(status)) {
          motorized();
          sendFeedTime(true);
        }
        Bridge.put("dayStatus", "0");
      }
    }
  }
}
