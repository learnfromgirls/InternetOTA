/*

  This example downloads sketch update over WiFi network.
  You can choose between HTTP and HTTPS connection.
  In HTTPS case, remember to flash the server root CA certificate
  using WiFi101/WiFiNINA updater tool.
  It doesn't start the OTA upload sever of the ArduinoOTA library,
  it only uses the InternalStorage object of the library
  to store and apply the downloaded binary file.

  To create the bin file for update of a SAMD board (except of M0),
  use in Arduino IDE command "Export compiled binary".
  To create a bin file for AVR boards see the instructions in README.MD.
  To try this example, you should have a web server where you put
  the binary update.
  Modify the constants below to match your configuration.

  Created for ArduinoOTA library in December 2020
  by Nicola Elia
  based on Juraj Andrassy sample sketch 'OTASketchDownload'
*/

#include <ArduinoOTA.h> // only for InternalStorage
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include "Stopwatch.h"


// Please enter your WiFi sensitive data in the arduino_secrets.h file
#include "arduino_secrets.h"

const short VERSION = 0;

const char MY_SSID[] = SECRET_SSID; // Loaded from arduino_secrets.h
const char MY_PASS[] = SECRET_PASS; // Loaded from arduino_secrets.h

//WiFiClient    wifiClient;  // HTTP
WiFiSSLClient wifiClientSSL;  // HTTPS
int status = WL_IDLE_STATUS;

// https://raw.githubusercontent.com/learnfromgirls/InternetOTA/main/update-v1.bin



void checkTimeAndMaybeDownload() {
  // Time interval check
  const unsigned long CHECK_INTERVAL = 3600000;  // Time interval between update checks (ms) 1 hour

  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < CHECK_INTERVAL)
    return;
  previousMillis = currentMillis;
  Serial.print("current Sketch version ");
  Serial.println(VERSION);
  handleSketchDownload();
}


void handleSketchDownload() {
  const char* SERVER = "raw.githubusercontent.com";  // Set your correct hostname
  const unsigned short SERVER_PORT = 443;     // Commonly 80 (HTTP) | 443 (HTTPS)
  const char* PATH = "/learnfromgirls/InternetOTA/main/update-v%d.bin";       // Set the URI to the .bin firmware


  Serial.print("making client for server=");
  Serial.println(SERVER);
  Serial.println("Initialize WiFi");
  // attempt to connect to Wifi network:

  status = WiFi.status();
  Stopwatch wifiConnectTime;
  wifiConnectTime.markStart();
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(MY_SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:

    status = WiFi.begin(MY_SSID, MY_PASS);
    unsigned long lastseconds = 0;
    do {
      yield();
      delay(1000);
      status = WiFi.status();
      wifiConnectTime.markEnd();


      if (status != WL_CONNECTED ) {
        if ( wifiConnectTime.getDurSeconds() - lastseconds > 1UL) {
          lastseconds =  wifiConnectTime.getDurSeconds();

          //Log.error ("Waiting for access point on %s waited=%l seconds\n\r", ssidPassConfig.ssid, wifiConnectTime.getDurSeconds());
        }
      }
    } while (status != WL_CONNECTED && wifiConnectTime.getDur() < 10000UL);
    if ( wifiConnectTime.getDur() > 10000UL) {
      return;
    }
  }
  WiFi.lowPowerMode();
  Serial.println("WiFi connected");

  //HttpClient client(wifiClient, SERVER, SERVER_PORT);  // HTTP
  HttpClient client(wifiClientSSL, SERVER, SERVER_PORT);  // HTTPS

  char buff[128];
  snprintf(buff, sizeof(buff), PATH, VERSION + 1);

  Serial.print("Check for update file ");
  Serial.println(buff);

  // Make the GET request
  client.get(buff);

  int statusCode = client.responseStatusCode();
  Serial.print("Update status code: ");
  Serial.println(statusCode);
  if (statusCode != 200) {
    client.stop();
    return;
  }

  long length = client.contentLength();
  if (length == HttpClient::kNoContentLengthHeader) {
    client.stop();
    Serial.println("Server didn't provide Content-length header. Can't continue with update.");
    return;
  }
  Serial.print("Server returned update file of size ");
  Serial.print(length);
  Serial.println(" bytes");

  if (!InternalStorage.open(length)) {
    client.stop();
    Serial.println("There is not enough space to store the update. Can't continue with update.");
    return;
  }
  int timeoutretries = 100; //allow up to 100 seconds of timeout retries.
  byte b;
  while (length > 0) {
    if (!client.readBytes(&b, 1)) { // reading a byte with timeout
      if (timeoutretries-- < 0) {
        break;
      } else {
        delay(1000);
      }
    } else {
      InternalStorage.write(b);
      length--;
    }
  }
  InternalStorage.close();
  client.stop();
  if (length > 0) {
    Serial.print("Timeout downloading update file at ");
    Serial.print(length);
    Serial.println(" bytes. Can't continue with update.");
    return;
  }

  Serial.println("Sketch update apply and reset.");
  Serial.flush();
  InternalStorage.apply(); // this doesn't return
}

void setup() {

  Serial.begin(115200);
  delay(1000); //no need to hang on serial

  Serial.print("Sketch version ");
  Serial.println(VERSION);


}

void loop() {
  // check for updates
  checkTimeAndMaybeDownload();

  // add your normal loop code below ...
}
