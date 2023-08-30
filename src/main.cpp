#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <mcp_can.h>
#include <stdio.h>
#include <stdlib.h>

// #define DEBUG

const char *msg_show[] = {
    "1fc01fdf#0100e001eeff0000"};

const char *msg_logo[] = {
    "1fbfdfff#0000FFFDFFFF7FF7", "1f9fdfff#0001010101010101",
    "1f9fdfff#0101010101010101", "1f9fdfff#0101010101010101",
    "1f9fdfff#0101010101010101", "1f9fdfff#0101010101010101",
    "1f9fdfff#0101010101010101", "1f9fdfff#0101010101010101",
    "1f9fdfff#0101010101010101", "1f7fdfff#FF7F",
    "1fbfdfff#0100FFFDFFFF7FF7", "1f9fdfff#0000010101000000",
    "1f9fdfff#0200000404080000", "1f9fdfff#0101010000000202",
    "1f9fdfff#0000040400080000", "1f9fdfff#0000000000020200",
    "1f9fdfff#0004040008080000", "1f9fdfff#0000000002020000",
    "1f9fdfff#0004000008000000", "1f7fdfff#FF7F",
    "1fbfbfff#0000FFFDFFFF7FF7", "1f9fbfff#0001010101010101",
    "1f9fbfff#0101010101010101", "1f9fbfff#0101010101010101",
    "1f9fbfff#0101010101010101", "1f9fbfff#0101010101010101",
    "1f9fbfff#0101010101010101", "1f9fbfff#0101010101010101",
    "1f9fbfff#0101010101010101", "1f7fbfff#FF7F",
    "1fbfbfff#0100FFFDFFFF7FF7", "1f9fbfff#0202020202000000",
    "1f9fbfff#0000000000080d07", "1f9fbfff#0202000000000000",
    "1f9fbfff#0000000000000906", "1f9fbfff#0404000000000000",
    "1f9fbfff#0000000000000906", "1f9fbfff#0404040404000000",
    "1f9fbfff#0000000000010b0e", "1f7fbfff#FF7F",
    "1fbf9fff#0000FFFDFFFF7FF7", "1f9f9fff#0001010101010101",
    "1f9f9fff#0101010101010101", "1f9f9fff#0101010101010101",
    "1f9f9fff#0101010101010101", "1f9f9fff#0101010101010101",
    "1f9f9fff#0101010101010101", "1f9f9fff#0101010101010101",
    "1f9f9fff#0101010101010101", "1f7f9fff#FF7F",
    "1fbf9fff#0100FFFDFFFF7FF7", "1f9f9fff#0000000004040000",
    "1f9f9fff#0002000001000000", "1f9f9fff#0000000000040400",
    "1f9f9fff#0002020001010000", "1f9f9fff#0808080000000404",
    "1f9f9fff#0000020200010000", "1f9f9fff#0000080808000000",
    "1f9f9fff#0400000202010000", "1f7f9fff#FF7F",
    "1fbf7fff#0000FFFDFFFF7FF7", "1f9f7fff#0001010101010101",
    "1f9f7fff#0101010101010101", "1f9f7fff#0101010101010101",
    "1f9f7fff#0101010101010101", "1f9f7fff#0101010101010101",
    "1f9f7fff#0101010101010101", "1f9f7fff#0101010101010101",
    "1f9f7fff#0101010101010101", "1f7f7fff#FF7F",
    "1fbf7fff#0100FFFDFFFF7FF7", "1f9f7fff#0606060606060606",
    "1f9f7fff#0404000000000000", "1f9f7fff#0f06060606060404",
    "1f9f7fff#0404040404000001", "1f9f7fff#0f0f0f0f05040404",
    "1f9f7fff#0404040501010101", "1f9f7fff#0e0e0c0d0d0d0d05",
    "1f9f7fff#0505010101000000", "1f7f7fff#FF7F",
    "1fbf5fff#0000FFFDFFFF7FF7", "1f9f5fff#0001010101010101",
    "1f9f5fff#0101010101010101", "1f9f5fff#0101010101010101",
    "1f9f5fff#0101010101010101", "1f9f5fff#0101010101010101",
    "1f9f5fff#0101010101010101", "1f9f5fff#0101010101010101",
    "1f9f5fff#0101010101010101", "1f7f5fff#FF7F",
    "1fbf5fff#0100FFFDFFFF7FF7", "1f9f5fff#0f07070707070707",
    "1f9f5fff#0707060602020202", "1f9f5fff#0f0f0f0f07070706",
    "1f9f5fff#0606060606060602", "1f9f5fff#0f0f0f0f0e0e0e06",
    "1f9f5fff#0606060606060604", "1f9f5fff#0f0e0e0e0e0e0e0e",
    "1f9f5fff#0e0e060604040404", "1f7f5fff#FF7F",
    "1fbf3fff#0000FFFDFFFF7FF7", "1f9f3fff#0001010101010101",
    "1f9f3fff#0101010101010101", "1f9f3fff#0101010101010101",
    "1f9f3fff#0101010101010101", "1f9f3fff#0101010101010101",
    "1f9f3fff#0101010101010101", "1f9f3fff#0101010101010101",
    "1f9f3fff#0101010101010101", "1f7f3fff#FF7F",
    "1fbf3fff#0100FFFDFFFF7FF7", "1f9f3fff#0707030b0b0b0b0b",
    "1f9f3fff#0a0a080808000000", "1f9f3fff#0f0f0f0f0a020202",
    "1f9f3fff#0202020a08080808", "1f9f3fff#0f06060606060202",
    "1f9f3fff#0202020202000008", "1f9f3fff#0606060606060606",
    "1f9f3fff#0202020000000000", "1f7f3fff#FF7F",
    "1fbf1fff#0000FFFDFFFF7FF7", "1f9f1fff#0001010101010101",
    "1f9f1fff#0101010101010101", "1f9f1fff#0101010101010101",
    "1f9f1fff#0101010101010101", "1f9f1fff#0101010101010101",
    "1f9f1fff#0101010101010101", "1f9f1fff#0101010101010101",
    "1f9f1fff#0101010101010101", "1f7f1fff#FF7F",
    "1fbf1fff#0100FFFDFFFF7FF7", "1f9f1fff#000008040c080c0e",
    "1f9f1fff#0c0c0e0f0f0f0606", "1f9f1fff#000008000c0c080c",
    "1f9f1fff#0e0e0c0e0e0f0f0f", "1f9f1fff#00000808000c0c08",
    "1f9f1fff#0c0e0e0c0c0e0e0e", "1f9f1fff#0000000800000c08",
    "1f9f1fff#080c0e0e0c0c0c0e", "1f7f1fff#FF7F",
    "1fbeffff#0000FFFDFFFF7FF7", "1f9effff#0001010101010101",
    "1f9effff#0101010101010101", "1f9effff#0101010101010101",
    "1f9effff#0101010101010101", "1f9effff#0101010101010101",
    "1f9effff#0101010101010101", "1f9effff#0101010101010101",
    "1f9effff#0101010101010101", "1f7effff#FF7F",
    "1fbeffff#0100FFFDFFFF7FF7", "1f9effff#070d0a060e0f0f0f",
    "1f9effff#0f0f0f070707070f", "1f9effff#0609060f0f0e0e0e",
    "1f9effff#070707070f0f0f0f", "1f9effff#0609060f0f070707",
    "1f9effff#0e0e0e0e0f0f0f0f", "1f9effff#0e0b0506070f0f0f",
    "1f9effff#0f0f0f0e0e0e0e0f", "1f7effff#FF7F",
    "1fbedfff#0000FFFDFFFF7FF7", "1f9edfff#0001010101010101",
    "1f9edfff#0101010101010101", "1f9edfff#0101010101010101",
    "1f9edfff#0101010101010101", "1f9edfff#0101010101010101",
    "1f9edfff#0101010101010101", "1f9edfff#0101010101010101",
    "1f9edfff#0101010101010101", "1f7edfff#FF7F",
    "1fbedfff#0100FFFDFFFF7FF7", "1f9edfff#0000000100000301",
    "1f9edfff#0103070703030307", "1f9edfff#0000010100030301",
    "1f9edfff#0307070303070707", "1f9edfff#0000010003030103",
    "1f9edfff#07070307070f0f0f", "1f9edfff#0000010203010307",
    "1f9edfff#0303070f0f0f0606", "1f7edfff#FF7F"
};

#define CAN0_INT 4
MCP_CAN CAN0(15);

void init_mcp();
void send_message(const char *messages[], int num_messages);

// CAN RX Variables
long unsigned int rxId;
unsigned char len;
unsigned char rxBuf[8];

// Serial Output String Buffer
char msgString[128];

void setup() {
  Serial.begin(9600);
  init_mcp();

  send_message(msg_logo, sizeof(msg_logo) / sizeof(msg_logo[0]));
}

void loop() {
  yield();
  delay(1000);
  send_message(msg_show, sizeof(msg_show) / sizeof(msg_show[0]));
}

void init_mcp() {
  // Initialize MCP2515 running at 8MHz with a baudrate of 1000kb/s and the
  // masks and filters disabled.
  if (CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) {
#ifdef DEBUG
    Serial.println("MCP2515 Initialized Successfully!");
#endif
    delay(1000);
  } else {
#ifdef DEBUG
    Serial.println("Error Initializing MCP2515...");
#endif
    delay(2000);
    setup();
  }

  CAN0.setMode(MCP_NORMAL); // Set operation mode to MCP_NORMAL so the MCP2515
                            // sends acks to received data.

  pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input

  // Serial.println("MCP2515 Ready...");
}

void send_message(const char *messages[], int num_messages) {
  // loop through the char array
  // int num_messages = sizeof(messages) / sizeof(messages[0]);
  for (int i = 0; i < num_messages; i++) {

    char msg[50] = "";
    char id[9] = "";
    char data[17] = "";
    strlcpy(msg, messages[i], 50);

#ifdef DEBUG
    Serial.printf("Sending message %d/%d: ", i + 1, num_messages);
#endif

    // split the string into id and data
    char *token = strtok(msg, "#");
    if (token != NULL) {
      // Serial.printf("ID: %s\n", token);
      strlcpy(id, token, 9);
    }

    token = strtok(NULL, "#");
    if (token != NULL) {
#ifdef DEBUG
      Serial.printf("Data: %s (%d)\n", token, strlen(token));
#endif
      strlcpy(data, token, strlen(token) + 1);
    }
#ifdef DEBUG
    Serial.printf("ID: %s, Data: %s (%d)\n", id, data, strlen(data));
#endif

    // convert the id to a long
    uint32_t id_long = strtoul(id, NULL, 16);

    // convert data to a byte array
    unsigned int array_size = strlen(data) / 2;
    byte data_byte[array_size];
    for (size_t i = 0; i < strlen(data); i++) {
      char hexPair[3]; // To store two hex characters and the null terminator
      hexPair[0] = data[i * 2];     // Get the first character of the pair
      hexPair[1] = data[i * 2 + 1]; // Get the second character of the pair
      hexPair[2] = '\0';            // Null-terminate the string

      data_byte[i] = strtol(hexPair, NULL, 16);
      // Serial.printf("Hex: %s, Dec: %d\n", hexPair, data_byte[i]);
    }
#ifdef DEBUG
    Serial.printf("ID: %X, Data:", id_long);
    for (unsigned int i = 0; i < array_size; i++) {
      Serial.printf("%X", data_byte[i]);
    }
    Serial.printf("\n");
#endif

    byte sndStat = CAN0.sendMsgBuf(id_long, 1, array_size, data_byte);
#ifdef DEBUG
    if (sndStat == CAN_OK) {
      Serial.printf("successfully send %d bytes!\n", array_size);
    } else {
      Serial.printf("error!\n");
    }
#endif
    // Serial.printf("ID: %s, Data: %s\n", id_long, data);

    yield();
  }
}