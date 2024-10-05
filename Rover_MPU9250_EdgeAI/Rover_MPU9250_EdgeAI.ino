/* author: koutsourelakis */

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "mpu9250.h"
#include "rover.h" 

/*
 *  ---------------------------------------------------------------------------------------------
 * | Pin Number |         Pin Name          |                      Function                      |
 * |------------|---------------------------|----------------------------------------------------|
 * |    18      | `INTERRUPT_PIN1`          | Interrupt pin connected to the optical sensor 1    |
 * |    19      | `INTERRUPT_PIN2`          | Interrupt pin connected to the optical sensor 2    |
 * |    27      | `rightWheelForward_OUT27` | Controls forward movement of the right wheel motor |
 * |    33      | `leftWheelForward_OUT33`  | Controls forward movement of the left wheel motor  |
 * |    26      | `rightWheelReverse_OUT26` | Controls reverse movement of the right wheel motor |
 * |    32      | `leftWheelReverse_OUT32`  | Controls reverse movement of the left wheel motor  |
 *  ---------------------------------------------------------------------------------------------
*/

/* BLE is inside an #ifdef. Uncomment the next line to activate it.
 * Note: BLE is not implemented as of now
 */
/*  #define ENABLE_BLE */

/* Mpu9250 object */
bfs::Mpu9250 imu;
/* Decision Tree model */
Eloquent::ML::Port::DecisionTree model;
String direction;


#define INTERRUPT_PIN1 18  /* Pin connected to the OUT of the optical sensor */
#define INTERRUPT_PIN2 19  /* Pin connected to the OUT of the optical sensor */

/* DHT_influx */
const char* mqtt_server = "192.168.160.22"; 
int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

/* ==== Uncomment if .json file containing SSID/pwd credentionals is not parsed via SPIFFFS === */
//const char* ssid = "harrkout";
//const char* password = "123456787";

AsyncWebServer server(80);

String header;

/* Set static IP configuration */
IPAddress staticIP(192, 168, 1, 4);  /*  static IP address, not working most times */
IPAddress gateway(192, 168, 1, 1);   /* gateway/router IP address */

String rightWheelReverse_PIO26 = "off";
String rightWheelForward_PIO27 = "off";
String leftWheelReverse_PIO32 = "off";
String leftWheelForward_PIO33 = "off";

const int rightWheelForward_OUT27 = 27;
const int leftWheelForward_OUT33 = 33;
const int rightWheelReverse_OUT26 = 26;
const int leftWheelReverse_OUT32 = 32;

/* Optical Encoder */
unsigned long lastTime = 0;
int tick = 0;

float wheelDiameter = 0.1;                                        /* Diameter in meters */
float ticksPerRevolution = 2;                                     /* One tick is one revolution of a wheel */
float distancePerTick = PI * wheelDiameter / ticksPerRevolution;  /* Distance travelled in meters */

void speedInterrupt() {
  tick++;
}

void forward() {
  Serial.println("Moving forward");
  digitalWrite(rightWheelForward_OUT27, HIGH);
  digitalWrite(leftWheelForward_OUT33, HIGH);
}

void backward() {
  Serial.println("Moving backward");
  digitalWrite(rightWheelReverse_OUT26, HIGH);
  digitalWrite(leftWheelReverse_OUT32, HIGH);
}

void left() {
  Serial.println("Turning left");
  digitalWrite(rightWheelForward_OUT27, HIGH);
  digitalWrite(leftWheelForward_OUT33, LOW);
}

void right() {
  Serial.println("Turning right");
  digitalWrite(rightWheelForward_OUT27, LOW);
  digitalWrite(leftWheelForward_OUT33, HIGH);
}

void stop() {
  Serial.println("Stopping");
  digitalWrite(rightWheelForward_OUT27, LOW);
  digitalWrite(leftWheelForward_OUT33, LOW);
  digitalWrite(rightWheelReverse_OUT26, LOW);
  digitalWrite(leftWheelReverse_OUT32, LOW);
}

/* === BLE not implemented or used === */

#ifdef ENABLE_BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define bleServerName "DHT_ESP32"
int state = 0;
bool deviceConnected = false;

/* See the following for generating UUIDs:
 * https://www.uuidgenerator.net/
 */
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

class MyCallbacks : public BLECharacteristicCallbacks {
   void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        Serial.println();
        Serial.println("*********");
        if (rxValue[0] != '0') state = 1;
        else state = 0;
      }

      /* Command handling */
      if (rxValue == "BWR") {
        digitalWrite(rightWheelForward_OUT27, HIGH);  /* Activate the pin */
      } else if (rxValue == "SWR") {
        digitalWrite(rightWheelForward_OUT27, LOW);   /* Deactivate the pin */
      } else if (rxValue == "BWL") {
        digitalWrite(leftWheelForward_OUT33, HIGH);   /* Activate the pin */
      } else if (rxValue == "SWL") {
        digitalWrite(leftWheelForward_OUT33, LOW);    /* Deactivate the pin */
      }
   }
};

/* Temperature Characteristic and Descriptor */
BLECharacteristic bmeTemperatureCelsiusCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeTemperatureCelsiusDescriptor(BLEUUID((uint16_t)0x2902));

/* Humidity Characteristic and Descriptor */
BLECharacteristic bmeHumidityCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeHumidityDescriptor(BLEUUID((uint16_t)0x2903));

/* Setup callbacks onConnect and onDisconnect */
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

#endif  /* ENABLE_BLE */

/* ================================================== */

void setup() {
  Serial.begin(115200);
  Serial.println();

  /* BLE Setup (if enabled) */
  #ifdef ENABLE_BLE
    /* Create the BLE Device */
    BLEDevice::init(bleServerName);

    /* Create the BLE Server */
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    /* Create the BLE Service */
    BLEService *bmeService = pServer->createService(SERVICE_UUID);

    /* Create the BLE Characteristic for RX */
    BLECharacteristic * pRxCharacteristic = bmeService->createCharacteristic(
                         CHARACTERISTIC_UUID_RX,
                         BLECharacteristic::PROPERTY_WRITE
                       );
    pRxCharacteristic->setCallbacks(new MyCallbacks());

    /* Start the service */
    bmeService->start();

    /* Start advertising */
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pServer->getAdvertising()->start();
    Serial.println("Waiting for a BLE client connection to notify...");
  #endif

  /* ============ IMU ============ */

  while(!Serial) {}
  /* Start the I2C bus */
  Wire.begin();
  Wire.setClock(400000);
  /* I2C bus,  0x68 address */
  imu.Config(&Wire, bfs::Mpu9250::I2C_ADDR_PRIM);
  /* Initialize and configure IMU */
  if (!imu.Begin()) {
    Serial.println("Error initializing communication with IMU");
    while(1) {}
  }
  /* Set the sample rate divider */ 
  if (!imu.ConfigSrd(19)) {
    Serial.println("Error configured SRD");
    while(1) {}
  }

  /*======================================== */

  /* Mount SPIFFS file system */
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // SPIFFS.format();

  /* Get flash chip size */
  uint32_t flashSize = ESP.getFlashChipSize();
  Serial.print("Flash Chip Size: ");
  Serial.print(flashSize);
  Serial.println(" bytes");
 
  /* Calculate SPIFFS flash offset */
  uint32_t spiffsSize = SPIFFS.totalBytes();
  uint32_t spiffsOffset = flashSize - spiffsSize;
  Serial.print("SPIFFS Flash Offset: 0x");
  Serial.println(spiffsOffset, HEX);

  /* List files in SPIFFS and print their contents */
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  /* Check if the file "/commands.txt" exists */
  if (SPIFFS.exists("/commands.txt")) {
    Serial.println("File '/commands.txt' exists in SPIFFS.");
  } else {
    Serial.println("File '/commands.txt' does not exist in SPIFFS.");
  }

  while (file) {
    Serial.print("File: ");
    Serial.println(file.name());
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
    file = root.openNextFile();
  }

  /* Open config.json file */
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return;
  }

  /* Parse JSON object */
  StaticJsonDocument<256> jsonConfig;
  DeserializationError error = deserializeJson(jsonConfig, configFile);
  if (error) {
    Serial.println("Failed to parse config file");
    return;
  }

  /* Retrieve SSID and password from JSON */
const char*  ssid = jsonConfig["ssid"];
const char*  password = jsonConfig["password"];

  /* ========================================= */

  /* optical */
  pinMode(INTERRUPT_PIN1, INPUT);                                                   /* Set the interrupt pin as input */
  pinMode(INTERRUPT_PIN2, INPUT);                                                   /* Set the interrupt pin as input */
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN1), speedInterrupt, FALLING);  /* Attach the interrupt */
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN2), speedInterrupt, FALLING);  /* Attach the interrupt */

  pinMode(rightWheelForward_OUT27, OUTPUT);
  pinMode(leftWheelForward_OUT33, OUTPUT);
  pinMode(rightWheelReverse_OUT26, OUTPUT);
  pinMode(leftWheelReverse_OUT32, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.softAPConfig(staticIP, staticIP, gateway);

  if (!WiFi.softAP(ssid, password)) {
    Serial.println("Soft AP creation failed.");
    while (1)
      ;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /* === Server === */

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");

    AsyncElegantOTA.begin(&server);

    response->print("<!DOCTYPE html><html>");
    response->print("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    response->print("<link rel=\"icon\" href=\"data:,\">");
    response->print("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    response->print(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
    response->print("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
    response->print(".button:active { background-color: #7F7F7F; }");
    response->print(".slider { width: 300px; }</style>");
    response->print("<script>");
    response->print("document.addEventListener('keydown', function(event) {");
    response->print("  switch(event.key) {");
    response->print("    case 'ArrowUp':");
    response->print("      sendRequest('/forward');");
    response->print("      break;");
    response->print("    case 'ArrowDown':");
    response->print("      sendRequest('/backward');");
    response->print("      break;");
    response->print("    case 'ArrowLeft':");
    response->print("      sendRequest('/left');");
    response->print("      break;");
    response->print("    case 'ArrowRight':");
    response->print("      sendRequest('/right');");
    response->print("      break;");
    response->print("  }");
    response->print("});");
    response->print("document.addEventListener('keyup', function(event) {");
    response->print("  sendRequest('/stop');");
    response->print("});");
    response->print("function sendRequest(url) {");
    response->print("  var xhttp = new XMLHttpRequest();");
    response->print("  xhttp.open('GET', url, true);");
    response->print("  xhttp.send();");
    response->print("}");
    response->print("</script>");

    response->print("<body><h1>Rover Pad v1.1</h1>");
    response->print("<div style=\"display: flex; flex-direction: column; align-items: center;\">");
    response->print("<div style=\"display: flex;\">");
    response->print("<p></p>");
    response->print("<p><button class=\"button\" onmousedown=\"sendRequest('/forward')\" onmouseup=\"sendRequest('/stop')\" ontouchstart=\"sendRequest('/forward')\" ontouchend=\"sendRequest('/stop')\">&#8593;</button></p>");
    response->print("<p></p>");
    response->print("</div>");
    response->print("<div style=\"display: flex;\">");
    response->print("<p><button class=\"button\" onmousedown=\"sendRequest('/left')\" onmouseup=\"sendRequest('/stop')\" ontouchstart=\"sendRequest('/left')\" ontouchend=\"sendRequest('/stop')\">&#8592;</button></p>");
    response->print("<p></p>");
    response->print("<p><button class=\"button\" onmousedown=\"sendRequest('/right')\" onmouseup=\"sendRequest('/stop')\" ontouchstart=\"sendRequest('/right')\" ontouchend=\"sendRequest('/stop')\">&#8594;</button></p>");
    response->print("</div>");
    response->print("<div style=\"display: flex;\">");
    response->print("<p></p>");
    response->print("<p><button class=\"button\" onmousedown=\"sendRequest('/backward')\" onmouseup=\"sendRequest('/stop')\" ontouchstart=\"sendRequest('/backward')\" ontouchend=\"sendRequest('/stop')\">&#8595;</button></p>");
    response->print("<p></p>");
    response->print("</div>");

    /**"speed" slider */
    response->print("<p>Speed: <span id=\"speedValue\">0</span></p>");
    response->print("<input type=\"range\" min=\"0\" max=\"255\" value=\"0\" class=\"slider\" id=\"speedSlider\">");

    /**Js for updating speed value */
    response->print("<script>");
    response->print("var slider = document.getElementById('speedSlider');");
    response->print("var output = document.getElementById('speedValue');");
    response->print("output.innerHTML = slider.value;");
    response->print("slider.oninput = function() {");
    response->print("  output.innerHTML = this.value;");
    response->print("  sendRequest('/speed/' + this.value);");
    response->print("}");
    response->print("</script>");

    /* Ticks per second */
    response->print("<p id=\"ticksPerSecond\"></p>");
    response->print("<script>");
    response->print("function updateTicksPerSecond() {");
    response->print("  var xhttp = new XMLHttpRequest();");
    response->print("  xhttp.onreadystatechange = function() {");
    response->print("    if (this.readyState == 4 && this.status == 200) {");
    response->print("      document.getElementById('ticksPerSecond').innerText = this.responseText;");
    response->print("      setTimeout(updateTicksPerSecond, 1000);");  /* Update every 1 second */
    response->print("    }");
    response->print("  };");
    response->print("  xhttp.open('GET', '/tickpersecond', true);");
    response->print("  xhttp.send();");
    response->print("}");

    /* Call the function initially */
    response->print("updateTicksPerSecond();");
    response->print("</script>");

    /* Speedometer */
    response->print("<p id=\"speedometer\"></p>");
    response->print("<script>");
    response->print("function updateSpeedometer() {");
    response->print("  var xhttp = new XMLHttpRequest();");
    response->print("  xhttp.onreadystatechange = function() {");
    response->print("    if (this.readyState == 4 && this.status == 200) {");
    response->print("      var ticksPerSecond = parseFloat(this.responseText);");
    response->print("      var speed = ticksPerSecond * " + String(distancePerTick * 3600 / 1000) + ";");  /* Convert ticks per second to km/h */
    response->print("      document.getElementById('speedometer').innerText = 'Speed: ' + speed.toFixed(2) + ' km/h';");
    response->print("      setTimeout(updateSpeedometer, 1000);");  /* Update every 1 second */
    response->print("    }");
    response->print("  };");
    response->print("  xhttp.open('GET', '/tickpersecond', true);");
    response->print("  xhttp.send();");
    response->print("}");

    /* Call the function initially */
    response->print("updateSpeedometer();");
    response->print("</script>");

    /* button for updating IP */
    response->print("<button class=\"button\" onclick=\"window.location.href='http://");
    response->print(WiFi.localIP().toString());
    response->print("/update';\">Update IP</button>");

    /* Direction */
    response->print("<p id=\"predictedDirection\">Direction: </p>");
    response->print("<script>");
    response->print("function updateDirection() {");
    response->print("  var xhttp = new XMLHttpRequest();");
    response->print("  xhttp.onreadystatechange = function() {");
    response->print("    if (this.readyState == 4 && this.status == 200) {");
    response->print("      document.getElementById('predictedDirection').innerText = 'Direction: ' + this.responseText;");
    response->print("    }");
    response->print("  };");
    response->print("  xhttp.open('GET', '/direction', true);");
    response->print("  xhttp.send();");
    response->print("}");
    response->print("setInterval(updateDirection, 1000);"); /* Update every second */
    response->print("</script>");
    
    response->print("</body></html>");
    request->send(response);
  });

  server.on("/direction", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", direction); /* Use the 'direction' variable defined in your loop */
  });

  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request) {
    forward();
    request->send(200, "text/plain", "Moving forward");
    client.publish("esp32/dht11", "forward");
  });

  server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request) {
    backward();
    request->send(200, "text/plain", "Moving backward");
    client.publish("esp32/dht11", "backward");
  });

  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request) {
    left();
    request->send(200, "text/plain", "Turning left");
    client.publish("esp32/dht11", "left");
  });

  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request) {
    right();
    request->send(200, "text/plain", "Turning right");
    client.publish("esp32/dht11", "right");
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
    stop();
    request->send(200, "text/plain", "Stopping");
    client.publish("esp32/dht11", "stop");
  });

  server.on("/speed", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value")) {
      String value = request->getParam("value")->value();
      int newSpeed = value.toInt();
      
      /* Ensure the speed value is within the valid range (0 - 255) */
      newSpeed = constrain(newSpeed, 0, 255);
      
      /* Update the speed of both motors */
      analogWrite(rightWheelForward_OUT27, newSpeed);
      analogWrite(leftWheelForward_OUT33, newSpeed);
          
      request->send(200, "text/plain", "Speed updated to: " + String(newSpeed));
    } else {
      request->send(400, "text/plain", "Missing speed parameter");
    }
  });

  server.on("/tickpersecond", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(tick));
  });

  server.begin();

  Serial.println("HTTP server started");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);

}

/* MQTT connection status */
void reconnect(){
  while (!client.connected()){
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32 client")){
      Serial.println("Connected");
    }
    else{
      Serial.print(client.state());
      Serial.println("Failed - Try again in 5 seconds");
      delay(5000);
    }
  }
}

/* Main */
void loop() {
  
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  /*============ IMU ================
  /* Check if new IMU data is available */
  if (imu.Read() && imu.new_imu_data()) {
      /* Read gyroscope data */
      float gyroX = imu.gyro_x_radps();
      float gyroY = imu.gyro_y_radps();
      float gyroZ = imu.gyro_z_radps();

      /* Prepare input for the decision tree (only gyro data) */
      float input[3] = { gyroX, gyroY, gyroZ };

      /* Predict direction using the decision tree model */
      int prediction = model.predict(input);

      /* Print gyroscope data */
      Serial.print("Gyro Data: ");
      Serial.print("GyroX: "); Serial.print(gyroX);
      Serial.print("\tGyroY: "); Serial.print(gyroY);
      Serial.print("\tGyroZ: "); Serial.print(gyroZ);

      /* Convert prediction to string */
      switch (prediction) {
          case 0: direction = "Wheelie"; break;
          case 1: direction = "Forward"; break;
          case 2: direction = "Idle"; break;
          case 3: direction = "Turning Left"; break;
          case 4: direction = "Turning Right"; break;
          case 5: direction = "Reverse"; break;
          default: direction = "Unknown"; break;
      }
      
      Serial.print("\tPredicted Direction: "); Serial.println(direction);
      delay(1000);
    
  /* ======================================= */
  
  /* Display the speed on the Serial Monitor every second */
  if (millis() - lastTime >= 1000) {
    float distanceTraveled = tick * distancePerTick;         /* Total distance traveled in one second */
    float speed_kmh = (distanceTraveled / 1000.0) * 3600.0;  /* Convert meters per second to kilometers per hour */
    Serial.print("Speed (km/h): ");
    Serial.println(speed_kmh);
    lastTime = millis();

    tick = 0;

    if (isnan(speed_kmh)){
      Serial.println("Failed to read from speed_kmh sensor!");
      return;
    }

    char payload[50];
    char payload2[50];
    
    snprintf(payload, sizeof(payload), "{\"speed\": %.2f}", speed_kmh);
    snprintf(payload2, sizeof(payload2), "{\"direction\": %s\}", direction.c_str());

    client.publish("esp32/dht11", payload);
    client.publish("esp32/direction", payload2); 
  } 
 }
}
