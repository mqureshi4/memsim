#ifndef DRAM_BANK_H
#define DRAM_BANK_H
#include <vector>

#include "global_types.h"

using namespace std;

#define NUM_DRAM_BANK_ENTRIES 32

typedef struct DRAM_Bank  DRAM_Bank;
typedef struct DRAM_BankQ_Entry DRAM_BankQ_Entry;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

typedef enum DRAM_RequestType_Enum {
    DRAM_REQ_RD=0,
    DRAM_REQ_WB=1,
    DRAM_REQ_NRR=2, // RH Mitigation using NRR
    DRAM_REQ_REF=3, // Refresh
    DRAM_REQ_RFM=4, // RFM
    NUM_DRAM_REQTYPE=5
} DRAM_ReqType;

typedef enum DRAM_SchedulingPolicy_Enum {
    DRAM_SCHED_FCFS=0,
    DRAM_SCHED_FRFCFS=1,
    NUM_DRAM_SCHEDTYPE=2
} DRAM_SchedulingPolicy;

typedef enum DRAM_BankStatusType_Enum {
    DRAM_BANK_BUSY=0,
    DRAM_BANK_READY=1,
    NUM_BANK_STATUS_TYPE=2
} DRAM_BankStatusType;


typedef enum DRAM_BankAcessType_Enum {
    DRAM_ROWBUF_HIT=0,
    DRAM_ROWBUF_EMPTY=1,
    DRAM_ROWBUF_CONFLICT=2,
    NUM_DRAM_ROWBUF_OUTCOME_TYPE=3
} DRAM_BankAccessType;

typedef enum DRAM_BankQEntryStatusType_Enum {
    DRAM_BANKQ_ENTRY_INVALID=0,
    DRAM_BANKQ_ENTRY_WAIT=1,
    DRAM_BANKQ_ENTRY_INSERVICE=2,
    DRAM_BANKQ_ENTRY_DONE=3,
    NUM_DRAM_BANKQ_STATUS_TYPE=4
} DRAM_BankQEntryStatusType;

/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

struct DRAM_BankQ_Entry{
  DRAM_BankQEntryStatusType status;
  DRAM_ReqType reqtype;
  uns64 rowid; 
  Addr lineaddr; // to wake up
  uns64 birth_time; // when did it get serviced
  uns64 done_time; // if in service when will it be done
  uns64 service_delay; // time in bank
  uns64 wait_delay; // time in waiting before service
};


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


struct DRAM_Bank{
    uns   id;
    uns   channelid;
    uns   num_rows;
    DRAM_BankStatusType status; // valid, open, closed
    vector<DRAM_BankQ_Entry> bankq;

    Flag  row_valid; 
    uns64 open_row_id;
    uns64 sleep_cycle; // if bank is doing an op, busy until when
    uns64 rowbufopen_cycle; // to enforce RAS constraint
    uns64 rowbufclose_cycle; // for closed page policy and for Tonmax
    uns RAA; // RFM
  
    uns64 s_ACT;
    uns64 s_REQ;
    uns64 s_service_delay;
    uns64 s_wait_delay;
};

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DRAM_Bank*   dram_bank_new(uns bankid, uns channelid, uns num_rows);

void         dram_bank_cycle(DRAM_Bank *b);

uns          dram_bank_insert(DRAM_Bank *b,  DRAM_ReqType type,  uns64 rowid, Addr lineaddr);

void         dram_bank_remove(DRAM_Bank *b,  uns index);


uns64        dram_bank_service(DRAM_Bank *b, DRAM_ReqType type, uns64 rowid, uns64 req_birthtime); //sleeps, returns delay for done 

uns          dram_bank_schedule(DRAM_Bank *b); // pick one for service

void         dram_bank_refresh(DRAM_Bank *b, uns64 in_cycle);
void         dram_bank_rfmab(DRAM_Bank *b, uns64 in_cycle);
void         dram_bank_rfmsb(DRAM_Bank *b, uns64 in_cycle);


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


#endif 
