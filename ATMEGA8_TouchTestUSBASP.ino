 //CODE WORKING WITH UNO WITH HW SERIAL
#define CMD_HEAD1 0x5A
#define CMD_HEAD2 0xA5
#define CMD_WRITE 0x82
#define CMD_READ 0x83
#define CMD_PAGESWITCH 0x84

//Functions list:
//DWIN
void writeToVP(uint16_t vpAddress, uint16_t value);
uint16_t readVP(uint16_t vpAddress);
void switchToPage(byte pageNumber);

uint16_t currentPage = 0xFFFF;  // Unknown at start
uint16_t lastPage = 0xFFFF;     // For detecting changes
unsigned long lastPageCheckTime = 0;
const unsigned long pageCheckInterval = 500; // ms
    unsigned long therapyStartTime = 0; 

void setup() 
{  
  Serial.begin(9600);  
  delay(1000);  // Display initialization
      switchToPage(1);

}

void loop() {
  if (millis() - lastPageCheckTime >= pageCheckInterval) {
    lastPageCheckTime = millis();
    writeToVP(0x5000, 100);
    Serial.println("Ping...");
      unsigned long elapsed = millis() - therapyStartTime; 
      formatElapsedTime(elapsed); // Directly print formatted time
    }
  }

void formatElapsedTime(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  seconds %= 60;
  minutes %= 60;

  char buffer[11]; // 10 chars + null terminator
  sprintf(buffer, "%03lu:%02lu:%02lu ", hours, minutes, seconds);
  // Now send to DWIN text box at VP 0x5030
  writeTextToVP(0x5030, buffer);
}

// Function to write a 16-bit value to a given VP address
void writeToVP(uint16_t vpAddress, uint16_t value) {
  byte cmd[8] = {
    CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE,
    highByte(vpAddress), lowByte(vpAddress),
    highByte(value),     lowByte(value)
  };

  Serial.write(cmd, 8);
}

// Clean VP reading function
uint16_t readVP(uint16_t vpAddress) {
  // 1. Clear buffers
  Serial.flush();
  while(Serial.available()) Serial.read();

  // 2. Send read command
  byte cmd[7] = {
    CMD_HEAD1, CMD_HEAD2,             // Header
    0x04,                   // Length
    CMD_READ,                   // Read command
    highByte(vpAddress),    // Address high
    lowByte(vpAddress),     // Address low
    0x01                    // Read 1 word
  };
  Serial.write(cmd, 7);

  // 3. Wait for response
  byte response[9];
  byte idx = 0;
  unsigned long start = millis();
  
  while(millis() - start < 300) {  // 300ms timeout
    if(Serial.available()) {
      response[idx++] = Serial.read();
      
      // Complete response received (9 bytes)
      if(idx >= 9) {
        // Verify response structure
        if(response[0] == CMD_HEAD1 && response[1] == CMD_HEAD2 &&  // Header
           response[3] == CMD_READ) {                         // Read command
          // Return the value (bytes 7-8)
          return (response[7] << 8) | response[8];
        }
      }
    }
  }
  return 0xFFFF;  // Return error value
}

void switchToPage(byte pageNumber) {
  byte cmd[] = {
    CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE,
    0x00, CMD_PAGESWITCH, // VP address 0x0084 (system page switch register)
    0x5A, 0x01, // D3=0x5A, D2=0x01 (page switch)
    0x00, pageNumber // Picture ID (high, low)
  };
  Serial.write(cmd, sizeof(cmd));
}

void writeTextToVP(uint16_t vpAddress, const char* text) {
  uint8_t len = strlen(text);
  Serial.write(0x5A);
  Serial.write(0xA5);
  Serial.write(3 + 2 + len); // total length
  Serial.write(0x82);
  Serial.write(highByte(vpAddress));
  Serial.write(lowByte(vpAddress));
  for (uint8_t i = 0; i < len; i++) {
    Serial.write(text[i]);
  }
}

/*
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Serial test start...");
}

void loop() {
  Serial.println("Ping...");
  delay(1000);
}
*/