/*##############################################################################
#   Smart Home Display                                                         #
#   Last change:   22.10.2024                                                  #
#   Version:       0.3                                                         #
#   Author:        tobo-123                                                    #
#   Github:        tobo-123                                                    # 
################################################################################
  
Copyright (c) 2024 tobo-123

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

#################################################################################

Change log:
V0.1
- First version
- Client registration
V0.2
- With blinking mode
V0.3
- Interprets more than one state change in a server message. Changed variable names
- fixed bug at re-subscribe polling

##################################################################################*/

#include <Arduino.h>
#include <ESP8266WiFi.h> 
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

struct param {String name; int pin; int brightness; bool blinking; };

/*##################################################################################
#####   !!!!!        Adjust the following entries        !!!!!    ##################
##################################################################################*/

bool register_client = true;                                                  //keep true for the first run. When client is registered, change to false and re-upload the program to your ESP

const char* WIFI_ssid = "#########";                                          //your Wifi name
const char* WIFI_password = "#########";                                      //your Wifi password
const String BSH_ip = "192.168.0.xx";                                         //IP adress of your BSH controller
const String BSH_password = "###########";                                    //Your BSH controller password in base64 coding

// GPIO pin numbers, please check or change

const int Pin_Led1 = 5;   // #1 LED, e.g., red
const int Pin_Led2 = 4;   // #2 LED, e.g., blue
const int Pin_Led3 = 0;   // #3 LED, e.g., yellow
const int Pin_Led4 = 12;  // #4 LED, e.g., white

const int number_entries = 4;                                                 //Number of entries in the parameter table. Please change if you have more/less entries

const param parameter[] = {{"Window open", Pin_Led1, 200, false},             //This table connects the userdefinedstates of your BSH with your LED. The entries are just examples, you need to change them
                           {"Alarm reminder", Pin_Led2, 25, false},           //Scheme: "name of your userdefined state", Pin_Led_Number, brightness level (0-255), flashing (true/false)
                           {"Post box", Pin_Led3, 255, true},     
                           {"Light on", Pin_Led4, 12, false}};

//Enter your client_cert.pem here

const char* client_cert PROGMEM = R"string_literal(
-----BEGIN CERTIFICATE-----
MIIDcTCCAlmgAwIBAgIUH0Pbupca1qIOwHtCydADDGXs3YEwDQYJKoZIhvcNAQEL
BQAwRzELMAkGA1UEBhMCREUxCzAJBgNVBAgMAkJFMQ8wDQYDVQQHDAZCZXJsaW4x
DDAKBgNVBAoMA0VTUDEMMAoGA1UEAwwDRVNQMCAXDTI0MDkzMDEwMjc0NloYDzIw
some lines removed
c80uWOy1yVrpsqty3GBJcPsMO9f/m4nQHqGR5m/Z9DuFt5DtiQAG4bqYIEzpboia
NbwB8X8eJIXGjLYfl7x6G7Yo/BlVX2qivaOT710Ms1szgEFy6449hdXlPSfVA3tS
DcjLQmyvWrQWtd9CDBkzP5PLG5KM2AJ0Z+cd1nXeO+nNw5l1lWmRBmDTcr1Loa6u
vcFfUC1/fuRDlPbpxDfTdnTEepOGrCmWF/dUJoFFtMkB/HvBsKwE+OlcBx5AdeJv
VfDKD7Y1WIuARPFjvp/4QTwQvNoW
-----END CERTIFICATE-----
)string_literal";

//Enter your client_key.pem here

const char* client_key PROGMEM = R"string_literal(
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDtA8MhKS5Q5JED
MNHFjHzANUL7qdnVlM7BFPLk18R4GHXONNAqXjrmGB7DP50AIoLmYo+UC0nALv8X
/3iH878vgLZYH+WalrVsnUHQGkhGkkoT/yXWMBq5jePddfPtZL3mrpa9yU8RIJuq
some lines removed
eiX+/TIlKCjKF71qz5bhQtHfhrPiHjPFvXU4Vi6F7QKBgGZ9xO3pUAVkoB6Kothl
i90SiabAh3FxACqZyxR7u3TNnxgn6LHHjT/srIcpUPok+3rmZT2Bdymq75HvTmA7
9XWH57UgOCpjFmPhg0MN48qCZEMPHKprxWkTtyDLNMpL7JYI1ILbYKudbpZIOAme
EGCBICaGpxbFwcb++//rUbIu
-----END PRIVATE KEY-----
)string_literal";


/*##################################################################################
#####   !!!!!  Nothings needs to be changed after here   !!!!!    ##################
##################################################################################*/


const char* BSH_root_CA PROGMEM = R"string_literal(
-----BEGIN CERTIFICATE-----
MIIFETCCAvmgAwIBAgIUR8y7kFBqVMZCYZdSQWVuVJgSAqYwDQYJKoZIhvcNAQEL
BQAwYzELMAkGA1UEBhMCREUxITAfBgNVBAoMGEJvc2NoIFRoZXJtb3RlY2huaWsg
R21iSDExMC8GA1UEAwwoU21hcnQgSG9tZSBDb250cm9sbGVyIFByb2R1Y3RpdmUg
Um9vdCBDQTAeFw0xNTA4MTgwNzI0MjFaFw0yNTA4MTYwNzI0MjFaMFsxCzAJBgNV
BAYTAkRFMSEwHwYDVQQKDBhCb3NjaCBUaGVybW90ZWNobmlrIEdtYkgxKTAnBgNV
BAMMIFNtYXJ0IEhvbWUgQ29udHJvbGxlciBJc3N1aW5nIENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsBNK3PPd/E9jbf3YkZIDtfIl2Vo0Nx7oeOsh
F0L9tZwqC3+85ymB5LgFBOoHpr7tTFRb4elyPsfyv/GfXuJmDIxVAWBn/pxFzODa
J3DGJ2kvwipvMNp7IxXHhK10YsG8AaT0QaeaYGq1GRp5uNZafwAOOkrrQfwtG+za
Qn9qUxLYBrB++RN/5mk4Z7gyrq7fi84T23yMOtVkdb+mlb9qStQ3mllglqrRlJQo
MKdQxe24Farg6N3y7h5bxLJEEXGqGExDNwR46ep+4Ys7W2QeD/2LXwYvKQ+wO70+
BNxnikkq8kPcq8694HMsfzUTBrxuHQGi6td9o+3CW01AOEvV0wIDAQABo4HEMIHB
MBIGA1UdEwEB/wQIMAYBAf8CAQEwHQYDVR0OBBYEFHy1ci5zZEQaHLDAaYFYez8R
FHsXMB8GA1UdIwQYMBaAFOFQaxE4w2eoyE+f6oXGTxH1V1Y+MA4GA1UdDwEB/wQE
AwIBBjBbBgNVHR8EVDBSMFCgTqBMhkpodHRwczovLzI5Lm1jZy5lc2NyeXB0LmNv
bS9jcmw/aWQ9ZTE1MDZiMTEzOGMzNjdhOGM4NGY5ZmVhODVjNjRmMTFmNTU3NTYz
ZTANBgkqhkiG9w0BAQsFAAOCAgEAZpp9kE7Qy6tcQrfW4DJAqEcUhzg4zncJYxpb
dTn/o5TvH/uPVOfoxJgtsTFtsY/ytcPJReLcgmqrRN1gTNefdXylJr688hFyhf1Z
xGDoZG8MuzM9QXaHC6UNFzaeZj46ZYfdJiUtDXsYN82opGE6GhBju5JOLoFd2vYK
qUnVKWqdrN0KkihClry6NcfiLEA70m00pNtsVZyVGyk7DP4ErVF5K3j40T5v4ZJl
Q9ri/V97zyqXeIti8kZdla7kzJBFbGEumlUyVPRpoxdpnvWM7AgTOXXsh2sCFAA1
0hUHVOwBZCylaNUXjKMtnA938ykhNCx+OCd2NpZBf3qB6+w2MS7dQuRvMsDJcnLq
X80QHJzXpmDsXEiwKyvmZnZbiAgoOiUSe2O6OaGsDRW8UBzi+wm42pxgbDnAcGUu
r9Cf5y0+SFS0aQkqcWbJYwPy+LQi2MJGkv34FxTOCqygluzZt+w5xZyq5PcpPNm5
1s4Ps2+psvNhcAG3EHRF9vBnlr1MCVU04XYig54HeNGFIQQAFWFFR/9DgnH/cFLf
gPoJEZV/VZtsOjy/EsqYZMFJBzJEtKOiTCKDe+pVirDB9zrcVsJG8LGiLd7266e9
1Eg5GjNiavG7ninMOWSJLfW4xPD6S3zxDAYjsPDJbMFqEFIF2ZvyYC1mVeflB/WM
xnZ+67w=
-----END CERTIFICATE-----
)string_literal";


//Variables and objects

JsonDocument doc;
JsonArray array;
const char* name;
const char* error_message;
bool state;
long error_code;

String json = "";
String output;
String polling_id;
String line;

char c;

bool subscribe_poll = true;
bool poll = true;

int firstindex;
int lastindex;
unsigned long current_time = 0;
unsigned long prev_time = 0;

struct tm timeinfo;

IPAddress IP;

bool state_array[number_entries] = {false};

WiFiClientSecure *client = new WiFiClientSecure;


// Setup -------------------------------------------------------------------------------------------

void setup() {

  // Setup serial connection 

  Serial.begin(115200);
  while(!Serial) {
     delay(500);
  };
 

  // pin assignment  

  pinMode(Pin_Led1, OUTPUT);
  pinMode(Pin_Led2, OUTPUT);
  pinMode(Pin_Led3, OUTPUT);
  pinMode(Pin_Led4, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // check all LED at start-up

  digitalWrite(LED_BUILTIN, HIGH);

  digitalWrite(Pin_Led1, HIGH);  
  delay (500);
  digitalWrite(Pin_Led1, LOW);  
  digitalWrite(Pin_Led2, HIGH); 
  delay (500);
  digitalWrite(Pin_Led2, LOW);
  digitalWrite(Pin_Led3, HIGH);
  delay (500);
  digitalWrite(Pin_Led3, LOW); 
  digitalWrite(Pin_Led4, HIGH);
  delay (500);
  digitalWrite(Pin_Led4, LOW);

  Serial.println();


  // Setup wifi connection 

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_ssid, WIFI_password);
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    digitalWrite(Pin_Led1, HIGH);
    delay(500);
    digitalWrite(Pin_Led1, LOW);
    delay(500);
  }
  Serial.println(" WiFi connected!");
  digitalWrite(Pin_Led1, HIGH);

  IP.fromString(BSH_ip);

  // Receive time 

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println(" time received!");
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  digitalWrite(Pin_Led2, HIGH);
  
  // Setup keys and certificates

  X509List servercert(BSH_root_CA);
  X509List clientcert(client_cert);
  PrivateKey clientkey(client_key);

  client->setTrustAnchors(&servercert);
  client->setClientRSACert(&clientcert, &clientkey);

  // Register client

  if (register_client) {
    registerclient();
  }

  // Connect to server at port 8444. This only works after successful client registration
  
  Serial.println("Connecting to server " + BSH_ip + " ...");

  if (!client->connect(IP, 8444)) {
    Serial.println("Connection failed! Program is stopped");
    while(true) {
       delay(1000);
    }
  }
  else {
    Serial.println("Connected to server!");
    digitalWrite(Pin_Led3, HIGH);
  }

  delay(2000);
  digitalWrite(Pin_Led1, LOW);
  digitalWrite(Pin_Led2, LOW);
  digitalWrite(Pin_Led3, LOW);

  prev_time = millis();

}


// Main loop -------------------------------------------------------------------------------------------

void loop() {

  // Register long-polling for unserdefinedstates and start poll when needed

  if (subscribe_poll) {
    polling_id = subscribePolling();
    subscribe_poll  = false;
  }

  if (poll) {
    if (polling_id != "no result") {
      startPoll(polling_id);
      poll  = false;
    }
  }
  
  // Reading available data non-blocking

  while (client->available()) {
    c = client->read();
    //Serial.print(c);
    json += c;
  }
   
  // Let LED flash, where flashing is enabled and state is on

  current_time = millis();

  if (((current_time - prev_time) > 500) && ((current_time - prev_time) < 699)) {

    for (int i = 0; i < number_entries; i++) {
      if ((parameter[i].blinking) && (state_array[i])) {
        analogWrite(parameter[i].pin, parameter[i].brightness);
      }
    }
  }

  if ((current_time - prev_time) > 1000) {

    for (int i = 0; i < number_entries; i++) {
      if ((parameter[i].blinking) && (state_array[i])) {
        digitalWrite(parameter[i].pin, false);
      }
    }
    prev_time = current_time;
  }

  // If we received something > 580 chars long (header + body), start analyze the data

  if (json.length() > 580) {

    // Find the json

    firstindex = json.indexOf("[");
    lastindex = json.lastIndexOf("]");
    json = json.substring(firstindex, lastindex+1);

    //Serial.println("Received body message: " + json);
  
    // Deserialize the json

    DeserializationError error = deserializeJson(doc, json);
    json = "";

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    }

    array = doc[0]["result"].as<JsonArray>();
    Serial.println("Number of messages received: " + String(array.size()));
    error_code = doc[0]["error"]["code"];           // error code
    error_message = doc[0]["error"]["message"];     // error message

    for (int i = 0; i < array.size(); i++) {        

      name = doc[0]["result"][i]["name"];          // name of the userdefinedstate
      state = doc[0]["result"][i]["state"];        // state of the userdefinedstate: false or true

      // If "name" is not empty, whe have received a message with a new status

      if (String(name) != "") {
        Serial.println("New status: " + String(name) + " = " + String(state));

        // Check, which GPIO pin needs to bet set

        for (int i = 0; i < number_entries; i++ ) {
          if (parameter[i].name == String(name)) {
            if (state) {
              analogWrite(parameter[i].pin, parameter[i].brightness);
              state_array[i] = true;
            }
            else {
              digitalWrite(parameter[i].pin, false);
              state_array[i] = false;
            }
          }
        }
      }
    }
  
    // If we receive an error, we re-subscribe the polling

    if (error_code < 0) {
      subscribe_poll = true;
      Serial.println("Error code received: " + String(error_code) +  ", error message : " + String(error_message));
      //appendFile("/log_file.txt", String(asctime(&timeinfo)) + "Error code received: " + String(error_code)  + "\n");
    }

    poll = true;

  }
 
  //check, if client is still available. If we lost connection, restart the ESP

  if (!client) {
    ESP.restart();
  }

  delay (100);
}


// Functions -------------------------------------------------------------------------------------------


String subscribePolling (void) {

  // Register long-polling for unserdefinedstates

  output = "[{\"jsonrpc\":\"2.0\",\"method\":\"RE/subscribe\",\"params\": [\"com/bosch/sh/remote/userdefinedstates\", null]}]";

  client->print(String("POST ") + "/remote/json-rpc" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_ip + ":8444\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Content-Length: " + output.length() + "\r\n" +
                 "\r\n" +
                 output + "\n");


  while (client->connected()) {
    line = client->readStringUntil('\n');
    //Serial.println(line);
    if (line == "\r") {
      //Serial.println("headers received");
      break;
    }
  }

  while (client->available()) {
    c = client->read();
    json += c;
  }

  // Find the json

  firstindex = json.indexOf("[");
  lastindex = json.lastIndexOf("]");
  json = json.substring(firstindex, lastindex+1);

  //Serial.println("Received body message: " + json);

  // Deserialize the JSON

  DeserializationError error = deserializeJson(doc, json);
  json = "";

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return "no result";
  }

  // If result is empty, we didn't got our polling_id

  if (String(doc[0]["result"]) == "") {
    Serial.println("Polling couldn't subscribed!");
    return "no result";
  }

  // Returns the polling_id, it's needed for startPoll

  Serial.println("Polling subscribed. Polling_Id: " + String(doc[0]["result"]));
  return String(doc[0]["result"]);
}



void startPoll (String _polling_id) {

  output = "[{\"jsonrpc\":\"2.0\",\"method\":\"RE/longPoll\",\"params\": [\"" + _polling_id + "\",8000]}]";   // Poll is valid for 8000s, will be renewed automatically

  client->print(String("POST ") + "/remote/json-rpc" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_ip + ":8444\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Connection: keep-alive\r\n" +
                 "Content-Length: " + output.length() + "\r\n" +
                 "\r\n" +
                 output + "\n");
  Serial.println("Poll sent... wait for answer");
}



void registerclient () {

  String cert;

  Serial.println("Connecting to server " + BSH_ip + " ...");

  if (!client->connect(IP, 8443)) {
    Serial.println("Connection to server at port 8443 failed! Did you press the button on the smart home controller? Program is stopped");
    while(true) {
      delay(1000);
    }
  }
  else {
    Serial.println("Connected to server at port 8443!");
    digitalWrite(Pin_Led3, HIGH);
    delay(500);
  }

  //formate the client_cert for being sent as JSON to the BSH controller

  cert = String(client_cert);
  cert.replace("\n","");
  cert.replace("-----BEGIN CERTIFICATE-----","-----BEGIN CERTIFICATE-----\r");
  cert.replace("-----END CERTIFICATE-----","\r-----END CERTIFICATE-----");
  
  //setup the JSON

  doc["@type"] = "client";
  doc["id"] = "oss_esp_display";
  doc["name"] = "OSS ESP Display";
  doc["primaryRole"] = "ROLE_RESTRICTED_CLIENT";
  doc["certificate"] = cert;

  serializeJson(doc, json);

  //send header and JSON

  client->print(String("POST ") + "/smarthome/clients" + " HTTP/1.1\r\n" +
                 "Host: " + BSH_ip + ":8443\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Systempassword: " + BSH_password + "\r\n" + 
                 "Connection: keep-alive\r\n" +
                 "Content-Length: " + json.length() + "\r\n" +
                 "\r\n" +
                 json + "\n");
                 
  Serial.println("Client registration posted... wait for answer");

  while (client->connected()) {
    line = client->readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  while (client->available()) {
    c = client->read();
    Serial.print(c);
  }

  Serial.println("\nCheck in BSH app if the OSS ESP Display occurs at mobil devices. If not, check BSH_password. If yes, set register_client = false and re-upload. Program is stopped.");
  while(true) {
    delay(1000);
  }

}