#include <SPI.h>
#include <mcp_can.h>

#define CAN_CS 10
MCP_CAN CAN(CAN_CS);

#define IR_SENSOR_PIN 2
#define LDR_SENSOR_PIN A1

void setup() {
  Serial.begin(9600);

  pinMode(IR_SENSOR_PIN, INPUT); // Fix #2

  if (CAN.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN Initialized Successfully!");
  } else {
    Serial.println("CAN Initialization Failed!");
    while (1);
  }

  CAN.setMode(MCP_NORMAL);
  Serial.println("Slave 2 Transmitter Ready");
}

void loop() {
  int irStatus = digitalRead(IR_SENSOR_PIN);
  int ldrValue = analogRead(LDR_SENSOR_PIN);
  int lightLevel = map(ldrValue, 0, 1023, 0, 100);

  byte data[3];
  data[0] = irStatus;
  data[1] = (lightLevel >> 8) & 0xFF;  // Fix #1 - MSB
  data[2] = lightLevel & 0xFF;         // LSB

  if (CAN.sendMsgBuf(0x02, 0, 3, data) == CAN_OK) {
    Serial.println("Data Sent from Slave 2!");
  } else {
    Serial.println("Error Sending Data from Slave 2");
  }

  Serial.print("Slave 2 - IR Sensor: ");
  Serial.print(irStatus ? "HIGH" : "LOW");
  Serial.print(", Light Level: ");
  Serial.print(lightLevel);
  Serial.println(" %");

  delay(1000);
}