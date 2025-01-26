#include <HardwareSerial.h>
#include "DHT.h" // Include the DHT library

HardwareSerial GSM(1); // Use Serial1 for GSM module (RX, TX)

// Pin definitions
const int zPin = 32; // Z-axis (analog)
const int DHTPin = 4; // DHT22 data pin
#define DHTTYPE DHT22 // DHT sensor type

DHT dht(DHTPin, DHTTYPE); // Initialize DHT sensor

// Variables for Z-axis calculations
int zSum = 0;
int zReadings = 0;
int zMax = 0;

// Variables for temperature and humidity calculations
float tempSum = 0.0;
float humSum = 0.0;
int dhtReadings = 0;

unsigned long lastSendTime = 0; // To track when to send SMS
const unsigned long sendInterval = 30000; // 30 seconds

void setup() {
    Serial.begin(115200); // Serial monitor for debugging
    GSM.begin(9600, SERIAL_8N1, 16, 17); // GSM module on RX=16, TX=17
    dht.begin(); // Initialize DHT sensor
    delay(3000); // Wait for the module to initialize

    Serial.println("Initializing GSM Module...");
    GSM.println("AT");
    delay(1000);
    while (GSM.available()) {
        Serial.write(GSM.read());
    }
    
    // Set SMS mode to text (AT+CMGF=1)
    Serial.println("Setting SMS mode to Text...");
    GSM.println("AT+CMGF=1");
    delay(1000);
    while (GSM.available()) {
        Serial.write(GSM.read());
    }

    // Set the SMSC number (SMS Center Number)
    Serial.println("Setting SMSC...");
    GSM.println("AT+CSCA=\"+919849087001\""); // Replace with the correct SMSC number for your network
    delay(1000);
    while (GSM.available()) {
        Serial.write(GSM.read());
    }
}

void loop() {
    // Read Z-axis value from ADXL335
    int zValue = analogRead(zPin);

    // Accumulate the sum and update the maximum value
    zSum += zValue;
    zReadings++;
    if (zValue > zMax) zMax = zValue;

    // Read temperature and humidity from DHT22
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Check if DHT22 readings are valid
    if (!isnan(temperature) && !isnan(humidity)) {
        tempSum += temperature;
        humSum += humidity;
        dhtReadings++;
    }

    // Check if it's time to send SMS
    if (millis() - lastSendTime >= sendInterval) {
        // Calculate averages
        int zAvg = zSum / zReadings;
        float tempAvg = tempSum / dhtReadings;
        float humAvg = humSum / dhtReadings;

        // Send the SMS with the averages and maximum Z-axis values
        sendSMS(zAvg, zMax, tempAvg, humAvg);

        // Reset values for the next interval
        zSum = 0;
        zReadings = 0;
        zMax = 0;
        tempSum = 0.0;
        humSum = 0.0;
        dhtReadings = 0;
        lastSendTime = millis();
    }
}

void sendSMS(int zAvg, int zMax, float tempAvg, float humAvg) {
    Serial.println("Sending SMS...");
    GSM.println("AT+CMGS=\"+919363694950\""); // Replace with the receiver's number
    delay(1000);

    // Send the SMS message in one line
    GSM.print("Z-axis Avg: "); 
    GSM.print(zAvg); 
    GSM.print(", Max: "); 
    GSM.print(zMax); 
    GSM.print(", Temp Avg: ");
    GSM.print(tempAvg, 1); // 1 decimal point for temperature
    GSM.print("C, Hum Avg: ");
    GSM.print(humAvg, 1); // 1 decimal point for humidity
    GSM.print("%");
    GSM.write(26); // CTRL+Z to send the message

    delay(5000); // Wait for a response

    // Read the GSM module response
    while (GSM.available()) {
        Serial.write(GSM.read());
    }

    Serial.println("SMS Sent!");
}
