/*
 *
 *  Created on: 2020/02/10
 *  Author: Richard
 */

#ifndef _IVI_FEATURES_
#define _IVI_FEATURES_

#define IVI_UPDATE_ON   1

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/
/******************************************************************
 * Added by Kevin
 * ***************************************************************/
//CORE_DECLARE_IRQ_STATE;
#define ASSERT(x) if (!(x)) { CORE_ENTER_ATOMIC(); while (1); }

#define IV_RECOVERY_MODE  1 //1: client, 0:server

#define SNB_STATE       1
#define IV_TEST_MODE    1

#define TEST_IV_HOP     30
#define MAX_IV_HOP      42

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#define IDLE            0
#define ONGOING         1
#define PRIMARY_ELEM    0

typedef enum {
  normal_update,
  test_update,
  test_end
}test_p_t;

#define ELEMENT_SEQNUM_MAX      0x01000000
#define IVI_INC_MIN             MIN(TEST_IV_HOP, MAX_IV_HOP - 1)

#define TIMER_IVI_DETECT        (TIMER_10SEC/TIMER_DEVICE_TASK) //60 sec

#define IVI_SEQ_WAITING         1
#define IVI_DETECT_SEQ          2
#define IVI_UPDATE_ENABLE       3
#define IVI_UPDATE_WAITING      4
#define IVI_UPDATE_ENDING       5


#define IVI_UPDATE_INIT         0
#define IVI_DETECT              1
#define IVI_UPDATE_WAIT         2
#define IVI_UPDATE_ACTION       3
#define IVI_ACTION_WAIT         4
#define IVI_UPDATE_END          5




void IviInit();
bool IviUpdate(uchar status);
uint32 EvtMeshIviProc(PCmdPacket pEvent);
void iv_config(uint8_t iv_test_mode,uint8_t iv_recovery_mode,uint8_t snb_state);
void iv_update_normal(void);
void iv_update_hop(void);
void ShowCurrRemSeq(void);
uint32 EvtMeshIviClientProc(PCmdPacket pEvent);
uint32 EvtMeshIviServerProc(PCmdPacket pEvent);
Bool IvIndexUpdate(uchar status);
bool MeshCheckSeqNum();
void IviUpdateStatus(uchar status);

void ClientIviUpdateProc();
void NodeIviUpdateProc();




#endif //_IVI_FEATURES_

