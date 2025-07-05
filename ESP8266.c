#include <SPI.h>
#include <mcp_can.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>

// WiFi Credentials
const char* ssid = "SSID";
const char* password = "******";

// SMTP Settings
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "*********@gmail.com"
#define AUTHOR_PASSWORD "***********"

// CAN Setup
#define CAN_CS 15 // ESP8266-specific pin mapping for SPI
MCP_CAN CAN(CAN_CS);

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables to store latest values
float temperature = 0.0;
uint16_t fuelLevel = 0;
uint16_t lightLevel = 0;
bool irStatus = false;

// Email content
String emailBody;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 Initialized");

  // WiFi connection
  WiFi.begin(ssid, password);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.setCursor(0, 1);
  lcd.print("WiFi Connected");

  // CAN init
  if (CAN.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN Initialized Successfully!");
    lcd.setCursor(0, 1);
    lcd.print("CAN Init Success");
  } else {
    Serial.println("CAN Initialization Failed!");
    lcd.setCursor(0, 1);
    lcd.print("CAN Init Failed!");
    while (1);
  }

  CAN.setMode(MCP_NORMAL);
  delay(1500);
  lcd.clear();
}

unsigned long lastEmailTime = 0; // Stores the last email send time
const unsigned long emailInterval = 1 * 60 * 1000; // 5 minutes in milliseconds

void loop() {
  unsigned long currentMillis = millis(); // Get current time in milliseconds

  unsigned long rxId;
  byte len = 0;
  byte buf[8];

  // Receive CAN message
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&rxId, &len, buf);

    if (rxId == 0x01 && len == 6) {
      memcpy(&temperature, buf, 4);
      memcpy(&fuelLevel, buf + 4, 2);
    } else if (rxId == 0x02 && len == 3) {
      irStatus = buf[0];
      lightLevel = ((uint16_t)buf[1] << 8) | buf[2];
    }

    // Display on LCD
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C F:");
    lcd.print(fuelLevel);
    lcd.print("% ");

    lcd.setCursor(0, 1);
    lcd.print("L:");
    lcd.print(lightLevel);
    lcd.print("% IR:");
    lcd.print(irStatus ? "0 " : "1 ");
  }

  // Check if it's time to send the email
  if (currentMillis - lastEmailTime >= emailInterval) {
    // Prepare email content
    emailBody = "Temperature: " + String(temperature, 1) + "C\n" +
                "Fuel Level: " + String(fuelLevel) + "%\n" +
                "Light Level: " + String(lightLevel) + "%\n" +
                "IR Status: " + String(irStatus ? "INactive" : "Active") + "\n";

    // Send email
    sendEmail("bodigenithya@gmail.com", "ESP8266 Sensor Data", emailBody.c_str());

    lastEmailTime = currentMillis; // Update the last email time
  }

  delay(200); // Short delay for stability
}


// Updated sendEmail function
void sendEmail(const char *recipient, const char *subject, const char *message) {
  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;

  SMTP_Message email;
  email.sender.name = "ESP8266 Monitor";
  email.sender.email = AUTHOR_EMAIL;
  email.subject = subject;
  email.addRecipient("Recipient", recipient);
  email.text.content = message;

  SMTPSession smtp;
  if (!smtp.connect(&config)) {
    Serial.println("Failed to connect to SMTP server.");
    return;
  }

  if (!MailClient.sendMail(&smtp, &email)) {
    Serial.println("Failed to send email.");
  } else {
    Serial.println("Email sent successfully.");
  }

  smtp.closeSession(); // Close the session to free resources
}