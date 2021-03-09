

#include "global.h"
#include "device_bus.h"
#include "bus_usart.h"
#include "Mesh_Node.h"
#include "BUS_RS485.h"
#include "si7013.h"


void Rs485Init()
{//TraceProc();
   
    GPIO_PinModeSet(RS485_TX_RX_PORT, RS485_TX_RX_PIN, gpioModePushPull, 0); //TX
    Rs485Rx();      //default
}

// change RS-485 to Tx status
void Rs485Tx()
{//TraceProc();
    UsartSetStatus(USART_TX_WAITING,ON);
    UsartSetStatus(USART_RX_WAITING,OFF);
    RS485ToTx(); //Delay_ms(5); 
//    if(RS485GetPin() == LOW ) TraceErr("RS485 Tx PIN");
}

// change RS-485 to Rx status
void Rs485Rx()
{//TraceProc();
    UsartSetStatus(USART_RX_WAITING,ON);
    UsartSetStatus(USART_TX_WAITING,OFF);
    USART_IntDisable(USART, USART_IEN_RXDATAV);
    RS485ToRx(); //Delay_ms(5); 
    USART_IntClear(USART, USART_IEN_RXDATAV);
    USART_IntEnable(USART, USART_IEN_RXDATAV);
//    if(RS485GetPin() == HIGH ) TraceErr("RS485 Rx PIN");
}
uchar Rs485DetectCount=0;
void Rs485StandbyMode()
{//TraceProc();
    if(Rs485DetectCount-- != 0) {return;}
    else Rs485DetectCount = 100;
    if(RS485GetPin() == HIGH ) 
      {TraceErr("RS485 Rx PIN Hi");
        RS485ToRx();
      }
}


 
//
// check RS-485 conect or not
//
uchar CheckRs485Device()
{TraceProc();
    uchar rs485_dev = SENSOR_DISCONNECT;

    if(Si7013_Detect(I2C0, SI7021_ADDR, NULL) == TRUE)
        {pMeshNodeData->SensorClass = SENSOR_DISCONNECT; goto Assigned;}
    if(CheckRs485Connect() == FALSE) 
        {pMeshNodeData->SensorClass = SENSOR_DISCONNECT; goto Assigned;}
    
    if(pMeshNodeData->SensorClass != SENSOR_DISCONNECT)
      {//check RS-485 device whether exist or not
       rs485_dev = pMeshNodeData->SensorClass;
       switch(rs485_dev)
        {
            case SENSOR_PT485:
                if(CheckPT485() != TRUE) rs485_dev  = SENSOR_DISCONNECT;
                break;
            case SENSOR_AIP:
                if(CheckAIP() != TRUE) rs485_dev  = SENSOR_DISCONNECT;
                break;
            case SENSOR_A308M:
                if(CheckA308M() != TRUE) rs485_dev  = SENSOR_DISCONNECT;
                break;
            case SENSOR_WATER_LEVEL:
                if(CheckWaterLevel() != TRUE) rs485_dev  = SENSOR_DISCONNECT;
                break;
            case SENSOR_JNC_SD:
                if(CheckJncSd() != TRUE) rs485_dev  = SENSOR_DISCONNECT;
                break;
            case SENSOR_ULTRA_SOUND:
                if(CheckUltra() != TRUE) rs485_dev  = SENSOR_DISCONNECT;
                break;            
            
            default: TraceErr1("CheckRs485Device",rs485_dev);
                rs485_dev = SENSOR_DISCONNECT;
                break;
        };
      }
     if(rs485_dev == SENSOR_DISCONNECT)  rs485_dev = ScanRs485Device();     
    pMeshNodeData->SensorClass = rs485_dev;
    
Assigned:
    TraceDec1("*** rs485_dev ***",rs485_dev);
    if(rs485_dev == SENSOR_PT485 ) pFunSensor = GetPT485Info;
    else if(rs485_dev == SENSOR_AIP ) pFunSensor = GetAipInfo;
    else if(rs485_dev == SENSOR_A308M ) pFunSensor = GetA308mInfo;
    else if(rs485_dev == SENSOR_WATER_LEVEL ) {pFunSensor = GetWaterLevelInfo; Trace(" Water Level Function Active");}
    else if(rs485_dev == SENSOR_JNC_SD ) {pFunSensor = GetJncSdInfo; Trace(" SD Function Active");}
    else if(rs485_dev == SENSOR_IAQS ) {pFunSensor = GetIaqsInfo; Trace(" IAQS Active");}
    else if(rs485_dev == SENSOR_ULTRA_SOUND ) {pFunSensor = GetUltraSoundInfo; Trace(" Ultra Sound Active");}
    else pFunSensor = GetSi7021Info; //GetBtmSensorInfo;

    
    if(pMeshNodeData->SensorClass != SENSOR_DISCONNECT) 
        {
         SetLedStatus(LED_STATUS_SERVER_TO_RS485);
         SetLedStatus(LED_STATUS_SERVER_IO_CHANGE);
         SetNodeStatus(NS_SERVER_RS485_ENABLE,ON);
        }
    else{
         SetNodeStatus(NS_SERVER_RS485_ENABLE,OFF);
        }
     
    UsartResetRxTx(USART_ID_TX_RX);
    WriteNodeData();
   return rs485_dev;
}



//
// sned modbus cmd for PT485 to get tempature value
//
bool ModbusCmdPT485()
{TraceProc();
    uchar modbus_cmd[8]={0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA}; //to get value of temperature
    bool ret_code = TRUE;
        
    UsartTxSendCmd(modbus_cmd,MODBUS_CMD_NUM);

    return ret_code;
}

//
// sned modbus cmd for PT485 to get tempature value
//
bool ModbusCmdAIP()
{TraceProc();
    uchar get_power_status_cmd[8]={0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFD, 0xCA}; // for power status
    uchar get_power_watt_cmd[8]={0x01, 0x04, 0x00, 0x04, 0x00, 0x01,0x70, 0x0B}; // for power consumption
    
    bool ret_code = TRUE;
        
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

//
//
//
bool CheckRs485Connect()
{TraceProc();
    bool ret_code = FALSE;
    uint16 loop;
    //uchar modbus_cmd[8]={0x01, 0x04, 0x00, 0x00, 0x00, 0x01, 0x31, 0xCA}; //to get device model name
    uchar modbus_cmd[8]={0x01, 0x03, 0x04, 0x0A, 0x00, 0x02, 0xE5, 0x39}; //to get device model name
    SetLed(LED_BLUE,ON); SetLed(LED_RED,OFF);
    for(loop=0; loop<40; loop++)
        {TraceDec1("CheckRs485Connect",loop);
        CHECK_RS485_CMD(modbus_cmd,8);
        if(UsartGetRxCounter() != 0) 
            {Trace("RS-485 Connect 1");
             ret_code = TRUE; break;           
            }
         Delay_ms(400); SetLedToggle(LED_BLUE); SetLedToggle(LED_RED);
        }
    if(ret_code == FALSE) TraceErr1("RS-485 Disconnect 2",UsartGetRxCounter());
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
uchar ScanRs485Device1()
{TraceProc();
    uchar ret_code = SENSOR_DISCONNECT;
    PRS485Sensor p_sensor=&Rs485Sensor[0];
    uchar loop;
    for(loop=0; loop<7; loop++)
        {
         CHECK_RS485_CMD(p_sensor->ModelCmd ,8);
         if(UsartGetRxCounter() != 0)
            {
             if(memcmp(p_sensor->ModelName,UsartGetBuff(USART_ID_RX),11) == 0)
                {Trace("PT485 Connect");ret_code = TRUE;}
             else TraceErr("PT485 Disconnect 1");
            }
        }
    
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}


//
// scan RS-485 device
//
uchar ScanRs485Device()
{TraceProc();
    uchar ret_code = SENSOR_DISCONNECT;
    
    if(CheckPT485() == TRUE) {Trace("PT485 connect");
         ret_code = SENSOR_PT485;
        }
    else if(CheckAIP() == TRUE)  {Trace("AIP connect");
         ret_code = SENSOR_AIP;
        }
    else if(CheckA308M() == TRUE) {Trace("A308M connect");
         ret_code = SENSOR_A308M;
        }
    else if(CheckWaterLevel() == TRUE) {Trace("Water Level connect");
         ret_code = SENSOR_WATER_LEVEL;
        }
    else if(CheckJncSd() == TRUE) {Trace("JNC SD connect");
         ret_code = SENSOR_JNC_SD;
        }
    else if(CheckIaqs() == TRUE) {Trace("IAQS connect");
         ret_code = SENSOR_IAQS;
        }    
    else TraceErr("ScanRs485Device device disconnect");

    return ret_code;
}




//
// Model Name = PT485
//
bool CheckPT485()
{TraceProc();
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; //to get PT485 model name
    uchar model_name[11]={0x01,0x03,0x06,0x54,0x50,0x38,0x34,0x20,0x35,0x79,0xD4};
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {//Trace("RS-485 Connect");
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),sizeof(model_name)) == 0)
            {Trace("PT485 Connect");ret_code = TRUE;}
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
{TraceProc();
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x20, 0x00, 0x03, 0x04, 0x01}; //to get AIP model name
    uchar model_name[11]={0x01,0x03,0x06,0x49,0x41,0x20,0x50,0x20,0x20,0x00,0xEA};
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {//Trace("RS-485 Connect");
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),sizeof(model_name)) == 0)
            {Trace("AIP Connect");ret_code = TRUE;}
         else TraceErr("AIP Disconnect 1");
        }
    else{ TraceErr("AIP Disconnect 2");
        }

    UsartResetRxTx(USART_ID_TX_RX);

    return ret_code;
}


//
// Check A308M connect
//
bool CheckA308M()
{TraceProc();
    bool ret_code = FALSE;
    //uchar device_name[8]={0x01, 0x03, 0x04, 0x0A, 0x00, 0x02, 0xE5, 0x39}; //to get A308 model name
    uchar device_name[8]={0x01, 0x03, 0x04, 0x05, 0x00, 0x01, 0xE5, 0x39}; //for new firmware
    uchar model_name[9]={0x01, 0x03, 0x04, 0x00, 0x00, 0x00, 0x01, 0x3B, 0xF3};
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {//Trace("A308M Connect");
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),sizeof(model_name)) == 0)
            {Trace("A308M Connect");ret_code = TRUE;}
         else TraceErr("A308M Disconnect 1");
        }
    else{ TraceErr("A308M Disconnect 2");
        }

    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}

bool CheckWaterLevel()
{TraceProc();
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; 
    uchar model_name[11]={0x01, 0x03, 0x06, 0x4C, 0x57, 0x2D, 0x53, 0x31, 0x42, 0x36, 0x69};
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),sizeof(model_name)) == 0)
            {Trace("Water Level Connect 1");ret_code = TRUE;}
         else TraceErr("Water Level Disconnect 2");
        }
    else{ TraceErr("Water Level Disconnect 3");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}





bool CheckJncSd()
{TraceProc();
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; 
    uchar model_name[11]={0x01, 0x03, 0x06, 0x53, 0x44, 0x00, 0x00, 0x00, 0x00, 0xDD, 0x19};
    
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),sizeof(model_name)) == 0)
            {Trace("JNC-SD Connect 1");ret_code = TRUE;}
         else TraceErr("JNC-SD Disconnect 2");
        }
    else{ TraceErr("JNC-SD Disconnect 3");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    return ret_code;
}

bool CheckIaqs()
{TraceProc();
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; 
    uchar model_name[11]={0x01, 0x03, 0x06, 0x41, 0x49, 0x53, 0x51, 0x32, 0x76, 0x27, 0xB9};// IAQSV2
    
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),sizeof(model_name)) == 0)
            {Trace("JNC-IAQS Connect 1");ret_code = TRUE;}
         else TraceErr("JNC-IAQS Disconnect 2");
        }
    else{ TraceErr("JNC-IAQS Disconnect 3");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    ret_code = FALSE;
    return ret_code;
}


bool CheckUltra()
{TraceProc();
    bool ret_code = FALSE;
    uchar device_name[8]={0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB}; 
    uchar model_name[11]={0x01, 0x03, 0x06, 0x41, 0x49, 0x53, 0x51, 0x32, 0x76, 0x27, 0xB9};// Ultra Sound
    
    CHECK_RS485_CMD(device_name,sizeof(device_name));
    if(UsartGetRxCounter() != 0) 
        {
         if(memcmp(model_name,UsartGetBuff(USART_ID_RX),sizeof(model_name)) == 0)
            {Trace("JNC-IAQS Connect 1");ret_code = TRUE;}
         else TraceErr("JNC-IAQS Disconnect 2");
        }
    else{ TraceErr("JNC-IAQS Disconnect 3");
        }
    UsartResetRxTx(USART_ID_TX_RX);
    ret_code = FALSE;
    return ret_code;
}


