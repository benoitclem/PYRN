
#include "MyOsHelpers.h"


#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "MyOsHelpers.c"
#endif
#include "MyDebug.h"

#define SIZE 			128
#define MAGIC_WORD 		0xE25A2EA5

//extern void *os_active_TCB[];
#include "RTX_Conf.h"

/* List head of chained ready tasks */
extern struct OS_XCB os_rdy;
/* List head of chained delay tasks */
extern struct OS_XCB os_dly;

void PrintThreadInfo(P_TCB ptcb) {
	U32 *pStackTop = ptcb->stack+(ptcb->priv_stack/sizeof(U32));
    DBG("T(%03d) %s | %p - %p | %05d | %08x | %05d (%08x)", ptcb->task_id, StateLabelForInt(ptcb->state), pStackTop, ptcb->stack, ptcb->priv_stack, ptcb->tsk_stack,((uint32_t)pStackTop)-((uint32_t)ptcb->tsk_stack),ptcb->stack[0]);
	if((ptcb->stack[0] != MAGIC_WORD) && (ptcb->task_id != 1))
		DBG_MEMDUMP("wrong magic word",((char*)ptcb->stack)-SIZE,ptcb->priv_stack+SIZE*2);	
}
    
void PrintActiveThreads(void) {
    P_TCB ptask;
    uint16_t i = 0;
	DBG("T(TID)  STATE   |  STACKTOP  -  STACKBOT  | STKSZ | CURR-SP  | M-USE (e25a2ea5)");
    while(1) {
        ptask = (P_TCB)os_active_TCB[i];
        //DBG("%p",ptask);
		
        if(ptask == NULL)
            break;
        else
            PrintThreadInfo(ptask);
        i++;
    }
}

void PrintRDYThreads(void) {
    DBG("=== Print RDY ===");
    P_TCB ptcb = os_rdy.p_lnk;
    while(ptcb) {
        PrintThreadInfo(ptcb);
        // go next
        ptcb = ptcb->p_lnk;
    }
}

void PrintDLYThreads(void) {    
    DBG("=== Print DLY ===");
    P_TCB ptcb = os_dly.p_dlnk;
    while(ptcb) {
        PrintThreadInfo(ptcb);
        // go next
        ptcb = ptcb->p_dlnk;
    }
}

const char *StateLabelForInt(uint8_t s) {
    switch(s){
        case INACTIVE:  return INACTIVE_LBL;
        case READY:     return READY_LBL;
        case RUNNING:   return RUNNING_LBL;
        case WAIT_DLY:  return WAIT_DLY_LBL;
        case WAIT_ITV:  return WAIT_ITV_LBL;
        case WAIT_OR:   return WAIT_OR_LBL;
        case WAIT_AND:  return WAIT_AND_LBL;
        case WAIT_SEM:  return WAIT_SEM_LBL;
        case WAIT_MBX:  return WAIT_MBX_LBL;
        case WAIT_MUT:  return WAIT_MUT_LBL;
        default:        return "UNKNOWN";
    }
}
