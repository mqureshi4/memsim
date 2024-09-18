#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "os.h"

extern uns OS_PAGESIZE;
extern uns LINESIZE;



OS *os_new(uns num_pages, uns num_threads){
  OS *os = new OS; 
  
  os->num_pages      = num_pages;
  os->num_threads    = num_threads;
  os->lines_in_page  = OS_PAGESIZE/LINESIZE;
  os->s_miss_count   = 0;
  
  os->ipt     = (Flag *) calloc (os->num_pages, sizeof (Flag));
  assert(os->ipt);
  
  printf("Initialized OS for %u pages\n", num_pages);

  os->ipt[0]=TRUE; // not allocating page-0 to anyone

  return os;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns64 os_vpn_to_pfn(OS *os, uns64 vpn, uns tid){
  assert((uns64)vpn>>36 == 0);
  uns64 tid_vpn = ((uns64)(tid) << 36) + vpn; // Embed tid information in high bits

   if(os->pt.count(tid_vpn)==0){ 
    uns64 victim_pfn = os_get_victim_from_ipt(os);
    os->pt[tid_vpn]=victim_pfn; // <======== INSERT 
    os->s_miss_count++;
  }

  return os->pt[tid_vpn];
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns64    os_get_victim_from_ipt(OS *os){
  uns MAX_TRIES=1000;
  uns tries=0;

  assert(os->s_miss_count < 0.95*os->num_pages);

  while(tries < MAX_TRIES){
    uns victim = rand()%os->num_pages;
    if(os->ipt[victim] == FALSE){
      os->ipt[victim] = TRUE;
      return victim;
    }
    tries++;
  }

  printf("OS Could not find an invalid page. Dying\n");
  assert(0);
  return 0; 
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void os_print_stats(OS *os){
    char header[256];
    sprintf(header, "OS");
    printf("\n%s_PAGE_MISS       \t : %llu",  header, os->s_miss_count);
    printf("\n");
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

Addr os_v2p_lineaddr(OS *os, Addr lineaddr, uns tid){
  uns64 vpn = lineaddr/os->lines_in_page;
  uns64 lineid = lineaddr%os->lines_in_page;
  uns64   pfn = os_vpn_to_pfn(os, vpn, tid);
  Addr retval = (pfn*os->lines_in_page)+lineid;
  return retval;
}
