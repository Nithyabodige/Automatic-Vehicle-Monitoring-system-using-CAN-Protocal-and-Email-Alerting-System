#include <SPI.h>
#include <mcp_can.h>

// Define MCP2515 CS pin
#define CAN_CS 10

// Initialize MCP2515
MCP_CAN CAN(CAN_CS);

// Pin definitions for sensors
#define TEMP_SENSOR_PIN A0
#define ULTRASONIC_SENSOR_PIN A2

void setup() {
  Serial.begin(9600);

  // Initialize CAN module
  if (CAN.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN Initialized Successfully!");
  } else {
    Serial.println("CAN Initialization Failed!");
    while (1);
  }
  CAN.setMode(MCP_NORMAL); // Set MCP2515 to normal mode
  Serial.println("Slave 1 Transmitter Ready");
}

void loop() {
  // Read sensors
  float temperature = analogRead(TEMP_SENSOR_PIN) * (5.0 / 1023.0) * 100; // Example calculation for temperature in Celsius
  int waterLevel = map(analogRead(WATER_SENSOR_PIN), 0, 1023, 0, 100);  // Water level as a percentage (0-100%)

  // Prepare CAN message (6 bytes)
  byte data[6];
  memcpy(data, &temperature, 4);     // Copy temperature (4 bytes)
  memcpy(data + 4, &waterLevel, 2);  // Copy water level (2 bytes)

  // Send CAN message with ID for Slave 1
  if (CAN.sendMsgBuf(0x01, 0, 6, data) == CAN_OK) {
    Serial.println("Data Sent from Slave 1!");
  } else {
    Serial.println("Error Sending Data from Slave 1");
  }

  // Print data for debugging
  Serial.print("Slave 1 - Temperature: ");
  Serial.print(temperature);
  Serial.print(" C, Water Level: ");
  Serial.print(waterLevel);
  Serial.println(" %");

  delay(1000); // Send data every second
}