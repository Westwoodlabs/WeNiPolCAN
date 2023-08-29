#include <Arduino.h>
#include <Wire.h>
#include <mcp_can.h>
#include <SPI.h>

#define CAN0_INT 4
MCP_CAN CAN0(15);

void init_mcp();

// CAN RX Variables
long unsigned int rxId;
unsigned char len;
unsigned char rxBuf[8];

// Serial Output String Buffer
char msgString[128];

void setup() {
  Serial.begin(9600);
  init_mcp();
}

void loop() {
  if(!digitalRead(CAN0_INT)) // If CAN0_INT pin is low, read receive buffer
    {
        CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)

        if((rxId & 0x80000000) == 0x80000000)             // Determine if ID is standard (11 bits) or extended (29 bits)
          sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
        else
          sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
      
        Serial.print(msgString);
      
        if((rxId & 0x40000000) == 0x40000000){            // Determine if message is a remote request frame.
          sprintf(msgString, " REMOTE REQUEST FRAME");
          Serial.print(msgString);
        } else {
          for(byte i = 0; i<len; i++){
            sprintf(msgString, " 0x%.2X", rxBuf[i]);
            Serial.print(msgString);
          }
        }
            
        Serial.println();
    }
}

void init_mcp() {
    // Initialize MCP2515 running at 8MHz with a baudrate of 1000kb/s and the masks and filters disabled.
    if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK)
    {
        Serial.println("MCP2515 Initialized Successfully!");
        delay(1000);
    }
    else
    {
        Serial.println("Error Initializing MCP2515...");
        delay(2000);
        setup();
    }

    CAN0.setMode(MCP_NORMAL); // Set operation mode to MCP_NORMAL so the MCP2515 sends acks to received data.

    pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input

    Serial.println("MCP2515 Ready...");
}