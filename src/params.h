#ifndef __PARAMS_H__
#define __PARAMS_H__

#include "global_types.h"
#include <string.h>

#define PRINT_DOTS   1
#define DOT_INTERVAL 1000000

uns64  CORE_WIDTH   =    4;
uns64  ROB_SIZE     =    256;


uns64       TRACE_LIMIT     = (2*1000*1000*1000); // Max 2B memory access
uns64       INST_LIMIT      = 100*1000*1000; // set default 
uns64       NUM_THREADS     = 0;
uns64       RATE_MODE_THREADS  = 0;
uns64       LINESIZE        = 64; 
uns64       OS_PAGESIZE     = 4096; 


uns64       L3_SIZE_KB      = 8192; 
uns64       L3_ASSOC        = 16; 
uns64       L3_LATENCY      = 24; // cycles
uns64       L3_REPL         = 0; //0:LRU 1:RND 2:SRRIP
uns64       L3_PERFECT      = 0; //simulate 100% hit rate for L3


uns64       MEM_SIZE_MB     = 32768; 
uns64       MEM_CHANNELS    = 2;
uns64       MEM_BANKS       = 64; // Total banks in memory, not  per channel
uns64       MEM_PAGESIZE    = 4096; //Size of a DRAM Row
uns64       MEM_CLOSEPAGE   = 0;

uns64       DRAM_REF_POLICY= 2; /* ALL BANK */
uns64       DRAM_MAP_POLICY=3; // AMD-ZEN
uns64       DRAM_SCHED_POLICY=1; // 0:FCFS 1:FR-FCFS
uns64       DRAM_MOP_GANGSIZE = 8; 

uns64       tRC=48*4; // all DRAM latencies specified as ns x 4 (4GHz processor)
uns64       tACT=16*4;
uns64       tCAS=12*4;
uns64       tPRE=16*4;
uns64       tRAS=32*4;
uns64       tRDRD=13;
uns64       tBUS=13;

uns64       tRFC=410*4;
uns64       tREFI=3900*4;
uns64       tREFW=32*1000*1000*4;
uns64       tALERT=350*4; //-- only 350ns of stall out of 530
uns64       tRFM=205*4;   //-- half latency as tRFC (same bank RFM)

uns64       DRAM_RFM_POLICY=0;
uns64       RFM_TH=16;
uns64       RFM_REFTH=16;
uns64       RFM_RAAMMT=6*16;
uns64       DRAM_BANKGROUPS=8;
uns64       DRAM_MAX_TOPEN   = 3900*5; // with refresh postponement

uns64       MEM_RSRV_MB      = 0; // reserving memory for metadata (e.g. CRA)

uns64       RAND_SEED       = 1234;

uns64       cycle;
uns64       last_printdot_cycle;
char        addr_trace_filename[256][1024];
int         num_threads = 0;


/***************************************************************************************
 * Functions
 ***************************************************************************************/


void die_usage() {
    printf("Usage : sim [options] <MAT_trace_0> ... <MAT_trace_n> \n\n");
    printf("Trace driven DRAM-cache based memory-system simulator\n");

    printf("   Option (examples)\n");
    printf("               -inst_limit  <num>    Set instruction limit (Default: 0)\n");
    printf("               -ratemode    <num>    Number of threads in rate mode (Default: 1)\n");
    printf("               -l3sizemb    <num>    Set L3  Cache size to <num> MB (Default: 1MB)\n");
    printf("               -l3assoc     <num>    Set L3  Cache assoc <num> (Default: 16)\n");
    printf("               -l3perfect            Set L3  to 100 percent hit rate(Default:off)\n");
    printf("               -memclosepage         Set DRAM to close page (Default:off)\n");

    exit(0);
}


/***************************************************************************************
 * Functions
 ***************************************************************************************/

												          
void die_message(const char * msg) {
    printf("Error! %s. Exiting...\n", msg);
    exit(1);
}




/***************************************************************************************
 * Functions
 ***************************************************************************************/



void read_params(int argc, char **argv){
  int ii;

    //--------------------------------------------------------------------
    // -- Get command line options
    //--------------------------------------------------------------------    
    for ( ii = 1; ii < argc; ii++) {
	if (argv[ii][0] == '-') {	    
	  if (!strcmp(argv[ii], "-h") || !strcmp(argv[ii], "-help")) {
		die_usage();
	    }

	   else if (!strcmp(argv[ii], "-inst_limit")) {
	     if (ii < argc - 1) {		  
	       INST_LIMIT = atoi(argv[ii+1]);
	       ii += 1;
	     }
	   }

	   else if (!strcmp(argv[ii], "-robsize")) {
	     if (ii < argc - 1) {		  
	       ROB_SIZE = atoi(argv[ii+1]);
	       ii += 1;
	     }
	   }
	  
	   else if (!strcmp(argv[ii], "-os_pagesize")) {
		if (ii < argc - 1) {		  
		    OS_PAGESIZE = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }
	  
	    else if (!strcmp(argv[ii], "-l3perfect")) {
	      L3_PERFECT=1; 
	    }


	    else if (!strcmp(argv[ii], "-l3repl")) {
		if (ii < argc - 1) {		  
		    L3_REPL = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-l3sizekb")) {
		if (ii < argc - 1) {		  
		    L3_SIZE_KB = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-l3sizemb")) {
		if (ii < argc - 1) {		  
		    L3_SIZE_KB = 1024*atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-l3assoc")) {
		if (ii < argc - 1) {		  
		    L3_ASSOC = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }


	    else if (!strcmp(argv[ii], "-l3latency")) {
		if (ii < argc - 1) {		  
		    L3_LATENCY = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	  
	    else if (!strcmp(argv[ii], "-memclosepage")) {
	      MEM_CLOSEPAGE = 1;
	    }

	    else if (!strcmp(argv[ii], "-drammaxtopen")) {
		if (ii < argc - 1) {		  
		    DRAM_MAX_TOPEN = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	   else if (!strcmp(argv[ii], "-rand_seed")) {
	      if (ii < argc - 1) {		  
		RAND_SEED = atoi(argv[ii+1]);
		ii += 1;
	      }
	    }
	  
	  //-- legacy
	    else if (!strcmp(argv[ii], "-mtapp")) {
		if (ii < argc - 1) {		  
		    RATE_MODE_THREADS = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-ratemode")) {
		if (ii < argc - 1) {		  
		    RATE_MODE_THREADS = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-dramrefpolicy")) {
		if (ii < argc - 1) {		  
		    DRAM_REF_POLICY = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-drammappolicy")) {
	      if (ii < argc - 1) {		  
		DRAM_MAP_POLICY = atoi(argv[ii+1]);
		ii += 1;
	      }
	    }

	    else if (!strcmp(argv[ii], "-dramschedpolicy")) {
	      if (ii < argc - 1) {		  
		DRAM_SCHED_POLICY = atoi(argv[ii+1]);
		ii += 1;
	      }
	    }

	    else if (!strcmp(argv[ii], "-drammop")) {
	      if (ii < argc - 1) {		  
		DRAM_MOP_GANGSIZE = atoi(argv[ii+1]);
		ii += 1;
	      }
	    }

	  
	    else if (!strcmp(argv[ii], "-limit")) {
		if (ii < argc - 1) {		  
		    TRACE_LIMIT = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }


	    else if (!strcmp(argv[ii], "-tRC")) {
		if (ii < argc - 1) {		  
		    tRC = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-tRFC")) {
	        if (ii < argc - 1) {		  
		    tRFC = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-tACT")) {
	        if (ii < argc - 1) {		  
		    tACT = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-tCAS")) {
	        if (ii < argc - 1) {		  
		    tCAS = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-tPRE")) {
	        if (ii < argc - 1) {		  
		    tPRE = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-tRAS")) {
	        if (ii < argc - 1) {		  
		    tRAS = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-tRDRD")) {
	        if (ii < argc - 1) {		  
		    tRDRD = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-tREFI")) {
	      if (ii < argc - 1) {		  
		tREFI = atoi(argv[ii+1]);
		ii += 1;
	      }
	    }
	  
	    else if (!strcmp(argv[ii], "-tREFW")) {
	        if (ii < argc - 1) {		  
		    tREFW = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	   else if (!strcmp(argv[ii], "-tRFM")) {
	        if (ii < argc - 1) {		  
		    tRFM = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    
	  //-- legacy
	    else if (!strcmp(argv[ii], "-enablerfm")) {
	      DRAM_RFM_POLICY = 2; // All bank by default
	    }

	    else if (!strcmp(argv[ii], "-dramrfmpolicy")) {
		if (ii < argc - 1) {		  
		    DRAM_RFM_POLICY = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }


	    else if (!strcmp(argv[ii], "-rfmth")) {
		if (ii < argc - 1) {		  
		    RFM_TH = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-rfmrefth")) {
		if (ii < argc - 1) {		  
		    RFM_REFTH = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }

	    else if (!strcmp(argv[ii], "-rfmraammt")) {
		if (ii < argc - 1) {		  
		    RFM_RAAMMT = atoi(argv[ii+1]);
		    ii += 1;
		}
	    }


	    else {
		char msg[256];
		sprintf(msg, "Invalid option %s", argv[ii]);
		die_message(msg);
	    }
	}
	else if (num_threads<MAX_THREADS) {
	    strcpy(addr_trace_filename[num_threads], argv[ii]);
	    num_threads++;
	    NUM_THREADS = num_threads;
	}
	else {
	    char msg[256];
	    sprintf(msg, "Invalid option %s", argv[ii]);
	    die_message(msg);
	}    
    }
	    
    //--------------------------------------------------------------------
    // Error checking
    //--------------------------------------------------------------------
    if (num_threads==0) {
	die_message("Must provide valid at least one addr_trace");
    }

    if( RATE_MODE_THREADS ){
	num_threads = RATE_MODE_THREADS;
	NUM_THREADS = RATE_MODE_THREADS;
	for(ii=1; ii<num_threads; ii++){
	    strcpy(addr_trace_filename[ii], addr_trace_filename[0]);
	}
    }
}





#endif  
