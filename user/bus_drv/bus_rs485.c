

#include "global.h"
#include "device_bus.h"
#include "bus_usart.h"
#include "Mesh_Node.h"
#include "BUS_RS485.h"
#include "si7013.h"
#include "G6_BT_Mesh.h"


void Rs485Init()
{
   
    GPIO_PinModeSet(RS485_TX_RX_PORT, RS485_TX_RX_PIN, gpioModePushPull, 0); //TX
    Rs485Rx();      //default
}

// change RS-485 to Tx status
void Rs485Tx()
{
    UsartSetStatus(USART_TX_WAITING,ON);
    UsartSetStatus(USART_RX_WAITING,OFF);
    RS485ToTx();
//    if(RS485GetPin() == LOW ) TraceErr("RS485 Tx PIN");
}

// change RS-485 to Rx status
void Rs485Rx()
{
    UsartSetStatus(USART_RX_WAITING,ON);
    UsartSetStatus(USART_TX_WAITING,OFF);
    USART_IntDisable(USART, USART_IEN_RXDATAV);
    RS485ToRx(); 
    USART_IntClear(USART, USART_IEN_RXDATAV);
    USART_IntEnable(USART, USART_IEN_RXDATAV);
//    if(RS485GetPin() == HIGH ) TraceErr("RS485 Rx PIN");
}
uchar Rs485DetectCount=0;
void Rs485StandbyMode()
{
    if(Rs485DetectCount-- != 0) {return;}
    else Rs485DetectCount = 100;
    if(RS485GetPin() == HIGH ) 
      {TraceErr("RS485 Rx PIN Hi");
        RS485ToRx();
      }
}

const uchar PzemClean[4]={0x01, 0x42, 0x80, 0x11};
const uchar PzemClibr[6]={0xF8, 0x41, 0x37, 0x21, 0xB7, 0x78};
 
//
// check RS-485 conect or not
//
uchar CheckRs485Device(int16 connectTryCount)
{
    uchar rs485_dev = SENSOR_DISCONNECT;

#ifdef ULTRA_SOUND_SKYNET
    pMeshNodeData->SensorClass = SENSOR_ULTRA_SOUND;rs485_dev = SENSOR_ULTRA_SOUND;
    rs485_dev = SENSOR_ULTRA_SOUND;  goto Assigned;
#endif

#ifdef BT_MESH_G6
       pMeshNodeData->SensorClass = SENSOR_BTM_G6;
#elif defined(BTM_TRANSMITTER)
       pMeshNodeData->SensorClass = SENSOR_CUSTOM_SERIAL;
#endif

#if defined(BTM_A308) && A308_SIMULATION
       pMeshNodeData->SensorClass = SENSOR_A308M;
#endif

    dprint("BTM Sensor type:%d\r\n", pMeshNodeData->SensorClass);
    if(pMeshNodeData->SensorClass == SENSOR_PZEM){
        CHECK_RS485_CMD((PUCHAR)&PzemClean[0],sizeof_array(PzemClean)); UsartResetRxTx(USART_ID_TX_RX);
        Delay_ms(500); SetLedToggle(LED_GREEN);
        pFunSensor = GetPzem;GetDeviceInfoDelay = 5;
        SetNodeStatus(NS_SERVER_RS485_ENABLE,ON);
        rs485_dev = SENSOR_PZEM; goto Check485_End;
        }    
    else if(pMeshNodeData->SensorClass == SENSOR_OEM){
        pFunSensor = GetOemSensor;GetDeviceInfoDelay = 5;
        SetNodeStatus(NS_SERVER_RS485_ENABLE,ON);
        rs485_dev = SENSOR_OEM; goto Check485_End;
        }
    else if(pMeshNodeData->SensorClass == SENSOR_AGB_POWER){
        pFunSensor = GetAgbPower;GetDeviceInfoDelay = 5;
        SetNodeStatus(NS_SERVER_RS485_ENABLE,ON);
        rs485_dev = SENSOR_AGB_POWER; goto Check485_End;
        }    
    else if(pMeshNodeData->SensorClass == SENSOR_BTM_G6){Trace("BTM G6");
        SetNodeStatus(NS_G6_READY|NS_SYS_NO_WAITING,ON);
        pFunSensor = GetBtmG6Info;GetDeviceInfoDelay = 5;
        SetNodeStatus(NS_SERVER_RS485_ENABLE,ON);
        rs485_dev = SENSOR_BTM_G6; goto Check485_End;
        }    
    else if(pMeshNodeData->SensorClass == SENSOR_VELOCITY){
        pFunSensor = GetVelocityInfo;GetDeviceInfoDelay = 5;
        SetNodeStatus(NS_SERVER_RS485_ENABLE,ON);
        rs485_dev = SENSOR_VELOCITY; goto Check485_End;
        }
    else if(pMeshNodeData->SensorClass == SENSOR_JYGD15){
    	pFunSensor = GetJYGD15Info;GetDeviceInfoDelay = 5;
    	 SetNodeStatus(NS_SERVER_RS485_ENABLE,ON);
    	 rs485_dev = SENSOR_JYGD15; goto Check485_End;
    }
#ifdef BTM_TRANSMITTER
    else if(pMeshNodeData->SensorClass == SENSOR_CUSTOM_SERIAL){
    	pFunSensor = GetCustomSerial;GetDeviceInfoDelay = 5;
        SetNodeStatus(NS_SERVER_RS485_ENABLE,ON);
        rs485_dev = SENSOR_CUSTOM_SERIAL; goto Check485_End;
    }
#elif defined(BTM_A308) && A308_SIMULATION
    else if(pMeshNodeData->SensorClass == SENSOR_A308M){
    	pFunSensor = GetA308mInfo;GetDeviceInfoDelay = 80;
		SetNodeStatus(NS_SERVER_RS485_ENABLE,OFF);
		rs485_dev = SENSOR_A308M; goto Check485_End;
    }
#endif

    if(Si7013_Detect(I2C0, SI7021_ADDR, NULL) == TRUE)
      {
    	//dprint("!!! Si7013_Detected !!!\r\n");
        if(CheckUltraSound() == TRUE){
            GetUltraSoundInfo();
            pMeshNodeData->SensorClass = SENSOR_ULTRA_SOUND;rs485_dev = SENSOR_ULTRA_SOUND;  goto Assigned;
        }
        if(CheckCDMCo2() != TRUE){
            pMeshNodeData->SensorClass = SENSOR_SI7021;rs485_dev = SENSOR_SI7021;  goto Assigned;
        }
        else{ 
            pMeshNodeData->SensorClass = SENSOR_SKYNET_CO2;rs485_dev = SENSOR_SKYNET_CO2;  goto Assigned;
        }       
      }
    else if(CheckCDMCo2()){
    	dprint("!!! Si7013 is not existed !!!\r\n");
    	pMeshNodeData->SensorClass = SENSOR_SKYNET_CO2;rs485_dev = SENSOR_SKYNET_CO2;  goto Assigned;
    }
  
    if(CheckRs485Connect(connectTryCount) == FALSE)
        {pMeshNodeData->SensorClass = SENSOR_DISCONNECT; goto Assigned;}
    
    if(pMeshNodeData->SensorClass != SENSOR_DISCONNECT)
      {//check RS-485 device whether exist or not
       rs485_dev = pMeshNodeData->SensorClass;
       switch(rs485_dev)
        {
            case SENSOR_PT485:
                if(CheckPT485() != TRUE)    rs485_dev  = SENSOR_DISCONNECT;  break;
            case SENSOR_AIP:
                if(CheckAIP() != TRUE)      rs485_dev  = SENSOR_DISCONNECT; break;
            case SENSOR_WATER_LEVEL:
                if(CheckWaterLevel() != TRUE) rs485_dev  = SENSOR_DISCONNECT;  break;
            case SENSOR_JNC_SD:
                if(CheckJncSd() != TRUE)    rs485_dev  = SENSOR_DISCONNECT;  break;
            case SENSOR_ULTRA_SOUND:
                if(CheckUltraSound() != TRUE) rs485_dev  = SENSOR_DISCONNECT; break;
            case SENSOR_A6D6:
                if(CheckA6D6() != TRUE)     rs485_dev  = SENSOR_DISCONNECT; break; 
            case SENSOR_IAQS:
                if(CheckIaqs() != TRUE)     rs485_dev  = SENSOR_DISCONNECT;  break;
            case SENSOR_CW9:
                if(CheckCw9() != TRUE)      rs485_dev  = SENSOR_DISCONNECT; break;
            case SENSOR_A308M:
                if(CheckA308M() != TRUE)    rs485_dev  = SENSOR_DISCONNECT; break;
#ifdef  BT_MESH_G6
                
            case SENSOR_BTM_G6:
                if(CheckBtmG6() != TRUE)       rs485_dev  = SENSOR_DISCONNECT; break;
#endif                
            default: TraceErr1("CheckRs485Device",rs485_dev);
                rs485_dev = SENSOR_DISCONNECT;  break;
        };
      }
Assigned:

     if(rs485_dev == SENSOR_DISCONNECT)  rs485_dev = ScanRs485Device();     
    pMeshNodeData->SensorClass = rs485_dev;
    
   //TraceDec1("*** rs485_dev ***",rs485_dev);
    if(rs485_dev == SENSOR_PT485)       {pFunSensor = GetPT485Info;GetDeviceInfoDelay = 5;}
    else if(rs485_dev == SENSOR_AIP)    {pFunSensor = GetAipInfo;GetDeviceInfoDelay = 5;}
    else if(rs485_dev == SENSOR_A308M)  {pFunSensor = GetA308mInfo;GetDeviceInfoDelay = 80;} //80 for wakeup timer
    else if(rs485_dev == SENSOR_WATER_LEVEL) {pFunSensor = GetWaterLevelInfo;GetDeviceInfoDelay = 5; }
    else if(rs485_dev == SENSOR_JNC_SD) {pFunSensor = GetJncSdInfo;GetDeviceInfoDelay = 5;} 
    else if(rs485_dev == SENSOR_ULTRA_SOUND ) {pFunSensor = GetUltraSoundInfo; GetDeviceInfoDelay = 50;} 
    else if(rs485_dev == SENSOR_A6D6)   {pFunSensor = GetA6D6Info; GetDeviceInfoDelay = 2;}
    else if(rs485_dev == SENSOR_SI7021) {pFunSensor = GetSkynetInfo;GetDeviceInfoDelay = 5;} //for default function
    else if(rs485_dev == SENSOR_RELAY)  {pFunSensor = GetRelay; GetDeviceInfoDelay = 2;}
    else if(rs485_dev == SENSOR_IAQS)   {pFunSensor = GetIaqsInfo; GetDeviceInfoDelay = 2;}
    else if(rs485_dev == SENSOR_CW9 )   {pFunSensor = GetCw9Info; GetDeviceInfoDelay = 2;}
    else if(rs485_dev == SENSOR_SKYNET_CO2) {pFunSensor = GetSkynetCo2Info; GetDeviceInfoDelay = 2;}
    else if(rs485_dev == SENSOR_BTM_G6)     {pFunSensor = GetBtmG6Info; GetDeviceInfoDelay = 2;}
    else if(rs485_dev == SENSOR_VELOCITY)   {pFunSensor = GetVelocityInfo; GetDeviceInfoDelay = 2;}
    else {TraceErr("Sensor Check");} // 


Check485_End:    

	dprint("*** CheckRs485Device: %d\r\n", rs485_dev);
    if(rs485_dev == SENSOR_SI7021 || rs485_dev == SENSOR_RELAY){
         SetNodeStatus(NS_SERVER_RS485_ENABLE,OFF);
        }
    else{
         SetLedStatus(LED_STATUS_SERVER_TO_RS485);
         SetLedStatus(LED_STATUS_SERVER_IO_CHANGE);
         SetNodeStatus(NS_SERVER_RS485_ENABLE,ON);        
        }
    UsartResetRxTx(USART_ID_TX_RX);
    WriteNodeData();
    //Printf("CheckRs485Device() dev:%d, func:%d\r\n",rs485_dev,pFunSensor);
    //Printf("GetVelocityInfo:%d\r\n",GetVelocityInfo);
   return rs485_dev;
}



//
// sned modbus cmd for PT485 to get tempature value
//
bool ModbusCmdPT485()
{
    uchar modbus_cmd[8]={0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA}; //to get value of temperature
    bool ret_code = TRUE;
        
    UsartTxSendCmd(modbus_cmd,MODBUS_CMD_NUM);

    return ret_code;
}

//
// sned modbus cmd for PT485 to get tempature value
//
bool ModbusCmdAIP()
{
    bool ret_code = TRUE;
    uchar get_power_status_cmd[8]={0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFD, 0xCA}; // for power status
    uchar get_power_watt_cmd[8]={0x01, 0x04, 0x00, 0x04, 0x00, 0x01,0x70, 0x0B}; // for power consumption
    
        
    UsartTxSendCmd(get_power_status_cmd,MODBUS_CMD_NUM);

    return ret_code;
}


//
// return value of PT485
//
uint16 Rs485ValuePT485()
{
    uint16 ret_code = 0;
    PUCHAR p_buff;
    p_buff = UsartGetBuff(USART_ID_RX);
    ret_code = WordSwap(*((PUINT16)&p_buff[3]));
    TraceDec1("PT485 Temp", ret_code);
    return ret_code;
}


void ModbusInitDelay(uint16 timer_ms)
{
    while(timer_ms > 500)
        {
          Delay_ms(500); SetLedToggle(LED_GREEN); 
          timer_ms -=500;
        };

}



//
//
//
bool CheckRs485Connect(int16 connectTryCount)
{
    bool ret_code = FALSE;
    int16 loop;
    if (connectTryCount<=0) connectTryCount=15;
   // return TRUE;
    uchar modbus_cmd1[8]={0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA}; //to get device model name
    uchar modbus_cmd[8]={0x01, 0x03, 0x04, 0x0A, 0x00, 0x02, 0xE5, 0x39}; //to get device model name
    SetLed(LED_BLUE,ON); SetLed(LED_RED,OFF);
    //for(loop=0; loop<15; loop++) //try 15 time for reading module name
    for(loop=0; loop<connectTryCount; loop++) //try 15 time for reading module name
        {
         CHECK_RS485_CMD(modbus_cmd,8); //command-1 take 200+15ms
         CHECK_RS485_CMD(modbus_cmd1,8);//command-2 take 200+15ms
        if(UsartGetRxCounter() != 0){ //leave detecting process if there is data has been received.
             ret_code = TRUE; break;           
            }
         Delay_ms(400); SetLedToggle(LED_BLUE); SetLedToggle(LED_RED);
        }
    if(ret_code == FALSE) dprint("RS-485 Disconnect. RxCounter:%d\r\n",UsartGetRxCounter());
    else dprint("RS-485 connect. RxCounter:%d\r\n",UsartGetRxCounter());
    UsartResetRxTx(USART_ID_TX_RX);
    SetLedStatus(LED_STATUS_OFF);
  
    return ret_code;
}


typedef struct 
{
    uchar ModelCmd[8];
    uchar ModelName[11];
    
}RS485Sensor,*PRS485Sensor;


 RS485Sensor Rs485Sensor[7]=
{
    //Command                                       //Response Data
    {//PT485     
     {0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}, {0x01, 0x03, 0x06, 0x54, 0x50, 0x38, 0x34, 0x20, 0x35, 0x79, 0xD4} },
    {//AIP
     {0x01, 0x03, 0x00, 0x20, 0x00, 0x03, 0x04, 0x01}, {0x01, 0x03, 0x06 ,0x49, 0x41, 0x20, 0x50, 0x20, 0x20, 0x00, 0xEA} },
    {//A308M
     {0x01, 0x03, 0x04, 0x0A, 0x00, 0x02, 0xE5, 0x39}, {0x01, 0x03, 0x04, 0x00, 0x00, 0x00, 0x01, 0x3B, 0xF3} },
    {//WaterLevel
     {0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}, {0x01, 0x03, 0x06, 0x4C, 0x57, 0x2D, 0x53, 0x31, 0x42, 0x36, 0x69} },
    {//JNC SD
     {0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}, {0x01, 0x03, 0x06, 0x53, 0x44, 0x00, 0x00, 0x00, 0x00, 0xDD, 0x19} },
    {//IAQS
     {0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}, {0x01, 0x03, 0x06, 0x41, 0x49, 0x53, 0x51, 0x32, 0x76, 0x27, 0xB9} },
    {//Ultra Sound
     {0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}, {0x01, 0x03, 0x06, 0x41, 0x49, 0x53, 0x51, 0x32, 0x76, 0x27, 0xB9} } 
};
//
// scan RS-485 device
//
uchar ScanRs485Device()
{
    uchar ret_code = SENSOR_DISCONNECT;
    
    if(CheckPT485() == TRUE) {
         ret_code = SENSOR_PT485;
        }
    else if(CheckAIP() == TRUE)  {
         ret_code = SENSOR_AIP;
        }
    else if(CheckA308M() == TRUE) {
         ret_code = SENSOR_A308M;
        }
    else if(CheckWaterLevel() == TRUE) {
         ret_code = SENSOR_WATER_LEVEL;
        }
    else if(CheckJncSd() == TRUE) {
         ret_code = SENSOR_JNC_SD;
        }
    else if(CheckUltraSound() == TRUE) {
         ret_code = SENSOR_ULTRA_SOUND;
        }    
    else if(CheckA6D6() == TRUE) {
         ret_code = SENSOR_A6D6;
        }
    else if(CheckIaqs() == TRUE) {
         ret_code = SENSOR_IAQS;
        }
    else if(CheckCw9() == TRUE) {
         ret_code = SENSOR_CW9;
        }
    else if(CheckBtmG6() == TRUE) {
         ret_code = SENSOR_BTM_G6;
        }    
    else {ret_code = SENSOR_RELAY;}

    return ret_code;
}

//
//
//
bool CheckCDMCo2()
{
    bool ret_code = FALSE;
    uchar device_name[6]={0xFE, 0x64, 0x0F, 0x00, 0x75, 0xE3}; //to get PT485 model name
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0){
         if(memcmp(device_name,UsartGetBuff(USART_ID_RX),sizeof(device_name)) == 0)
            {
            ret_code = TRUE;}
         else TraceErr("CDMCo2 Disconnect 1");
        }
    UsartResetRxTx(USART_ID_TX_RX);

    return ret_code;
}


#define DEVICE_NAME_LEN     sizeof(model_name) //8 //sizeof(model_name)


//
// Model Name = PT485
//
bool CheckPT485()
{
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; //to get PT485 model name
    uchar model_name[11]={0x01,0x03,0x06,0x54,0x50,0x38,0x34,0x20,0x35,0x79,0xD4};
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),DEVICE_NAME_LEN) == 0)
            {
            ret_code = TRUE;}
         else TraceErr("PT485 Disconnect 1");
        }
    else{ TraceErr("PT485 Disconnect 2");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}

//
// Check AIP connect
//
bool CheckAIP()
{
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x20, 0x00, 0x03, 0x04, 0x01}; //to get AIP model name
    uchar model_name[11]={0x01,0x03,0x06,0x49,0x41,0x20,0x50,0x20,0x20,0x00,0xEA};
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),DEVICE_NAME_LEN) == 0)
            {
            ret_code = TRUE; }
         else {TraceErr("AIP Disconnect 1"); }
        }
    else{ TraceErr("AIP Disconnect 2");
        }

    UsartResetRxTx(USART_ID_TX_RX);

    return ret_code;
}

const uchar A308MResetCmd[4][8]=
{
    {0x01, 0x06, 0x00, 0x0C, 0x00, 0x00, 0x49, 0xC9, }, //Reset X Bias
    {0x01, 0x06, 0x00, 0x0C, 0x00, 0x01, 0x88, 0x09, }, //Reset Y Bias
    {0x01, 0x06, 0x00, 0x0C, 0x00, 0x02, 0xC8, 0x08, }, //Reset Z Bias
    {0x01, 0x06, 0x00, 0x20, 0x00, 0x00, 0x88, 0x00, }, //Reset Auto setup x,y,z Bias
};
void ResetA308M()
{
	dprint("*** RestA308M ***\r\n");
 CHECK_RS485_CMD((PUCHAR)&A308MResetCmd[0],8); UsartResetRxTx(USART_ID_TX_RX);Delay_ms(1000); SetLedToggle(LED_GREEN);
 CHECK_RS485_CMD((PUCHAR)&A308MResetCmd[1],8); UsartResetRxTx(USART_ID_TX_RX);Delay_ms(1000); SetLedToggle(LED_GREEN);
 CHECK_RS485_CMD((PUCHAR)&A308MResetCmd[2],8); UsartResetRxTx(USART_ID_TX_RX);Delay_ms(1000); SetLedToggle(LED_GREEN);
 CHECK_RS485_CMD((PUCHAR)&A308MResetCmd[3],8); UsartResetRxTx(USART_ID_TX_RX);Delay_ms(1000); SetLedToggle(LED_GREEN);
}

const uchar CmdBtA308M_ver[MODBUS_CMD_NUM] ={0x01, 0x03, 0x04, 0x00, 0x00, 0x02, 0xC5, 0x3B};

//
// Check A308M connect
//
bool CheckA308M()
{
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x04, 0x05, 0x00, 0x01, 0x95, 0x3B}; //for new firmware
    uchar model_name[7]={0x01, 0x03, 0x02, 0x00, 0x01, 0x79, 0x84};
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),DEVICE_NAME_LEN) == 0)
            {
             if(ServerSendModbusCmd((PUCHAR)CmdBtA308M_ver,MODBUS_CMD_NUM) == TRUE)                
                ResetA308M();ret_code = TRUE;
            }
         else {TraceErr("A308M Disconnect 1"); }
        }
    else{ TraceErr("A308M Disconnect 2");
        }

    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}

bool CheckWaterLevel()
{
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; 
    uchar model_name[11]={0x01, 0x03, 0x06, 0x4C, 0x57, 0x2D, 0x53, 0x31, 0x42, 0x36, 0x69};
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),8) == 0)
            {
            ret_code = TRUE;}
         else TraceErr("Water Level Disconnect 2");
        }
    else{ TraceErr("Water Level Disconnect 3");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}


bool CheckUltraSound()
{
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; 
    uchar model_name[11]={0x01, 0x03, 0x06, 0x44, 0x55, 0x20, 0x20, 0x20, 0x20, 0x30, 0x2F  };// Ultra Sound
    
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),DEVICE_NAME_LEN) == 0){
           ret_code = TRUE;
         }
         else TraceErr("JNC-UltraSound Disconnect 2");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}




bool CheckJncSd()
{
    bool ret_code = FALSE;
    uchar loop;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; 
    uchar model_name[11]={0x01, 0x03, 0x06, 0x53, 0x44, 0x00, 0x00, 0x00, 0x00, 0xDD, 0x19};    
    
    CHECK_RS485_CMD(device_name,sizeof(device_name));        
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),DEVICE_NAME_LEN) == 0)
            {
            ret_code = TRUE;}
         else TraceErr("JNC-SD Disconnect 2");
        }
    else{ TraceErr("JNC-SD Disconnect 3");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}

bool CheckIaqs()
{
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; 
    uchar model_name[11]={0x01, 0x03, 0x06, 0x41, 0x49, 0x53, 0x51, 0x32, 0x76, 0x27, 0xB9};// IAQSV2
    
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),DEVICE_NAME_LEN) == 0)
            {
            ret_code = TRUE;}
         else TraceErr("IAQS Disconnect 2");
        }
    else{ TraceErr("IAQS Disconnect 3");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}

bool CheckCw9()
{
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; 
    uchar model_name[11]={0x01, 0x03, 0x06, 0x57, 0x43, 0x20, 0x39, 0x20, 0x20, 0xAA, 0x88};// CW9
    
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),DEVICE_NAME_LEN) == 0)
            {
            ret_code = TRUE;}
         else TraceErr("Cw9 Disconnect 2");
        }
    else{ TraceErr("Cw9 Disconnect 3");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}



bool CheckA6D6()
{
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; 
    uchar model_name[11]={0x01, 0x03, 0x06, 0x38, 0x41, 0x36, 0x44, 0x20, 0x20, 0x4E, 0x47   };
    
    CHECK_RS485_CMD(device_name,sizeof(device_name)); //UsartPrintBuff(USART_ID_RX);
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),DEVICE_NAME_LEN) == 0)
            {
            ret_code = TRUE;}
         else TraceErr("JNC-A6D6 Disconnect 2");
        }
    else{ TraceErr("JNC-A6D6 Disconnect 3");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}

bool CheckBtmG6()
{//TraceProc();
    bool ret_code=TRUE;
    ret_code = GetSysDate();

    if(ret_code) TraceOk("BTM G6");  else TraceErr("BTM G6");
    
    //return FALSE;
    return ret_code;
}


