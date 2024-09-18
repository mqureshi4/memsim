#ifndef MEMSYS_H
#define MEMSYS_H

#include <unordered_map>
#include "global_types.h"
#include "dram.h"

typedef struct MemSys MemSys;

typedef struct MSHR_Entry MSHR_Entry;


struct MSHR_Entry {
  uns coreid;
  uns robid;
  uns64 inst_num;
};


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

struct MemSys {
  DRAM     *mainmem;
  uns64     lines_in_mainmem_rbuf;
  std::unordered_map<uns64, MSHR_Entry> mshr;
};

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

MemSys *memsys_new(void);
Flag    memsys_access(MemSys *m, Addr lineaddr, uns coreid, uns robid, uns64 inst_num, Addr wb_lineaddr);
void    memsys_cycle(MemSys *m);
void    memsys_print_state(MemSys *m);
void    memsys_print_stats(MemSys *m);
void    memsys_mshr_insert(MemSys *m, Addr lineaddr, uns coreid, uns robid, uns64 inst_num);
void    memsys_callback(MemSys *m, Addr lineaddr);

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

#endif // MEMSYS_H
