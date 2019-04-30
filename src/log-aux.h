/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

log-aux v.1.0 2017-01-30; 
TODO v.1.1 2017-08-29: TOOK* from time_t to std::chrono::milliseconds
log-aux v.1.2 2019-02-06: added states' & fath.states counters to output; rewrote mkReportL to use isperse from reader-base.h

for simplicity's sake, TOOK* in SS.MS,
not in HH:MM:SS.MS mostly not that long anyway

logging & auxiliaries; for logging etc.

chuck all report-related output here?

*/

#ifndef LOG_AUX_H_
#define LOG_AUX_H_

#include<iomanip>
#include<sstream>
#include<string>
#include<vector>
#include<list>

#include"bitset-base.h"
#include"time-aux.h"//for time output
#include "dp-base.h"//for t_stateDesc output
#include "mem-rec.h"//for total&relative memory usage output
#include"reader-base.h"//t_lines, isperse, etc



//memory usage is tilde-separated, 10GB~2MB~3KB~123B
const std::string logHeader = "LAYER;DONE_AT;TOOK_TOTAL;TOOK;N_TSETS;BEST_PRICE;WORST_PRICE;"
							  "TS_B;IF_B;TS_W;IF_W;"//for the heuristic; leave blank for max performance of exact
							  "M_RAM;M_VIRT;"//records memory usage;   
							  "N_STATES;"//the number of states, finally;
							  "N_FATH_ST"; //how many states were fathomed, DP-BB only

inline std::string mkReportL(const mtag l
							 , const time_t done_at
							 , const t_msec took_total //run time from DP start (layer 0 done at) to current time
							 , const t_msec took //run time from the start of the current layer to the end of the current layer
							 , const size_t nTSets
							 , const t_stateDesc best_st
							 , const t_stateDesc worst_st
							 , const t_memRec pm
							 , const size_t nStates  = 0//optional, just in case I don't want to really call it
							 , const size_t nFathStates = 0) //also optional, only for DP-BB actually
{
	std::string colSep = ";"; t_lines vout;
	vout.push_back(std::to_string(l));
	vout.push_back(mkTimeStamp(done_at));
	vout.push_back(mkMsecDurationFull(took_total));
	vout.push_back(mkMsecDurationBrief(took));
	vout.push_back(std::to_string(nTSets));
	vout.push_back(std::to_string(best_st.v));
	vout.push_back(std::to_string(worst_st.v));
	vout.push_back(to_stringHex(best_st.K));
	vout.push_back(std::to_string(best_st.x));
	vout.push_back(to_stringHex(worst_st.K));
	vout.push_back(std::to_string(worst_st.x));
	vout.push_back(to_stringBytes(pm.physMem));//report current RAM usage
	vout.push_back(to_stringBytes(pm.virtMem));//report current virt.mem (page/swap, no incl. RAM) usage
	vout.push_back(std::to_string(nStates));
	vout.push_back(std::to_string(nFathStates));
	
	return isperse(vout, colSep);
}

#endif