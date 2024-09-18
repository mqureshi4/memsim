#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mcache.h"


#define MCACHE_SRRIP_MAX  7
#define MCACHE_SRRIP_INIT 1

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

MCache *
mcache_new(uns sets, uns assocs, uns repl_policy ){
  MCache *c = (MCache *) calloc (1, sizeof (MCache));
  c->sets    = sets;
  c->assocs  = assocs;
  c->repl_policy = (MCache_ReplPolicy)repl_policy;

  c->entries  = (MCache_Entry *) calloc (sets * assocs, sizeof(MCache_Entry));

  return c;
}




////////////////////////////////////////////////////////////////////
// the addr field is the lineaddress = address/cache_line_size
////////////////////////////////////////////////////////////////////

Flag mcache_access (MCache *c, Addr addr){
  Addr  tag  = addr; // full tags
  uns   set  = mcache_get_index(c,addr);
  uns   start = set * c->assocs;
  uns   end   = start + c->assocs;
  uns   ii;
    
  c->s_count++;
    
  for (ii=start; ii<end; ii++){
    MCache_Entry *entry = &c->entries[ii];
    
    if(entry->valid && (entry->tag == tag))
      {
	entry->last_access  = c->s_count;
	entry->ripctr       = MCACHE_SRRIP_MAX;
	c->touched_wayid = (ii-start);
	c->touched_setid = set;
	c->touched_lineid = ii;
	return HIT;
      }
  }

  //even on a miss, we need to know which set was accessed
  c->touched_wayid = 0;
  c->touched_setid = set;
  c->touched_lineid = start;

  c->s_miss++;
  return MISS;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

Flag    mcache_probe    (MCache *c, Addr addr){
  Addr  tag  = addr; // full tags
  uns   set  = mcache_get_index(c,addr);
  uns   start = set * c->assocs;
  uns   end   = start + c->assocs;
  uns   ii;

  for (ii=start; ii<end; ii++){
    MCache_Entry *entry = &c->entries[ii];
    if(entry->valid && (entry->tag == tag))
      {
	return TRUE;
      }
  }
  
  return FALSE;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

Flag    mcache_invalidate    (MCache *c, Addr addr){
  Addr  tag  = addr; // full tags
  uns   set  = mcache_get_index(c,addr);
  uns   start = set * c->assocs;
  uns   end   = start + c->assocs;
  uns   ii;

  for (ii=start; ii<end; ii++){
    MCache_Entry *entry = &c->entries[ii];
    if(entry->valid && (entry->tag == tag))
      {
	entry->valid = FALSE;
	return TRUE;
      }
  }
  
  return FALSE;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

Flag    mcache_mark_dirty    (MCache *c, Addr addr){
  Addr  tag  = addr; // full tags
  uns   set  = mcache_get_index(c,addr);
  uns   start = set * c->assocs;
  uns   end   = start + c->assocs;
  uns   ii;

  for (ii=start; ii<end; ii++){
    MCache_Entry *entry = &c->entries[ii];
    if(entry->valid && (entry->tag == tag))
      {
	entry->dirty = TRUE;
	return TRUE;
      }
  }
  
  return FALSE;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


void mcache_install (MCache *c, Addr addr){
  Addr  tag  = addr; // full tags
  uns   set  = mcache_get_index(c,addr);
  uns   start = set * c->assocs;
  uns   end   = start + c->assocs;
  uns   ii, victim;
  
  Flag update_lrubits=TRUE;

  c->evicted_dirty_line = FALSE;
  
  MCache_Entry *entry;

  for (ii=start; ii<end; ii++){
    entry = &c->entries[ii];
    if(entry->valid && (entry->tag == tag)){
      printf("Installed entry already with addr:%llx present in set:%u\n", addr, set);
      exit(-1);
    }
  }
  
  // find victim and install entry
  victim = mcache_find_victim(c, set);
  entry = &c->entries[victim];

  if(entry->valid){
    c->s_evict++;
    if(entry->dirty){
      c->evicted_dirty_line = TRUE;
      c->evicted_line_addr = entry->tag;
    }
  }

 //udpate DRRIP info and select value of ripctr
  uns ripctr_val=MCACHE_SRRIP_INIT;

  
  //put new information in
  entry->tag   = tag;
  entry->valid = TRUE;
  entry->dirty = FALSE;
  entry->ripctr  = ripctr_val;
  
  if(update_lrubits){
    entry->last_access  = c->s_count;   
  }

  c->touched_lineid=victim;
  c->touched_setid=set;
  c->touched_wayid=victim-(set*c->assocs);
}





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns mcache_find_victim (MCache *c, uns set)
{
  int ii;
  int start = set   * c->assocs;    
  int end   = start + c->assocs;    

  //search for invalid first
  for (ii = start; ii < end; ii++){
    if(!c->entries[ii].valid){
      return ii;
    }
  }

  switch(c->repl_policy){
  case REPL_LRU: 
    return mcache_find_victim_lru(c, set);
  case REPL_RND: 
    return mcache_find_victim_rnd(c, set);
  case REPL_SRRIP: 
    return mcache_find_victim_srrip(c, set);
  default: 
    assert(0);
  }

  return -1;

}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns
mcache_find_victim_lru (MCache *c,  uns set)
{
  uns start = set   * c->assocs;    
  uns end   = start + c->assocs;    
  uns lowest=start;
  uns ii;


  for (ii = start; ii < end; ii++){
    if (c->entries[ii].last_access < c->entries[lowest].last_access){
      lowest = ii;
    }
  }

  return lowest;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns
mcache_find_victim_rnd (MCache *c,  uns set)
{
    uns start = set   * c->assocs;    
    uns victim = start + rand()%c->assocs; 

    return  victim;   
}



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns
mcache_find_victim_srrip (MCache *c,  uns set)
{
  uns start = set   * c->assocs;    
  uns end   = start + c->assocs;    
  uns ii;
  uns victim = end; // init to impossible

  while(victim == end){
    for (ii = start; ii < end; ii++){
      if (c->entries[ii].ripctr == 0){
	victim = ii;
	break;
      }
    }
    
    if(victim == end){
      for (ii = start; ii < end; ii++){
	c->entries[ii].ripctr--;
      }
    }
  }

  return  victim;   
}




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns mcache_get_index(MCache *c, Addr addr){
  uns retval;

  switch(c->index_policy){
  case 0:
    retval=addr%c->sets;
    break;

  default:
    exit(-1);
  }

  return retval;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void  mcache_print_stats(MCache *c, char *header){
  double missrate = 100.0 * (double)c->s_miss/(double)c->s_count;
  
  printf("\n%s_ACCESS       \t : %llu",  header,  c->s_count);
  printf("\n%s_MISS         \t : %llu",  header,  c->s_miss);
  printf("\n%s_MISSRATE     \t : %6.3f", header,  missrate);
  printf("\n");
}





