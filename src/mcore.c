#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <zlib.h>

#include "externs.h"
#include "mcore.h"

#define MCORE_STOP_ON_EOF       0
#define DEFAULT_MEM_DELAY    500000
#define MCORE_DO_WRITEBACKS     1


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

MCore *mcore_new(MemSys *memsys, OS *os, MCache *l3cache, char *addr_trace_fname, uns id){
  MCore *c = (MCore *) calloc (1, sizeof (MCore));
  c->id    = id;
  c->memsys  = memsys;
  c->os      = os;
  c->l3cache = l3cache;

  strcpy(c->addr_trace_fname, addr_trace_fname);
  mcore_init_trace(c);
  mcore_read_trace(c);

  return c;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void mcore_init_trace(MCore *c){

  //char  command_string[256];
  //  sprintf(command_string,"gunzip -c %s", c->addr_trace_fname);

  if ((c->addr_trace = gzopen(c->addr_trace_fname, "r")) == NULL){

    //---- maybe put random sleep and try again?
    printf("Problems initializing Core: %u ... Trying again\n", (uns)c->id);
    sleep(rand()%10);
    if ((c->addr_trace = gzopen(c->addr_trace_fname, "r")) == NULL){
      die_message("Unable to open the input trace file. Dying ...\n");
    }

  }

}


/*///////////////////////////////////////////////////////////////////
For OOO core
Retire 
if trace_inst_num, get delay and insert in ROB and curr+delay

///////////////////////////////////////////////////////////////////*/

void mcore_cycle (MCore *c){

  if(MCORE_STOP_ON_EOF && c->done){
    return;
  }

  c->cycle++;

  mcore_rob_retire(c); // try to retire

  //--- handling case of memory queue full
  if(c->sleep){
    if (!mcore_retry_sleeping_request(c)){
      return; // failed, then sleep again
    }
  }


  for(uns ii=0; ii< CORE_WIDTH; ii++){
    uns delay = 0;
    Flag mem_access=FALSE;
    
    if(c->rob.size == ROB_SIZE){
      return; // if ROB full, exit ...
    }
    
    c->inst_num++;
  
    if( (!c->done) && c->inst_num > INST_LIMIT){
      mcore_read_trace(c); // break loop
    }

    Addr orig_lineaddr = os_v2p_lineaddr(c->os, c->trace_va, c->id);
    Addr wb_lineaddr   = 0;

    if(c->inst_num >= c->trace_inst_num){

      if(c->trace_wb){
	mcache_mark_dirty(c->l3cache, orig_lineaddr);
      }

      if(c->trace_wb==FALSE){
	delay += L3_LATENCY; // incurred on both hit and miss
	c->access_count++;
	Flag l3outcome = mcache_access(c->l3cache, orig_lineaddr);

	if( (L3_PERFECT==FALSE) && (l3outcome==MISS)){
	  delay += DEFAULT_MEM_DELAY;
	  mem_access=TRUE;
	  mcache_install(c->l3cache,orig_lineaddr);

	  if(MCORE_DO_WRITEBACKS && c->l3cache->evicted_dirty_line){
	    wb_lineaddr =  c->l3cache->evicted_line_addr;
	  }
       
	  c->miss_count++;
	}
      }
     
      mcore_read_trace(c);
    }

    //--- insert entry into rob
    uns myrobid = mcore_rob_insert(c, c->inst_num, c->cycle, c->cycle+delay);
    if(mem_access){

      if(c->id == DEBUG_CORE_ID){
	printf("CORE-RDSEND for CoreID: %u ROBID: %u InstNum: %llu Cycle: %llu\n", c->id, myrobid, c->inst_num, c->cycle);
      }
       
      if(memsys_access(c->memsys, orig_lineaddr, c->id, myrobid, c->inst_num, wb_lineaddr)==FAIL){

	DBGMSGC(c->id, "SLEEPING: Core: %u ROBID: %u InstNum: %llu Cycle: %llu\n",
		      c->id, myrobid,  c->inst_num, c->cycle);
	
	c->queue_full_count++;
	c->sleep_lineaddr = orig_lineaddr;
	c->sleep_robid    = myrobid;
	c->sleep_inst_num = c->inst_num;
	c->sleep = TRUE;
	return; // exit the cycle
      }

    }
       
  }

}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void mcore_read_trace (MCore *c){

    mcore_fread_trace(c);

    if(gzeof(c->addr_trace) || ((!c->done) && (c->inst_num >= INST_LIMIT)) ){


      if(!c->done){
	//printf("\nCoreID: %u is done with %u INST...\n", c->id, (uns)INST_LIMIT);
	c->done_inst_count  = c->inst_num;
	c->done_cycle_count = c->cycle;
	c->done_access_count= c->access_count;
	c->done_miss_count  = c->miss_count;
	c->done_num_delay_count = c->num_delay_count;
	c->done_sum_delay_count = c->sum_delay_count;
	c->done_queue_full_count = c->queue_full_count;
	c->done_sleep_cycle_count = c->sleep_cycle_count;
	c->done = 1;
      }
    
      if(!MCORE_STOP_ON_EOF){
	gzclose(c->addr_trace);
	mcore_init_trace(c);
	mcore_fread_trace(c);
	c->lifetime_inst_count += c->inst_num;
	c->inst_num = 0;
      }
    }
  
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void mcore_print_stats(MCore *c){
  
  double ipc = (double)(c->done_inst_count)/(double)(c->done_cycle_count);
  double apki = 1000 * (double)c->done_access_count/(double)c->done_inst_count;
  double mpki = 1000 * (double)c->done_miss_count/(double)c->done_inst_count;
  double missrate = 100*mpki/apki;
  double avgdelay =  (double)c->done_sum_delay_count/(double)c->done_num_delay_count;
  char header[256];
  sprintf(header, "CORE_%02d", c->id);
  
  printf("\n%s_ID           \t : %2u",   header,  c->id);
  printf("\n%s_TRACE        \t : %s",    header,  c->addr_trace_fname);
  printf("\n%s_INST         \t : %llu",  header,  c->done_inst_count);
  printf("\n%s_CYCLES       \t : %llu",  header,  c->done_cycle_count);
  printf("\n%s_ACCESS       \t : %llu",  header,  c->done_access_count);
  printf("\n%s_MISS         \t : %llu",  header,  c->done_miss_count);
  printf("\n%s_QFULL        \t : %llu",  header,  c->done_queue_full_count);
  printf("\n%s_APKI         \t : %4.2f", header,  apki);
  printf("\n%s_MPKI         \t : %4.2f", header,  mpki);
  printf("\n%s_MISSRATE     \t : %4.2f", header,  missrate);
  //printf("\n%s_SUM_DELAY    \t : %llu",  header,  c->done_sum_delay_count);
  //printf("\n%s_NUM_DELAY    \t : %llu",  header,  c->done_num_delay_count);
  printf("\n%s_AVGDELAY     \t : %4.2f", header,  avgdelay);
  printf("\n%s_SLEEP_CYCLES \t : %llu",  header,  c->done_sleep_cycle_count);
  printf("\n%s_IPC          \t : %4.3f", header,  ipc);
  
  printf("\n");

  gzclose(c->addr_trace);
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void mcore_fread_trace (MCore *c){
  
    c->trace_inst_num=0;
    c->trace_va=0;
    c->trace_wb=0;

    gzread ( c->addr_trace, &c->trace_inst_num, 5);
    gzread ( c->addr_trace, &c->trace_wb, 1);
    gzread ( c->addr_trace, &c->trace_va, 4);

}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void mcore_print_state(MCore *c){
  char header[256];
  sprintf(header, "CORE_%02d (INST: %llu)", c->id, c->inst_num);
  printf("\n%s_ID           \t : %2u",   header,  c->id);
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns   mcore_rob_insert(MCore *c, uns64 inst_num, uns64 cycle, uns64 ready_time){
  assert(c->rob.size < ROB_SIZE);
  uns index = (c->rob.ptr + c->rob.size)%ROB_SIZE;
  
  c->rob.entries[index].birth_time = cycle;
  c->rob.entries[index].ready_time = ready_time;
  c->rob.entries[index].inst_num = inst_num;

  c->rob.size++;

 
  if( (c->id == DEBUG_CORE_ID) && (ready_time - cycle > 10) ){
    printf("ROB-INSERT: Inserting InstNum: %llu Cycle: %llu ReadyTime: %llu\n", inst_num, cycle, ready_time);
  }

  return index; 
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


uns   mcore_rob_retire(MCore *c){
  uns done=0;

  if(c->rob.size == 0){
    return 0;
  }

  for(uns ii=0; ii<CORE_WIDTH; ii++){
    if( c->rob.entries[c->rob.ptr].ready_time <= c->cycle){
      uns64 delta = c->rob.entries[c->rob.ptr].ready_time  - c->rob.entries[c->rob.ptr].birth_time;

      if(delta >=  DEFAULT_MEM_DELAY){
	 DIEMSG("LOST-PACKET: Core: %u ROBID: %u InstNum: %llu Cycle: %llu\n",
		      c->id, c->rob.ptr,  c->rob.entries[c->rob.ptr].inst_num, c->cycle);
      }
      
      c->rob.ptr = (c->rob.ptr+1)%ROB_SIZE;
      c->rob.size--;
      done++;
    }else{
      return done;
    }
  }

  return done;
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void  mcore_rob_wakeup(MCore *c,  uns robid, uns64 inst_num){

  if(c->rob.entries[robid].inst_num == inst_num){
    uns64 delta = c->cycle - c->rob.entries[robid].birth_time;
    //printf("Delta: %llu Num: %llu Sum: %llu\n", delta, c->num_delay_count, c->sum_delay_count);
    c->rob.entries[robid].ready_time = c->cycle;
    c->num_delay_count++;
    c->sum_delay_count+=delta;
  }else{
    DIEMSG("ZOMBIEWAKEUP: Core: %u InstNum: %llu RobID: %u StoredInstNum: %llu Cycle: %llu\n",
	   c->id, inst_num, robid, c->rob.entries[robid].inst_num, c->cycle);
  }
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

Flag  mcore_retry_sleeping_request(MCore *c){
  c->sleep_cycle_count++;
	
  if(memsys_access(c->memsys, c->sleep_lineaddr, c->id, c->sleep_robid, c->sleep_inst_num,0)){
    c->rob.entries[c->sleep_robid].birth_time = c->cycle;
    c->rob.entries[c->sleep_robid].ready_time = c->cycle + DEFAULT_MEM_DELAY;
    c->rob.entries[c->sleep_robid].inst_num = c->sleep_inst_num;
    c->sleep=FALSE;

    if(c->id == DEBUG_CORE_ID){
    printf("ROB-INSERT2: Inserting InstNum: %llu Cycle: %llu ReadyTime: %llu\n",
	   c->sleep_inst_num, c->cycle, c->rob.entries[c->sleep_robid].ready_time);
    }

    return TRUE;
  }
  return FALSE;
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
