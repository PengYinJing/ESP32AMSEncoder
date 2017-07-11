/*
 * BluemixClient.ino
 * 
 * Interface for IBM Waston IoT 
 * 
 * Created by: 
 *       K.C.Y
 * Date:
 *       2017/06/30
 */

#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "Global.h"
#include "PubSubClient.h"

const char* root_ca_cert = 
"-----BEGIN CERTIFICATE-----\n"
"MIIElDCCA3ygAwIBAgIQAf2j627KdciIQ4tyS8+8kTANBgkqhkiG9w0BAQsFADBh\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
"QTAeFw0xMzAzMDgxMjAwMDBaFw0yMzAzMDgxMjAwMDBaME0xCzAJBgNVBAYTAlVT\n"
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxJzAlBgNVBAMTHkRpZ2lDZXJ0IFNIQTIg\n"
"U2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n"
"ANyuWJBNwcQwFZA1W248ghX1LFy949v/cUP6ZCWA1O4Yok3wZtAKc24RmDYXZK83\n"
"nf36QYSvx6+M/hpzTc8zl5CilodTgyu5pnVILR1WN3vaMTIa16yrBvSqXUu3R0bd\n"
"KpPDkC55gIDvEwRqFDu1m5K+wgdlTvza/P96rtxcflUxDOg5B6TXvi/TC2rSsd9f\n"
"/ld0Uzs1gN2ujkSYs58O09rg1/RrKatEp0tYhG2SS4HD2nOLEpdIkARFdRrdNzGX\n"
"kujNVA075ME/OV4uuPNcfhCOhkEAjUVmR7ChZc6gqikJTvOX6+guqw9ypzAO+sf0\n"
"/RR3w6RbKFfCs/mC/bdFWJsCAwEAAaOCAVowggFWMBIGA1UdEwEB/wQIMAYBAf8C\n"
"AQAwDgYDVR0PAQH/BAQDAgGGMDQGCCsGAQUFBwEBBCgwJjAkBggrBgEFBQcwAYYY\n"
"aHR0cDovL29jc3AuZGlnaWNlcnQuY29tMHsGA1UdHwR0MHIwN6A1oDOGMWh0dHA6\n"
"Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RDQS5jcmwwN6A1\n"
"oDOGMWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RD\n"
"QS5jcmwwPQYDVR0gBDYwNDAyBgRVHSAAMCowKAYIKwYBBQUHAgEWHGh0dHBzOi8v\n"
"d3d3LmRpZ2ljZXJ0LmNvbS9DUFMwHQYDVR0OBBYEFA+AYRyCMWHVLyjnjUY4tCzh\n"
"xtniMB8GA1UdIwQYMBaAFAPeUDVW0Uy7ZvCj4hsbw5eyPdFVMA0GCSqGSIb3DQEB\n"
"CwUAA4IBAQAjPt9L0jFCpbZ+QlwaRMxp0Wi0XUvgBCFsS+JtzLHgl4+mUwnNqipl\n"
"5TlPHoOlblyYoiQm5vuh7ZPHLgLGTUq/sELfeNqzqPlt/yGFUzZgTHbO7Djc1lGA\n"
"8MXW5dRNJ2Srm8c+cftIl7gzbckTB+6WohsYFfZcTEDts8Ls/3HB40f/1LkAtDdC\n"
"2iDJ6m6K7hQGrn2iWZiIqBtvLfTyyRRfJs8sjX7tN8Cp1Tm5gr8ZDOo0rwAhaPit\n"
"c+LJMto4JQtV05od8GiG7S5BNO98pVAdvzr508EIDObtHopYJeS4d60tbvVS3bR0\n"
"j6tJLp07kzQoH3jOlOrHvdPJbRzeXDLz\n"
"-----END CERTIFICATE-----\n";

char *server = NULL;                                //IBM Watson server
char authMethod[] = "use-token-auth";               //Authentication method used to connect to IBM
char *token = NULL;                                 //Token used to connect to IBM
char *clientId = NULL;                              //ClientID used to connect to IBM

#if TEST_MQTT_MODE
WiFiClient s_WifiClient;
#else
WiFiClientSecure s_WifiClient;
#endif

Preferences s_Preferences;
PubSubClient * s_PubSubClient = NULL;               //Wrapper for wificlient allowing use of MQTT
WiFiServer s_WifiServer(80);                        //Spins up the server that wifimanager uses on first startup

#if 0
char s_JsonBuffer[512];          //Message buffer to send to IoT Watson
#endif

boolean bluemix_subscribe_check()
{
  if (!s_PubSubClient->loop())  // Ping IoTWatson
    return mqtt_connect();      // If unable to ping, check for valid connection
  return true;
}

boolean setup_wifi()
{
  if (WiFi.status() == WL_CONNECTED)
    return true;
  
  Serial.print(F("Connecting to Network: "));
  //Get the SSID and Password
  String ssid_verified;
  String password;
  s_Preferences.begin(PREF_SECTION_WIFI, false);
  ssid_verified = s_Preferences.getString(PREF_KEY_SSID);
  password = s_Preferences.getString(PREF_KEY_PASSWORD);
  s_Preferences.end();
  Serial.println(ssid_verified);
  WiFi.begin(ssid_verified.c_str(), password.c_str());

   //Attempt Wifi connection
   int timeout = 60;
   while (WiFi.status() != WL_CONNECTED && timeout > 0) {
      delay(500);
      Serial.print(F("."));
      timeout --;
   }
   if (timeout != 0) {
      Serial.print(F("WiFi connected, IP address: "));
      Serial.println(WiFi.localIP());
      return true;
   }
   else 
      return false;
}

boolean setup_bluemix()
{
  if (WiFi.status() != WL_CONNECTED)
    return false;
  if (s_PubSubClient && s_PubSubClient->connected())
    return true;
  /* Collect credentials from Preference */
  String org, device_type, device_id, tken;
#if 1
  Serial.println("Reading credenticals...");
  s_Preferences.begin(PREF_SECTION_WIFI, false);
  org = s_Preferences.getString(PREF_KEY_ORG, "");
  device_type = s_Preferences.getString(PREF_KEY_DEV_TYPE, "");
  device_id = s_Preferences.getString(PREF_KEY_DEV_ID, "");
  tken = s_Preferences.getString(PREF_KEY_TOKEN, "");
  s_Preferences.end();

  if (org.length() == 0 || device_type.length() == 0 || device_id.length() == 0 || tken.length() == 0)
    return false;
#else
  org = "hiok38";
  device_type = "ESP32";
  device_id = "ESP32AMSEncoder001";
  tken = "sD!SDgY3@piE-?v3Cx";
#endif
  /* Check & Free memory */
  if (server)
    free(server);
  if (token)
    free(token);
  if (clientId)
    free(clientId);
  if (s_PubSubClient)
    delete s_PubSubClient;

#if TEST_MQTT_MODE
  server = (char *)calloc(1, strlen(TEST_MQTT_SERVER) + 1);
  strcpy(server, TEST_MQTT_SERVER);
  clientId = (char*)calloc(1, strlen(TEST_MQTT_CLIENT_ID) + 1);
  strcpy(clientId, TEST_MQTT_CLIENT_ID);
  
  s_PubSubClient = new PubSubClient(server, 1883, mqtt_subscribe_handler, s_WifiClient);
#else
  /* Configure credentials for connection to Bluemix */
  server = (char *)calloc(1, strlen(org.c_str()) + strlen(".messaging.internetofthings.ibmcloud.com") + 1);
  strcpy(server, org.c_str());
  strcat(server, ".messaging.internetofthings.ibmcloud.com");
  token = (char *)calloc(1, strlen(tken.c_str()) + 1);
  strcpy(token, tken.c_str());
  clientId = (char *)calloc(1, strlen("d:::") + strlen(org.c_str()) + strlen(device_type.c_str()) + strlen(device_id.c_str()) + 1);
  strcpy(clientId, "d:");
  strcat(clientId, org.c_str());
  strcat(clientId, ":");
  strcat(clientId, device_type.c_str());
  strcat(clientId, ":");
  strcat(clientId, device_id.c_str());
  
  s_WifiClient.setCACert(root_ca_cert);
  s_PubSubClient = new PubSubClient(server, 8883, mqtt_subscribe_handler, s_WifiClient);
#endif /* TEST_MQTT_MODE */

  Serial.println("Connecting to Bluemix...");
  return mqtt_connect();
}

boolean mqtt_connect()
{
  int timeout = 10;
  if (!s_PubSubClient->connected())
  {  
    Serial.print("Reconnecting MQTT client to ");
    Serial.println(server);
    //Serial.println(authMethod);
    //Serial.println(token);
    while (!s_PubSubClient->connect(clientId, authMethod, token))
    {
       Serial.print(".");
       delay(1000);
       timeout--;
       if (timeout == 0)
        return false;
    }
  }
  Serial.println("Bluemix connection success!!!");
  return true;
}

void setup_wifi_server()
{
  WiFi.mode(WIFI_AP);

  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[6 - 2], HEX) + String(mac[6 - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "mike " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i = 0; i < AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);
      
  WiFi.softAP(AP_NameChar, "");

  Serial.print("AP IPv4: ");
  Serial.println(WiFi.softAPIP());

  s_WifiServer.begin();
  Serial.print(F("Awaiting Network Credentials"));
  
  /* Once the server is configured await the user to enter their connection credentials */
  while (!get_network_credenticals())
    delay(1000);
}

boolean get_network_credenticals()
{
  static int count = 0;
  count++;
  if (count == 20)
  {
    Serial.println();
    count = 0;
  }
    
  Serial.print(".");
  
  WiFiClient client = s_WifiServer.available();
  if (!client)
    return false;

  String req = client.readStringUntil('\r');
  Serial.println("/n" + req);
  client.flush();

  String s = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<!DOCTYPE HTML>\r\n<html>\r\n";
  
  /* Check to see if user submitted credentials, otherwise continue to serve up webpage */
  if (req.indexOf("msg?ssid") != -1)
  {
    s_Preferences.begin(PREF_SECTION_WIFI, false);
    
    Serial.println(F("Recieved network credentials. Waiting 10 seconds."));
    
    int p1 = req.indexOf("ssid=") + 5;
    int p2 = req.indexOf("&password");
    String str = req.substring(p1, p2);
    s_Preferences.putString(PREF_KEY_SSID, decode_string(str));
    
    p1 = p2 + 10;
    p2 = req.indexOf("&orgB=");
    str = req.substring(p1, p2);
    s_Preferences.putString(PREF_KEY_PASSWORD, decode_string(str));

    p1 = p2 + 6;
    p2 = req.indexOf("&devtypeB=");
    str = req.substring(p1, p2);
    s_Preferences.putString(PREF_KEY_ORG, decode_string(str));

    p1 = p2 + 10;
    p2 = req.indexOf("&devidB=");
    str = req.substring(p1, p2);
    s_Preferences.putString(PREF_KEY_DEV_TYPE, decode_string(str));

    p1 = p2 + 8;
    p2 = req.indexOf("&tokensB=");
    str = req.substring(p1, p2);
    s_Preferences.putString(PREF_KEY_DEV_ID, decode_string(str));

    p1 = p2 + 9;
    p2 = req.length() - String(" HTTP/1.1").length();
    str = req.substring(p1, p2);
    s_Preferences.putString(PREF_KEY_TOKEN, decode_string(str));

    s_Preferences.end();
    
    s += "<h1><center>Configuring ESP32</center></h1>";
    s += "</html>"; 
    client.print(s);
    
    return true;
  }
  else
  {
     s += 
      "<body>"    
      "<p>"
      "<center>"
      "<h1>ESP32 Configuration</h1>"
      "<div>"
      "</div>"
      "<form action='msg'><p>SSID:  <input type='text' name='ssid' size=50 autofocus></p>"
      "<p>Password: <input type='text' name='password' size=50 autofocus></p>"
      "<div><p>Organization: <input type='text' name='orgB' size=50 autofocus></p><p>Device Type: <input type='text' name='devtypeB' size=50 autofocus></p>"
      "<p>Device Id: <input type='text' name='devidB' size=50 autofocus></p><p>Token: <input type='text' name='tokensB' size=50 autofocus></p></div>"
      "<p><input type='submit' value='Submit'></p>"
      "</form>"
      "</center>";
      s += "</body></html>\n";
      // Send the response to the client
      client.print(s);
  }
  return false;
}

String decode_string(String str)
{
  str.replace("+", " ");
  str.replace("%21", "!");
  str.replace("%22", "");
  str.replace("%23", "#");
  str.replace("%24", "$");
  str.replace("%25", "%");
  str.replace("%26", "&");
  str.replace("%27", "'");
  str.replace("%28", "(");
  str.replace("%29", ")");
  str.replace("%2A", "*");
  str.replace("%2B", "+");
  str.replace("%2C", ",");
  str.replace("%2F", "/");
  str.replace("%3A", ":");
  str.replace("%3B", ";");
  str.replace("%3C", "<");
  str.replace("%3D", "=");
  str.replace("%3E", ">");
  str.replace("%3F", "?");
  str.replace("%40", "@");
  return str;
}

boolean bluemix_try_connect()
{
  if (!setup_wifi())
    setup_wifi_server();
  else if (!setup_bluemix())
    setup_wifi_server();
  else
  {
    //Serial.println("Connect success");
    return true;
  }
  return false;
}

#if 0
void bluemix_publish_data(uint64_t timestamp, char abi, short pcm)
{
  if (!s_PubSubClient)
    return;
  
  StaticJsonBuffer<JSON_OBJECT_SIZE(16)> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& d = root.createNestedObject("d");

  char str1[16], str2[16], str3[16];
  sprintf(str1, "%llu", timestamp);
  d["timestamp"] = str1;
  sprintf(str2, "%02x", abi);
  d["abi"] = str2;
  char* p = (char*)&pcm;
  sprintf(str3, "%02x%02x", p[1], p[0]);
  d["pcm"] = str3;

  root.printTo(s_JsonBuffer, sizeof(s_JsonBuffer));
  //Serial.printf("Publishing data : (%s)\n", s_JsonBuffer);
  
  int count = 0;
  while (!s_PubSubClient->publish("iot-2/evt/partial/fmt/json", (char*)s_JsonBuffer) && count < 3)
  {
      Serial.println("Publishing data FAILED");
      delay(2000);
      count ++;
      mqtt_connect();
  }
}
#else
void bluemix_publish_data(char* abi_buffer, char* pcm_buffer, int count)
{
  String payload = "{\"d\": {\"value\": [";
  char str[128];
  uint64_t* timestamp;
  char* abi;
  for (int i = 0; i < count; i++)
  {
    timestamp = (uint64_t*)abi_buffer;
    abi = abi_buffer + sizeof(uint64_t);
    sprintf(str, "{\"timestamp\": %llu, \"abi\": \"%02x\", \"pcm\": \"%02x%02x\"}", *timestamp, *abi, pcm_buffer[0], pcm_buffer[1]);
    payload += str;
    if (i < count - 1)
      payload += ",";
    abi_buffer += sizeof(uint64_t) + sizeof(char);
    pcm_buffer += sizeof(short);
  }
  payload += "]}}";

  //Serial.println(payload);
  //Serial.println(payload.length());
  
  count = 0;
  while (!s_PubSubClient->publish("iot-2/evt/partial/fmt/json", payload.c_str()) && count < 3)
  {
      Serial.println("Publishing data FAILED");
      delay(2000);
      count ++;
      mqtt_connect();
  }
}
#endif

void mqtt_subscribe_handler(char* topic, byte* payload, unsigned int payloadLength)
{
  
}

