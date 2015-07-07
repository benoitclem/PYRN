

#ifndef MY_OS_HELPERS_H
#define MY_OS_HELPERS_H

#include "rtos.h"
//#include "rt_TypeDef.h"

/* Values for 'state'   */
#define INACTIVE        0
#define READY           1
#define RUNNING         2
#define WAIT_DLY        3
#define WAIT_ITV        4
#define WAIT_OR         5
#define WAIT_AND        6
#define WAIT_SEM        7
#define WAIT_MBX        8
#define WAIT_MUT        9

#define INACTIVE_LBL        "INACTIVE"
#define READY_LBL           "READY   "
#define RUNNING_LBL         "RUNNING "
#define WAIT_DLY_LBL        "WAIT_DLY"
#define WAIT_ITV_LBL        "WAIT_ITV"
#define WAIT_OR_LBL         "WAIT_OR "
#define WAIT_AND_LBL        "WAIT_AND"
#define WAIT_SEM_LBL        "WAIT_SEM"
#define WAIT_MBX_LBL        "WAIT_MBX"
#define WAIT_MUT_LBL        "WAIT_MUT"

typedef struct OS_XCB {
  U8     cb_type;                 /* Control Block Type                      */
  struct OS_TCB *p_lnk;           /* Link pointer for ready/sem. wait list   */
  struct OS_TCB *p_rlnk;          /* Link pointer for sem./mbx lst backwards */
  struct OS_TCB *p_dlnk;          /* Link pointer for delay list             */
  struct OS_TCB *p_blnk;          /* Link pointer for delay list backwards   */
  U16    delta_time;              /* Time until time out                     */
} *P_XCB;

void PrintThreadInfo(P_TCB ptcb);
void PrintActiveThreads(void);
void PrintRDYThreads(void);
void PrintDLYThreads(void);
const char *StateLabelForInt(uint8_t s);

#endif // MY_OS_HELPERS_H
