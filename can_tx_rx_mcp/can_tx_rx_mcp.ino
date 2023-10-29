#include<SPI.h>
#include<mcp2515.h>

/* 
 *  author bostonrob 
 *  2023.0.29
 */
struct can_frame canMsg,frame_tx;

//this is help to tranfer data type in fast way
union{
  float f;
  char buf[4];
}dataTrans;

enum ControlMode{
  vel_mode=0x02,
  pos_mode
};

enum Status{
  idle_mode=0x01,
  closeloop_mode=0x08
};

unsigned long previousMillis = 0; // 
const unsigned long interval = 10000; // 

float pos=0;
float vel=0;
float tarpos=0;
float tarvel=0;
String buf;

MCP2515 mcp2515(10);

void setup() {
  // put your setup code here, to run once:
  while(!Serial); //open serial 
  Serial.begin(9600);
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_250KBPS,MCP_8MHZ);  
  mcp2515.setNormalMode();
  Serial.println("------- CAN Read ----------");
  Serial.println("ID  DLC   DATA");
  //set_axis_status(frame_tx);
  delay(1000);
  // set_control_mode(frame_tx);
  
  
}

void loop() {
 // unsigned long currentMillis = millis(); // get current time
  // put your main code here, to run repeatedly: this function will be blcok in 10ms maybe
    Getpoistion(canMsg);
    uartReceive();
   if(buf.length() > 0) 
   {  
      //buf=buf.substring(0,buf.length()-1);//截取字符串，舍弃最后一位 回车符
      // Run calibration sequence 
       if(buf[0] == 'M' ) 
      {   
        //解析
        buf.remove(0,1);
        int mode=buf.toInt();
        switch (mode){
          case 1:
            Serial.println("position mode");
            setControlmode(frame_tx,pos_mode);
            delay(1000);
            break;
          case 2:
            Serial.println("velocity mode");
            setControlmode(frame_tx,vel_mode);
            delay(1000);
            break;
          default:
            break;
        }
        buf="";
        set_vel(frame_tx, tarpos);
      }
       if(buf[0] == 'A' ) 
      {   
        //解析
        buf.remove(0,1);
        int mode=buf.toInt();
        switch (mode){
          case 1:
            Serial.println("idle mode");
            set_axis_status(frame_tx,idle_mode);
            delay(1000);
            break;
          case 2:
            Serial.println("control mode");
            set_axis_status(frame_tx,closeloop_mode);
            delay(1000);
            break;
          default:
            break;
        }
        buf="";
        set_vel(frame_tx, tarpos);
      }
      if(buf[0] == 'p' ) 
      {   
        //解析
        buf.remove(0,1);
        tarpos=buf.toFloat();
        Serial.println("tarpos");
        Serial.println(tarpos);
        delay(1000);
        buf="";
        set_pos(frame_tx, tarpos);
      }
         
      if(buf[0] == 'v' ) 
      {   
        //解析
        buf.remove(0,1);
        tarvel=buf.toFloat();
        Serial.println("tarvel");
        Serial.println(tarvel);
        delay(1000);
        buf="";
        set_vel(frame_tx, tarvel);
      } 

     
 
 }
}

void Askpostion(can_frame& frame_tx)
{
  frame_tx.can_id=0x029;
  frame_tx.can_dlc=8;
  frame_tx.data[0] = 0x00;
  frame_tx.data[1] = 0x00;
  frame_tx.data[2] = 0x00;
  frame_tx.data[3] = 0x00;
  frame_tx.data[4] = 0x00;
  frame_tx.data[5] = 0x00;
  frame_tx.data[6] = 0x00;
  frame_tx.data[7] = 0x00;
  // send can frame
  if (mcp2515.sendMessage(&frame_tx) ==  MCP2515::ERROR_OK) 
  {
      Serial.println("CAN Frame Sent Successfully!");
  } else 
  {
      Serial.println("Error Sending CAN Frame...");
  }
  
}
void setControlmode(can_frame& frame_tx,ControlMode mode)
{
  frame_tx.can_id=0x02b;
  frame_tx.can_dlc=8;
  frame_tx.data[0] = mode;
 
  frame_tx.data[1] = 0x00;
  frame_tx.data[2] = 0x00;
  frame_tx.data[3] = 0x00;
   if (mode==pos_mode){
       frame_tx.data[4] = 0x05;
   }
   else 
   {
    frame_tx.data[4] = 0x02;
   }
  frame_tx.data[5] = 0x00;
  frame_tx.data[6] = 0x00;
  frame_tx.data[7] = 0x00;
  // send can frame
  if (mcp2515.sendMessage(&frame_tx) ==  MCP2515::ERROR_OK) 
  {
      Serial.println("mode setting Successfully!");
  } else 
  {
      Serial.println("mode setting fail");
  }
  
}


void set_axis_status(can_frame& frame_tx,Status mode)
{
  frame_tx.can_id=0x027;
  frame_tx.can_dlc=8;
  frame_tx.data[0] = mode;
  frame_tx.data[1] = 0x00;
  frame_tx.data[2] = 0x00;
  frame_tx.data[3] = 0x00;
  frame_tx.data[4] = 0x00;
  frame_tx.data[5] = 0x00;
  frame_tx.data[6] = 0x00;
  frame_tx.data[7] = 0x00;
  // 
  if (mcp2515.sendMessage(&frame_tx) ==  MCP2515::ERROR_OK) 
  {
      Serial.println("set status successful!");
  } else 
  {
      Serial.println("Error Sending CAN Frame...");
  }
  
}


void set_pos(can_frame& frame_tx, float &tarpos)
{
  dataTrans.f=tarpos;
  frame_tx.can_id=0x02C;
  frame_tx.can_dlc=8;
  frame_tx.data[0] = dataTrans.buf[0];
  frame_tx.data[1] = dataTrans.buf[1];
  frame_tx.data[2] = dataTrans.buf[2];
  frame_tx.data[3] = dataTrans.buf[3];
  frame_tx.data[4] = 0x00;
  frame_tx.data[5] = 0x00;
  frame_tx.data[6] = 0x00;
  frame_tx.data[7] = 0x00;
  // 
  if (mcp2515.sendMessage(&frame_tx) ==  MCP2515::ERROR_OK) 
  {
      Serial.println("tarpos Sent Successfully!");
      Serial.println("tarpos");
      Serial.println(tarpos);
      delay(1000);
  } 
  else 
  {
      Serial.println("tarpos Sent fail");
  }
  
}


void set_vel(can_frame& frame_tx, float &tar_vel)
{
  dataTrans.f=tar_vel;
  frame_tx.can_id=0x02D;
  frame_tx.can_dlc=8;
  frame_tx.data[0] = dataTrans.buf[0];
  frame_tx.data[1] = dataTrans.buf[1];
  frame_tx.data[2] = dataTrans.buf[2];
  frame_tx.data[3] = dataTrans.buf[3];
  frame_tx.data[4] = 0x00;
  frame_tx.data[5] = 0x00;
  frame_tx.data[6] = 0x00;
  frame_tx.data[7] = 0x00;
  // 
  if (mcp2515.sendMessage(&frame_tx) ==  MCP2515::ERROR_OK) 
  {
      Serial.println("vel Sent Successfully!");
      Serial.println("tarvel");
      Serial.println(tarvel);
      delay(1000);
  } 
  else 
  {
      Serial.println("vel Sent fail");
  }
  
}



void Getpoistion(can_frame& canMsg)
{
   // check can receive or not
    if (mcp2515.checkReceive() == true) 
    {
       if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) 
       {
        Serial.print(canMsg.can_id, HEX); // print ID
        Serial.print(" "); 
        Serial.print(canMsg.can_dlc, HEX); // print DLC length
        Serial.print(" ");
       
        for (int i = 0; i<canMsg.can_dlc; i++)  
        {  
          Serial.print(canMsg.data[i],HEX);
          Serial.print(" ");
        }
        Serial.println(); 
        if(canMsg.can_id==0x029)
        {
          dataTrans.buf[0]=canMsg.data[0];
          dataTrans.buf[1]=canMsg.data[1];
          dataTrans.buf[2]=canMsg.data[2];
          dataTrans.buf[3]=canMsg.data[3];
          pos=dataTrans.f;
          dataTrans.buf[0]=canMsg.data[4];
          dataTrans.buf[1]=canMsg.data[5];
          dataTrans.buf[2]=canMsg.data[6];
          dataTrans.buf[3]=canMsg.data[7];
          vel=dataTrans.f;
//          Serial.print("pos");
//          Serial.println(pos);
//          Serial.print("vel");
//          Serial.println(vel);
        }   
      }
    }
    else 
    {
      //Serial.println("no can data");
      return; 
    }
}

//this is for me to send command in serial monitor 
void uartReceive(){ 
    while (Serial.available()>0)  
   { 
    buf += char(Serial.read());
    delay(5);                    //延时防止乱码
   }
   

}
