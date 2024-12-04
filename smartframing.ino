#define BLYNK_TEMPLATE_ID "Your_Template_ID"
#define BLYNK_TEMPLATE_NAME "Smart Farming"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// Define DHT Sensor
#define DHTPIN 2       // Pin where the DHT sensor is connected
#define DHTTYPE DHT11  // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Define Pins for Soil Moisture Sensor and Relays
#define SOIL_MOISTURE_PIN A0  // Soil moisture sensor pin on ESP8266
#define RELAY1_PIN D6  // GPIO 12 for first pump (controlled by soil moisture sensor)
#define RELAY2_PIN D7  // GPIO 13 for second pump (controlled via Blynk)

// Define LED Pins for temperature and humidity
#define GREEN_LED_PIN D2   // GPIO 4 for normal conditions
#define RED_LED_PIN D3     // GPIO 0 for unfavorable conditions
#define WHITE_LED_PIN D1   // GPIO 5, controlled via Blynk

// Define Buzzer Pin for soil moisture alerts
#define BUZZER_PIN D5  // GPIO 14

// WiFi and Blynk credentials (replace placeholders with your own)
char auth[] = "Your_Blynk_Auth_Token";
char ssid[] = "Your_WiFi_SSID";
char pass[] = "Your_WiFi_Password";

// Thresholds for the sensors
int tempThresholdHigh = 50;     // High temperature threshold (Celsius)
int tempThresholdLow = 15;      // Low temperature threshold (Celsius)
int soilMoistureThreshold = 600; // Threshold indicating dry soil (higher value = dry)

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  
  // Initialize DHT sensor
  dht.begin();

  // Set LED, Buzzer, and Relay Pins as input/output
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  // Initial state of LEDs, Buzzer, and Relays
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(WHITE_LED_PIN, LOW);

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();  // Run Blynk
  
  // Reading the DHT11 sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Reading soil moisture sensor
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  
  // Display the readings in Serial Monitor
  Serial.print("Temperature : ");
  Serial.print(temperature);
  Serial.print(" *C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Soil Moisture: ");
  Serial.println(soilMoistureValue);

  // LED indication based on temperature and humidity
  if (temperature > tempThresholdHigh || temperature < tempThresholdLow) {
    // Unfavorable conditions for temp
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
    Blynk.virtualWrite(V5, 1);  // Send Red LED status to Blynk (ON)
    Blynk.virtualWrite(V6, 0);  // Send Green LED status to Blynk (OFF)
  } else {
    // Favorable conditions for temp
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    Blynk.virtualWrite(V5, 0);  // Send Red LED status to Blynk (OFF)
    Blynk.virtualWrite(V6, 1);  // Send Green LED status to Blynk (ON)
  }

  // Automatic control for the first pump based on soil moisture
  if (soilMoistureValue < soilMoistureThreshold) {  
    // Soil is dry (value is below the threshold), so turn ON the pump
    digitalWrite(RELAY1_PIN, HIGH);  // Activate first pump (dry soil)
    digitalWrite(BUZZER_PIN, HIGH); // Buzzer ON for dry soil
  } else {
    // Soil is wet (value is above the threshold), so turn OFF the pump
    digitalWrite(RELAY1_PIN, LOW);   // Deactivate first pump (wet soil)
    digitalWrite(BUZZER_PIN, LOW);  // Buzzer OFF
  }

  // Sending data to Blynk app
  Blynk.virtualWrite(V2, temperature); // Send temperature to Virtual Pin V2
  Blynk.virtualWrite(V3, humidity);    // Send humidity to Virtual Pin V3
  Blynk.virtualWrite(V4, soilMoistureValue); // Send soil moisture to Virtual Pin V4

  // Delay before next reading
  delay(2000); // Wait 2 seconds between readings
}

// This function handles the control of the second pump via Blynk button (Virtual Pin V5)
BLYNK_WRITE(V5) {
  int pumpControl = param.asInt(); // Get the button state from the Blynk app
  if (pumpControl == 1) {
    digitalWrite(RELAY2_PIN, LOW);  // Turn ON the second pump
    Blynk.virtualWrite(V5, 1);  // Send Pump 2 status to Blynk (ON)
  } else {
    digitalWrite(RELAY2_PIN, HIGH);  // Turn OFF the second pump
    Blynk.virtualWrite(V5, 0);  // Send Pump 2 status to Blynk (OFF)
  }
}

// This function handles the control of the white LED via Blynk (Virtual Pin V1)
BLYNK_WRITE(V1) {
  int whiteLedState = param.asInt(); // Get the button state from the Blynk app
  if (whiteLedState == 1) {
    digitalWrite(WHITE_LED_PIN, HIGH); // Turn ON the white LED
  } else {
    digitalWrite(WHITE_LED_PIN, LOW);  // Turn OFF the white LED
  }
}
