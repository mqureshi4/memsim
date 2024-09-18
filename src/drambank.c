#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <algorithm> // Make sure this is included
#include <vector> // Include this if you're using vectors
#include <iterator> // Include this if you're using iterators

#include "mcore.h"
#include "externs.h"
#include "drambank.h"
#include "dramchannel.h"
#include "dram.h"
#include "memsys.h"



#define BANK_VERBOSE 1


extern MemSys *memsys;

extern uns64 cycle;

uns64 DRAM_RANDOMIZE_PRAC_ON_INIT=0;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DRAM_Bank*   dram_bank_new(uns bankid, uns channelid, uns num_rows){
  DRAM_Bank *b = new DRAM_Bank; 
  b->id        = bankid;
  b->channelid = channelid;
  b->status    = DRAM_BANK_READY;
  b->num_rows  = num_rows;
  b->row_valid = FALSE; 
  b->open_row_id = 0;
  b->sleep_cycle = 0; 
  b->rowbufopen_cycle = 0; 
  b->rowbufclose_cycle = 0;; // for closed page policy and for Tonmax
  b->RAA=0; // RFM
    
  if(MEM_CLOSEPAGE){
    DRAM_MAX_TOPEN=tRAS; // closed page policy closes page after t_RAS
  }

  b->s_ACT=0;
  b->s_REQ=0;
  b->s_service_delay=0;
  b->s_wait_delay=0;

  return b;
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void  dram_bank_cycle(DRAM_Bank *b){

  // if bank is busy check if it is free 
  if(b->status == DRAM_BANK_BUSY){
    if(cycle > b->sleep_cycle){
      b->status = DRAM_BANK_READY;
    }else{
      return;
    }
  }

  // if queue is empty go away
  if(b->bankq.size() == 0){
    return;
  }

  // if the bank is waiting for RDWR token, go away
  DRAM_Channel *mychannel = memsys->mainmem->channel[b->channelid];
  if(mychannel->dbusq.entries[b->id].valid){
    return;
  }

  
  // call schedule to pick an entry, call service -- take time, change q-entry status
  uns index = dram_bank_schedule(b);

  assert(b->bankq[index].status == DRAM_BANKQ_ENTRY_WAIT);

  
  uns64 bank_delay = dram_bank_service(b,  b->bankq[index].reqtype, b->bankq[index].rowid,  b->bankq[index].birth_time );

  b->bankq[index].status = DRAM_BANKQ_ENTRY_INSERVICE;
  uns64 tot_delay = bank_delay; 
  b->bankq[index].done_time = cycle + tot_delay;
  b->bankq[index].service_delay =  tot_delay;
  uns64 q_wait_time = cycle -  b->bankq[index].birth_time;
  b->bankq[index].wait_delay =  q_wait_time;

  //--- bank work done, bus schedule pushed to channel--
  b->bankq[index].status = DRAM_BANKQ_ENTRY_DONE;
  
  if( (b->bankq[index].reqtype == DRAM_REQ_RD)||(b->bankq[index].reqtype == DRAM_REQ_WB)){
    Addr mylineaddr = b->bankq[index].lineaddr;
    uns64 myreadytime = b->bankq[index].done_time;
    DRAM_ReqType myreqtype = b->bankq[index].reqtype;
    
    dram_channel_insert_rdwrq(mychannel, b->id,  myreqtype, mylineaddr, myreadytime); // RD-WR Q
    b->s_service_delay += bank_delay ;
    b->s_wait_delay += b->bankq[index].wait_delay ;
  }
    
  //remove
  dram_bank_remove(b, index);

}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns   dram_bank_insert(DRAM_Bank *b,  DRAM_ReqType type, uns64 rowid, Addr lineaddr){
  assert(rowid < b->num_rows);
 
  if(b->bankq.size() == NUM_DRAM_BANK_ENTRIES){
    return 0;
  }

  if( (type==DRAM_REQ_RD)|| (type==DRAM_REQ_WB)){
    b->s_REQ++;
  }

  DRAM_BankQ_Entry entry;
  entry.status = DRAM_BANKQ_ENTRY_WAIT;
  entry.reqtype = type;
  entry.rowid = rowid;
  entry.lineaddr = lineaddr;
  entry.birth_time = cycle;

  b->bankq.push_back(entry);

  return 1; // success
}

///////////////////////////////////////////////////////////
// if qentry is done, remove from q, 
////////////////////////////////////////////////////////////

void         dram_bank_remove(DRAM_Bank *b,  uns index){
  assert( b->bankq.size() > 0);
  assert( b->bankq[index].status == DRAM_BANKQ_ENTRY_DONE);
  b->bankq.erase(b->bankq.begin()+index);
}

////////////////////////////////////////////////////////////
// sleeps, returns delay for done 
////////////////////////////////////////////////////////////

uns64   dram_bank_service(DRAM_Bank *b,  DRAM_ReqType type, uns64 rowid, uns64 req_birthtime){
  uns64 retval=0;
  Flag new_act=FALSE; // did we do a new activation?

  assert(b->status != DRAM_BANK_BUSY);

  
  // if type == N, figure out the delay, sleep time, rowbuffer staus
  if( (type == DRAM_REQ_RD) || (type == DRAM_REQ_WB)){
    uns64 act_delay=0;

    //check for early page closure
    if( b->row_valid){
      if(cycle  >= b->rowbufclose_cycle){
	b->row_valid = FALSE;
	uns64 delta = cycle - b->rowbufclose_cycle;
	if(delta < tPRE){
	  act_delay += (tPRE-delta);
	}
      }
    }

    // empty
    if(b->row_valid == FALSE){
      b->rowbufopen_cycle = cycle + act_delay;
      act_delay+=tACT;
      b->rowbufclose_cycle = b->rowbufopen_cycle+DRAM_MAX_TOPEN; // page closure
      b->row_valid = TRUE;
      b->open_row_id = rowid;
      new_act = TRUE;
    }else{
      act_delay=0; // hit

      // conflict
      if(b->open_row_id != rowid){
	uns64 ras_delay=0, pre_delay=tPRE;
	uns64 delta = cycle - b->rowbufopen_cycle;
	if(delta < tRAS){
	  ras_delay = tRAS-delta;
	}
	b->rowbufopen_cycle = cycle + ras_delay + tPRE ;
	act_delay = (ras_delay + pre_delay +tACT);
	b->rowbufclose_cycle = b->rowbufopen_cycle+DRAM_MAX_TOPEN; // page closure
	b->row_valid = TRUE;
	b->open_row_id = rowid;
	new_act=TRUE;
      }
    }

    
    retval = act_delay; 
    b->status = DRAM_BANK_BUSY;
    b->sleep_cycle = cycle + act_delay + tRDRD; 
  }



  //---- if ACT done, update states, RFM -------
  if(new_act){
    b->s_ACT++; // counted only for RD and WR
    
    b->RAA++;

    //--- RFM scheduling as at 3 times RFMTH (equivalent for four entry DMQ)
    if( (DRAM_RFM_POLICY>0) && (b->RAA>=RFM_RAAMMT)){

      if(DRAM_RFM_POLICY == DRAM_RFM_AB){
	memsys->mainmem->channel[b->channelid]->RFMAB = TRUE;
      }
      
      if(DRAM_RFM_POLICY == DRAM_RFM_SB){
	uns num_bg = memsys->mainmem->channel[b->channelid]->banks_in_bankgroup;
	uns my_offset = b->id / num_bg;
	memsys->mainmem->channel[b->channelid]->RFMSB |= (1<< my_offset);
      }
    }

  
  }


  return retval;
}



////////////////////////////////////////////////////////////
//  return the index of the bankq entry selected for schedule
////////////////////////////////////////////////////////////

uns         dram_bank_schedule(DRAM_Bank *b){
  assert(b->bankq.size());

  if(DRAM_SCHED_POLICY == DRAM_SCHED_FCFS){
    return 0;
  }

  if(DRAM_SCHED_POLICY == DRAM_SCHED_FRFCFS){
    if(b->row_valid){
      for(uns ii=0; ii< b->bankq.size(); ii++){
	if( (b->bankq[ii].reqtype == DRAM_REQ_RD) &&
	    (b->bankq[ii].rowid == b->open_row_id)){
	  return ii;
	}
      }
    }
    return 0; 
  }

  assert(0);
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void  dram_bank_refresh(DRAM_Bank *b, uns64 in_cycle){
    b->status = DRAM_BANK_BUSY;
    b->sleep_cycle = in_cycle + tRFC;
    b->row_valid = FALSE;

    if(b->RAA > RFM_REFTH){
      b->RAA -= RFM_REFTH;
    }else{
      b->RAA = 0;
    }

}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void  dram_bank_rfmsb(DRAM_Bank *b, uns64 in_cycle){
  dram_bank_rfmab(b, in_cycle); // same behavior
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


void  dram_bank_rfmab(DRAM_Bank *b, uns64 in_cycle){
  b->status = DRAM_BANK_BUSY;
  b->sleep_cycle = in_cycle + tPRE + tRFM;
  b->row_valid = FALSE;

  if(b->RAA > RFM_TH){
    b->RAA -= RFM_TH;
  }else{
    b->RAA = 0;
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
