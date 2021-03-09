/*
 * Global.h
 *  Created on: 2019/09/19
 *  Author: richard.huang
 */


#ifndef _MEAH_FEATURES_H_
#define _MEAH_FEATURES_H_


//#define FRIEND_NODE
#define LPN_NODE


#define LPN_POLL_TIMEOUT    (10*1000)  //SLEEPING_TIMER


Result NodeFriend(uint8 status);
Result NodeProxy(uint8 status);
Result  NodeRelay(uint8 status);
Result NodeLpn(uint8 status);
Result NodeBeacon(uint8 status);
Result NodeRelay(uint8 status);





uint32 EvtMeshFriendProc(PCmdPacket pEvent);


//
//
Result NodeSleeping(void);
Result NodeWakeUp(void);




#endif

