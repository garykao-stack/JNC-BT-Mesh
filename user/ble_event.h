/*
 * ble_event.h
 *
 *  Created on: 2019/07/04
 *      Author: Richard
 */

#ifndef _BLE_EVENT_
#define _BLE_EVENT_

#include "buttons.h"

#define DATA_SIZE			255					// Size of the arrays for sending and receiving data

#define DATA_TRANSFER_SIZE_INDICATIONS		0 // If == 0 or > MTU-3 then it will send MTU-3 bytes of data, otherwise it will use this value
#define DATA_TRANSFER_SIZE_NOTIFICATIONS	0 // If == 0 or > MTU-3 then it will calculate the data amount to send for maximum over-the-air packet usage, otherwise it will use this value

#define PHY_1M				(0x01)
#define PHY_2M				(0x02)
#define PHY_S8				(0x04)
#define PHY_S2				(0x08)

#define TX_POWER			0

//#define USE_LED_FOR_CONNECTION_SIGNALING		// Define this so that LED0 is ON when connection is established and OFF when it's disconnected
//#define USE_LED_FOR_DATA_SENDING_SIGNALING 	// Define this so that LED1 is ON when data is being send

/* SLAVE SIDE MACROS */
#define NOTIFICATIONS_START				(uint32)(1 << 0)  	// Bit flag to external signal command
#define NOTIFICATIONS_END				(uint32)(1 << 1)	// Bit flag to external signal command
#define INDICATIONS_START				(uint32)(1 << 2)	// Bit flag to external signal command
#define INDICATIONS_END					(uint32)(1 << 4)	// Bit flag to external signal command
#define ADV_INTERVAL_MAX				160					// 160 * 0.625us = 100ms
#define ADV_INTERVAL_MIN				160					// 160 * 0.625us = 100ms
//#define SEND_FIXED_TRANSFER_COUNT		1000				// Uncomment this if you want to send a fixed amount of indications/notifications on each button press
//#define SEND_FIXED_TRANSFER_TIME		(32768*5)				// Uncomment this if you want to send indications/notifications for a fixed amount of time (in 32.768Hz clock ticks) on each button press

/* MASTER SIDE MACROS */
#define PHY_CHANGE						(uint32)(1 << 8)	// Bit flag to external signal command


#define CONN_INTERVAL_125KPHY_MAX		160					// 160 * 1.25ms = 200ms
#define CONN_INTERVAL_125KPHY_MIN		160					// 160 * 1.25ms = 200ms
#define SLAVE_LATENCY_125KPHY			0					// How many connection intervals can the slave skip if no data is to be sent
#define SUPERVISION_TIMEOUT_125KPHY		50					// 50 * 10ms = 500ms

//Richard Add
#define CONN_INTERVAL_500KPHY_MAX		100					// 160 * 1.25ms = 200ms
#define CONN_INTERVAL_500KPHY_MIN		100					// 160 * 1.25ms = 200ms
#define SLAVE_LATENCY_500KPHY			0					// How many connection intervals can the slave skip if no data is to be sent
#define SUPERVISION_TIMEOUT_500KPHY		50					// 50 * 10ms = 500ms
//Richard Add

#define CONN_INTERVAL_1MPHY_MAX			40					// 40 * 1.25ms = 50ms
#define CONN_INTERVAL_1MPHY_MIN			40					// 40 * 1.25ms = 50ms
#define SLAVE_LATENCY_1MPHY				0					// How many connection intervals can the slave skip if no data is to be sent
#define SUPERVISION_TIMEOUT_1MPHY		100					// 100 * 10ms = 1000ms

#define CONN_INTERVAL_2MPHY_MAX			20  				// 20 * 1.25ms = 25ms
#define CONN_INTERVAL_2MPHY_MIN			20					// 20 * 1.25ms = 25ms
#define SLAVE_LATENCY_2MPHY				0					// How many connection intervals can the slave skip if no data is to be sent
#define SUPERVISION_TIMEOUT_2MPHY		100					// 100 * 10ms = 1000ms

#define SCAN_INTERVAL					16					// 16 * 0.625 = 10ms
#define SCAN_WINDOW						16					// 16 * 0.625 = 10ms
#define ACTIVE_SCANNING					1					// 1 = active scanning (sends scan requests), 0 = passive scanning (doesn't send scan requests)
/* -------------------- */


#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4 //8 //richard modify
#endif

/*
 * Maximum number of Bluetooth advertisement sets.
 * 1 is allocated for Bluetooth LE stack
 * 1 one for Bluetooth mesh stack
 * 1 needs to be allocated for each Bluetooth mesh network
 *   - Currently up to 4 networks are supported at a time
 */
//#define MAX_ADVERTISERS (2 + 4) //richard disable

#ifndef MAX_ADVERTISERS
#define MAX_ADVERTISERS (2+4)
#endif

////
#define     ID_GENERIC_ACCESS       0x1800
#define     ID_DEVICE_NANE          0x2A00
#define     ID_APPERANCE            0x2A01
////
#define     ID_DEVICE_INFO          0x180A  //for device information
#define     ID_FACTORY_NAME_STR     0x2A29
#define     ID_MODEL_NUM_STR        0x2A24
#define     ID_SYSTEM_ID            0x2A23




#define UUID_SIG                0
#define UUID_USER               1

#define MAC_ADDRESS_SIZE        6
#define BLE_NAME_SIZE           32
#define CONNECT_NUM_MAX         2 //MAX_CONNECTIONS
#define SERVICE_NUM_MAX         4
#define CHARA_NUM_MAX           4
#define USER_UUID_SIZE_MAX      16
#define All_BLE_NODE_NUM            (CONNECT_NUM_MAX+1)


#define NODE_MASTER             1   // index for array
#define NODE_SLAVE              0   // BLE boot default behavior
#define BLE_SLAVE_NAME          "my_slave"
#define BLE_NAME_LEN            32


#define ADVERT_TYPE_PARTIAL_UUID16   0x02    //andle partial ($02) list of 16-bit UUIDs
#define ADVERT_TYPE_COMPLETE_UUID16  0x03    //andle partial ($02) list of 16-bit UUIDs
#define ADVERT_TYPE_MORE_UUID128     0x06     //More 128-bit UUIDs available
#define ADVERT_TYPE_UUID128          0x07     //compare UUID to our custom service UUID
#define ADVERT_TYPE_LOCAL_NAME       0x09     //complete local name

typedef struct _ScanRespDataPacket_
{
    uchar   adv_length;
    uchar   adv_type;
    uchar   pData[];
}ScanRespDataPacket,*PScanRespDataPacket;


typedef struct _BleCmdHeader_
{
    uchar BleGroup;
    uchar DataLength;
    uchar BleClass;     
    uchar BleSpecEvent; // BLE Special Event  
}BleCmdHeader,PBleCmdHeader;



#define PRINT_ADV_INFO                  0

// BLE status
#define BLE_NODE_NO_HANDLE              0xFF        //for uint8
#define NO_HANDLE                       0xFFFF      //for uint16
typedef enum _NodeMsg_
{
    MSG_INIT,  // for initial status
    MSG_SCANNING,  MSG_DISCOVER, MSG_DISCONNECTED,MSG_OPEN,
    MSG_CONNECTED, 
    MSG_DISCOVERING_SERVICE, MSG_DISCOVERING_ALL_SERVICE_COMPLETE,
    MSG_DISCOVERING_CHARA, MSG_DISCOVERING_ALL_CHARA_COMPLETE,
    MSG_DISCOVERING_DESCR, MSG_DISCOVERING_DESCR_COMPLETE,
    MSG_DISCOVERING_OK,
    MSG_DISCOVERING_SERVICE_BY_UUID,
    MSG_DISCOVERING_CHARA_BY_UUID,
    MSG_COMPLETE,
    MSG_READY,
    
    MSG_ENABLING_NOTIFY, MSG_NOTIFY_ENABLED,
    // for SM Status
    MSG_SM_INIT, MSG_SM_STATUS1, MSG_SM_STATUS2, MSG_SM_STATUS3, MSG_SM_STATUS4, MSG_SM_STATUS5, 
    
}NodeMsg;

// for Timer Handler
//
//
typedef enum _TimerID_
{
    TIMER_FACTORY_RESET=0x10,
    TIMER_RESTART,
    TIMER_PROVISIONING,
    TIMER_SAVE_STATE,
    TIMER_DELAYED_ONOFF,
    TIMER_DELAYED_LIGHTNESS,
    TIMER_ONOFF_TRANSITION,
    TIMER_LIGHTNESS_TRANSITION,
    TIMER_DUMP
}TimerID;


// Richard: define
#define EVENT_ID_NULL       0
#define TIMER_HANDLE_END    -1



typedef struct _MESSAGE_MAPPING_
{
    uint32 MsgInput,MsgSystem;
}MessageMapping,*PMessageMapping;



typedef enum _TimerHandle_
{
    TIMER_TEMP,     // Report temperature to host
    TIMER_HUM,      // Report humidity
    TIMER_TEMP_HUM, // Report temperature and humidity
    TIMER_ALERT,
}TimerHandle;


#define     TIMER_BUFF_SIZE     20

typedef struct _TimerHandleProc_
{
    uint32  Handle;
    uint32  RepeatTimer;    // xx ms
    Bool    (*pHandleProc)(PCmdPacket pEvent);
}TimerHandleProc,*PTimerHandleProc;


#define UI_TIMER_TEMP       TIMER_500MS      //Temperature measurement timer
#define UI_TIMER_ADVERT     TIMER_500MS      //Advertisement Timer
#define UI_TIMER_KEYPAD     TIMER_500MS      
#define UI_TIMER_LED        TIMER_500MS
#define UI_TIMER_ALTER      TIMER_500MS
#define UI_TIMER_POL_INV    NULL



#define BLE_NODE_FACTORY_RESET      8


extern uint8 ConnectHandle;         // handle of the last opened LE connection
//extern Result  result;


/*******************************************************************************
 * Function prototypes.
 ******************************************************************************/
void BleEventInit();
bool BleEventProc(PCmdPacket pEvent);
void initiate_factory_reset(void);
void set_device_name(bd_addr *pAddr);


//BLE Event Procedure
uint32 EvtSystemBootProc(PCmdPacket pEvent);
uint32 EvtSysAwakeProc(PCmdPacket pEvent);

uint32 EvtBleConnectionProc(PCmdPacket pEvent);
uint32 EvtConnectionPhyStatusProc(PCmdPacket pEvent);

uint32  EvtGapScanResponseProc(PCmdPacket pEvent);
uint32  EvtGapAdvTimeOutProc(PCmdPacket pEvent);

//****************** for GATT *************************************
uint32  EvtGattServerAttributeValueProc(PCmdPacket pEvent);
uint32  EvtGattServiceProc(PCmdPacket pEvent);
uint32  EvtGattCharaProc(PCmdPacket pEvent);
uint32  EvtGattCharaValueProc(PCmdPacket pEvent);
uint32  EvtGattMtuExchangedProc(PCmdPacket pEvent);
uint32  EvtGattCompletedProc(PCmdPacket pEvent);
uint32  EvtGattServerCharactStatusProc(PCmdPacket pEvent);
uint32  EvtGattServerUserReadRequest(PCmdPacket pEvent);
uint32  EvtGattServerUserWriteRequestProc(PCmdPacket pEvent); 

//****************** for SM *************************************
uint32  EvtSmPassKeyDisplayProc(PCmdPacket pEvent);
uint32  EvtSmPassKeyRequestProc(PCmdPacket pEvent);
uint32  EvtSmPassConfirmPasskeyProc(PCmdPacket pEvent);
uint32  EvtSmBoundedProc(PCmdPacket pEvent);
uint32  EvtSmBoundingFailedProc(PCmdPacket pEvent);
uint32  EvtSmListBoundingEntryProc(PCmdPacket pEvent);
uint32  EvtSmListAllBoundingsCompleteProc(PCmdPacket pEvent);
uint32  EvtSmConfirmBoundingProc(PCmdPacket pEvent);

////////////////////////////////////////////////////////////////////////
void enter_to_dfu_ota(uint8_t connection);
void SetAdvertise(uchar status);



#endif

