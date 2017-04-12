
#include <mcp_can.h>
#include <SPI.h>

#define S0 8        //from S0 to S3 and sensorOut depends on which sensor pins are connected to arduino
#define S1 7
#define S2 6
#define S3 5
#define sensorOut 4

#define CAN0_INT 2
//#define DEBUG   //if ypu want to debug with serial communication uncomment this line

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];
int holdTime; 

MCP_CAN CAN0(9);     // Set CS to pin 9

unsigned char readFromSensor(char RGB){ //funstion for determining color
  if(RGB == 'R'){
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
  }else if(RGB == 'G'){
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);  
  }else if(RGB == 'B'){
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH); 
  }

  int frequency = pulseIn(sensorOut, LOW);

  frequency = map(frequency, 20,310,255,0); //scaling frequency 20 == 255 and 310 == 0 
  
  return frequency; 
  
}


void setup()
{
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  
  // Setting frequency-scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  
  #ifdef DEBUG
    Serial.begin(115200);
  #endif
  // Initialize MCP2515 running at 16MHz with a baudrate of 125kb/s and the masks and filters disabled.
  while (CAN0.begin(MCP_ANY, CAN_125KBPS, MCP_16MHZ) != CAN_OK) { 
    #ifdef DEBUG
      Serial.println("Error Initializing MCP2515...");
    #endif
  }

  #ifdef DEBUG
    Serial.println("MCP2515 Initialized Successfully!");
  #endif

  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
}

int i = 0;
void loop()
{
  //arrayOfColors storage values of R G and B in an array 
  unsigned char arrayOfColors[3] = {readFromSensor('R'), readFromSensor('G'), readFromSensor('B')};
  
  if(!digitalRead(CAN0_INT))             // If CAN0_INT pin is low, read receive buffer
  {
    len = 0;
    
    CAN0.readMsgBuf(&rxId, &len, rxBuf); //function for read message
    
    if ((int)rxId == 2000) {    //check if ID is valid for this node 
      if (len > 0) {            //check if message is recived
        
        #ifdef DEBUG            //just for debug if DEBUG macro is defined on the top
          Serial.print("Id:");    // it means that every step prints on serial
          Serial.print((int)rxId);
          Serial.println();
        #endif

        //master sends inside of buffer set of instructions
        if (rxBuf[0] != 0) {     //if buffer is NULL it means that this node shouldn't send enything
          
          holdTime = (int)rxBuf[0];
          
          #ifdef DEBUG      //also for debug only
            Serial.print("BUF: ");
            Serial.print(rxBuf[0]);
            Serial.println();
            Serial.print("----------------");
            Serial.println(i++);
          #endif
          
          //if buffer is different of NULL, while CAN message is OK, 
          //send feedback message, on every map(buffer) seconds  
          while(CAN0.sendMsgBuf(2000, 0, 3, arrayOfColors) != CAN_OK);
          delay(holdTime); 
     
        }
      }
    }
  }
}
