#ifndef MCORE_H
#define MCORE_H

#include "global_types.h"
#include "memsys.h"
#include "os.h"
#include "mcache.h"
#include <zlib.h>


#define MAX_ROB_ENTRIES 1024

typedef struct ROB_Entry ROB_Entry;
typedef struct ROB  ROB;

typedef struct MCore MCore;



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


struct ROB_Entry{
  uns64 birth_time; 
  uns64 ready_time; 
  uns64 inst_num;
};


struct ROB{
  ROB_Entry entries[MAX_ROB_ENTRIES];
  uns ptr; 
  uns size;
};


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

struct MCore {
    uns   id;

    MemSys *memsys;
    MCache *l3cache;
    OS  *os;
  
    char  addr_trace_fname[1024];
    gzFile addr_trace;
    
    uns   done;

    Flag  sleep; // when memory queue full, core stalls until req can be inserted
    Addr  sleep_lineaddr; // needed for servicing sleeping core
    uns   sleep_robid; // needed for servicing sleeping core
    uns64 sleep_inst_num; // needed for servicing sleeping core

    ROB   rob;

    uns64  trace_inst_num;
    uns    trace_iaddr; // four bytes only IA
    Addr   trace_va;
    Flag   trace_wb;
    uns    trace_dhits;
    uns64  trace_inst_num_clone;//debug

    
    uns64 cycle;
    uns64 inst_num;
    uns64 access_count;
    uns64 miss_count;
    uns64 num_delay_count;// due to L3/mem access
    uns64 sum_delay_count;// due to L3/mem access
    uns64 queue_full_count; // DRAM inserts
    uns64 sleep_cycle_count; // queue full, core sleeps

    uns64  lifetime_inst_count;

    uns64 done_inst_count;
    uns64 done_cycle_count;
    uns64 done_access_count;
    uns64 done_miss_count;
    uns64 done_num_delay_count;
    uns64 done_sum_delay_count;
    uns64 done_queue_full_count;
    uns64 done_sleep_cycle_count;
};



//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

MCore *mcore_new(MemSys *memsys, OS *os, MCache *l3cache, char *addr_trace_fname, uns tid);
void   mcore_cycle(MCore *core);
void   mcore_print_stats(MCore *c);
void   mcore_print_state(MCore *c);
void   mcore_read_trace(MCore *c);
void   mcore_fread_trace(MCore *c);
void   mcore_init_trace(MCore *c);

Flag   mcore_retry_sleeping_request(MCore *c);

uns    mcore_rob_insert(MCore *c, uns64 inst_num, uns64 cycle, uns64 ready_time);
uns    mcore_rob_retire(MCore *c);
void  mcore_rob_wakeup(MCore *c, uns robid, uns64 inst_num);

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

#endif // MCORE_H
