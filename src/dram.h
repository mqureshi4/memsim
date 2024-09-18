#ifndef DRAM_H
#define DRAM_H

#include "global_types.h"
#include "dramchannel.h"


#define MAX_DRAM_CHANNELS 32

typedef struct DRAM   DRAM;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef enum DRAM_RefType_Enum {
  DRAM_REF_NONE=0, // NONE
  DRAM_REF_SB=1, // SAME BANK
  DRAM_REF_AB=2, // ALL BANK
  NUM_DRAM_REFTYPE=3
} DRAM_RefType;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef enum DRAM_RFMType_Enum {
  DRAM_RFM_NONE=0, // NONE
  DRAM_RFM_SB=1, // SAME BANK
  DRAM_RFM_AB=2, // ALL BANK -- legacy reason
  NUM_DRAM_RFMTYPE=3
} DRAM_RFMType;


typedef enum DRAM_MappingPolicy_Enum {
    DRAM_MAP_COFFEELAKE=0,
    DRAM_MAP_SKYLAKE=1,
    DRAM_MAP_MOP=2,  // MINIMALIST OPEN PAGE
    DRAM_MAP_ZEN=3, // ZEN MAPPING,
    DRAM_MAP_RUBIX=4, // RUBIX MAPPING,
    DRAM_MAP_LINESTRIPE=5, // LINE STRIPE
    NUM_DRAM_MAP=6
} DRAM_MappingPolicy;


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

struct DRAM {
    uns         id; // we may have multiple dram, so id helps differentiate
    char name[256];

    DRAM_Channel  *channel[MAX_DRAM_CHANNELS];
    DRAM_RefType  refmode;

    uns64       memsize;
    uns64       num_banks;
    uns64       rowbuf_size;
    uns64       num_channels;
    uns64       num_rowbufs;

    uns64       lines_in_rowbuf;
    uns64       rowbufs_in_mem;
    uns64       rowbufs_in_bank;
    uns64       lines_in_mem; // total lines in this DRAM module
    uns64       banks_in_channel;

    uns64       refsb_ptr; // pointer points to bank in bankgroup being refreshed
    uns64       banks_in_bankgroup; // needed for REF-SB and RFM-SB
  
    uns64       last_trefi_cycle; // for refresh
    uns64       last_trefw_cycle;
    uns64       num_trefi;
    uns64       num_trefw;
    uns64       s_tot_ACT;
    uns64       s_tot_REQ;
    uns64       s_tot_wait_delay;
    uns64       s_tot_service_delay;
    uns64       s_tot_bus_delay;
    uns64       s_tot_RFM;

};

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

DRAM*   dram_new(void);
void    dram_cycle(DRAM *d);
void    dram_parseaddr(DRAM *d, Addr lineaddr, uns64 *myrowbufid, uns64 *mybankid, uns64 *mychannelid);
Flag    dram_insert(DRAM *d, Addr lineaddr, Flag wb );

void    dram_print_state(DRAM *d);
void    dram_print_stats(DRAM *d);



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


#endif // DRAM_H
