#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

int block = 2;
byte blockcontent[16] = {"16CharacterStrng"};

byte readBackBlock[18];

#define S0 A0
#define S1 A1
#define S2 A2
#define S3 A3
#define sensorOut A4

int frequency = 0;

const int led1 = 6;
const int led2 = 7;

const int led3 = 3;
const int led4 = 2;

const int line = A5;

unsigned long currentMillis = 0;
unsigned long previousColourRed = 0;
unsigned long previousColourGreen = 0;
unsigned long previousColourBlue = 0;

const int button1 = 5;
const int button2 = 4;

bool rfidEnabled = true;



void setup() {
  // put your setup code here, to run once:
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(line, INPUT);
  
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(sensorOut, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.begin(9600);

  

  do {
    if (rfidEnabled && digitalRead(button2)){
      rfidEnabled = false;
      digitalWrite(led1, HIGH); // change this to play a higher frequency noise with the buzzer
      digitalWrite(led2, LOW);
    } else if (digitalRead(button2)){
      rfidEnabled = true;
      digitalWrite(led1, LOW); // change this to play a lower frequency noise with the buzzer
      digitalWrite(led2, HIGH);
    }
  } while (digitalRead(button1) == false);
  
  if (rfidEnabled){
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.println("RFID line");
    for (byte i = 0; i < 6; i++) {

      key.keyByte[i] = 0xFF;  //keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
    }
  }
  
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(line);
  //flashLeds();
  //colourSensor();
  if (rfidEnabled) { // checks if the rfid is going to be used 
    rfidFunc();
  }

  if (digitalRead(button1)){ // this will be replaced with another trigger
  digitalWrite(led1, HIGH); // motor 1 and motor 2 forward
  digitalWrite(led2, LOW);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, LOW);
  } else if (digitalRead(button2)){
    digitalWrite(led1, LOW); // motor 1 and motor 2 backwards
    digitalWrite(led2, HIGH);
    digitalWrite(led3, LOW);
    digitalWrite(led4, HIGH);
  } else {
    digitalWrite(led1, LOW); // both motors off
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
  }
}

void flashLeds() {
  digitalWrite(led1, HIGH);
  digitalWrite(led2, LOW);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, LOW);
  delay(500);
  digitalWrite(led1, LOW);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, LOW);
  digitalWrite(led4, HIGH);
  delay(500);
}

void colourSensor()
{
  // red photodiodes
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  // reading output frequency
  frequency = pulseIn(sensorOut, LOW);
  // printing the value
  Serial.print("R= ");
  Serial.print(frequency); // prints red colour frequency
  Serial.print(" ");
  delay(100);
  // green photodiodes
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  // reading output frequency
  frequency = pulseIn(sensorOut, LOW);
  // printing the value
  Serial.print("G= ");
  Serial.print(frequency); // prints green colour frequency
  Serial.print(" ");
  delay(100);
  // blue photodiodes
    digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  // reading output frequency
  frequency = pulseIn(sensorOut, LOW);
  // printing the value
  Serial.print("B= ");
  Serial.print(frequency); // prints blue colour frequency
  Serial.print(" ");
  Serial.println("");
  delay(1000);
}

void rfidFunc() 
{
    // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  Serial.println("card selected");

  //the blockcontent array is written into the card block
  writeBlock(block, blockcontent);

  //read the block back
  readBlock(block, readBackBlock);
  //uncomment below line if you want to see the entire 1k memory with the block written into it.
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  //print the block contents
  Serial.print("read block: ");
  for (int j=0 ; j<16 ; j++)
  {
    Serial.write (readBackBlock[j]);
  }

  Serial.println("");
}

int writeBlock(int blockNumber, byte arrayAdress[])
{
  //this makes sure that we only write into data blocks. Every 4th block is a trailer block for the access/security info.
  int largestModulo4Number = blockNumber/4*4;
  int trailerBlock = largestModulo4Number + 3;//determine trailer block for the sector
  if (blockNumber > 2 && (blockNumber + 1)%4 == 0){Serial.print(blockNumber);Serial.println(" is a trailer block:");return 2;}
  Serial.print(blockNumber);
  Serial.println(" is a data block:");

  

  //authentication of the desired block for access
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("PCD_Authenticate() failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 3;//return "3" as error message
  }

  

  //writing the block 
  status = mfrc522.MIFARE_Write(blockNumber, arrayAdress, 16);
  //status = mfrc522.MIFARE_Write(9, value1Block, 16);  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("MIFARE_Write() failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 4;//return "4" as error message
  }
  Serial.println("block was written");
}

int readBlock(int blockNumber, byte arrayAddress[]) 
{
  int largestModulo4Number=blockNumber/4*4;
  int trailerBlock=largestModulo4Number+3;//determine trailer block for the sector


  //authentication of the desired block for access
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print("PCD_Authenticate() failed (read): ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 3;//return "3" as error message
  }



  //reading a block
  byte buffersize = 18;//we need to define a variable with the read buffer size, since the MIFARE_Read method below needs a pointer to the variable that contains the size... 
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);//&buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
  if (status != MFRC522::STATUS_OK) {
    Serial.print("MIFARE_read() failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 4;//return "4" as error message
  }
  Serial.println("block was read");

}
