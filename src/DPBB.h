/*
(c) Yaroslav Salii & the Parliament of Owls, 2019+
License:sort of CC---if you go commercial, contact me

DP-BB v.1.0 2019-02-06; DP-BB solution, also logging for the number of states fathomed
	  


branch & bound scheme for dynamic programming; requires a lower bound function 
and an upper bound (value); may be reminiscent of my OP-PC code

*/

#ifndef DPBB_H_
#define DPBB_H_

#include"RTDP.h" 
#include"lower-bound.h"
#include<functional>//for std::greater 

//======NAMECALLING==========================/
inline std::string mk_DPBB_Code(const t_Direction iDirn, const std::string iLB_Code)
{
	return mkSlnCode(getDirectionCode(iDirn), "DPBB", iLB_Code);
}
//lower-bound.h::elCheapoLB is CHP; thus, FWD-DPBB-CHP 
//-------------------------------------------/

//==EXACT==DP==WITH==BRANCH=&=BOUND==========/
using t_BBDP =
struct t_BBDP : public t_DP
{
	const t_cost UB; //upper bound (a value)
	const std::string LB_Code; // lower bound algorithm's name; consider a pointer this funciton...
	size_t nFathomedStatesTotal{ 0 };

	//initialization like in ordinary DP
	t_BBDP(const t_Instance& iproblem, const t_Direction iDIR, const t_cost iUB, const std::string iLB_Code)
		: t_DP(iproblem, iDIR, mk_DPBB_Code(iDIR, iLB_Code))
		, UB(iUB) {};

	//========STATE==FATHOMING:==STATE_VAL+CO-STATE_LB<UB========/
	bool notFathomed_strictly(const t_cost stValue, const t_cost iLB, const t_cost iUB)
	{
		if (p.f.cAgr(stValue, iLB) <= iUB)//not aware of direction; might fail for non-commutative aggregation
			return true; else return false;
	}

	//==========B&B-AWARE====BF====COMPUTATION===========================/
	size_t compExpandBF(const t_bin& K //taskset
		, const t_bin& EK //its feasible expansions:its interfaces
		, const t_stLayer& prevL//prev. layer, for computing BF
		, t_stLayer& thisL//this layer, to fill in the BF's values
		, t_stLayer& nextL)//next layer, for generating the next taskset(s)
	{
		size_t nFathSt = 0;
		foreach_elt(m, EK, p.dim)//for each expanding city
		{
			bool canExpandWith_m = false;//default to ``can't expand:over budget''
			//for each (exit) point of the expanding city
			foreach_point(x, p.popInfo[m].pfirst, p.popInfo[m].plast)
			{
				//find this state's cost in view of prevL through (BF) 
				auto xK_Cost = minmin(x, K, prevL, p, D);
				if(notFathomed_strictly(  xK_Cost
										, elCheapoLB(x,setMinus(p.wkOrd.omask,K).reset(m),p,D).first
										, this->UB))
				{
					canExpandWith_m = true;
					thisL[K].emplace(x, xK_Cost);
				}
				else nFathSt++;//so it's fathomed; increase the counter
			}//next (exit) x from the city m 
			//expand the next layer with m if it's worth it (if at least one state is not over budget)
			if (canExpandWith_m)
				nextL[K | (BIT0 << m)].clear();
		}//next expanding city m\in EK

		return nFathSt;//tell the caller how many states were fathomed in view of UB and elCheapoLB
	}
//==========THE=WHOLE=SOLUTION=PROCEDURE====================/
	t_solution solve()
	{
		mtag l = 1; //current layer number
		for (l = 1; l < p.dim; l++)//for layers 1..dim-1;
		{
			//=================OMP=========TASKS====================/
			//for each task set (ideal/filter) of cardinality l
			size_t nStates = 0;//to count the states at this layer
			size_t nFathomedStates = 0; //to count the states FATHOMED at this layer
			for (auto ts : layer[l])//implemented as ts.first; .second is for the states
			{
				/*recall: FWD:coMin, BWD:coMax
				t_bin fexp = D.gE(ts.first, p.ord, p.wkOrd);*/
				//compute (BF) for all (x,K): x\in fexp, K=ts.first; it returns the number of FATHOMED states
				nFathomedStates+=compExpandBF(ts.first
					, D.gE(ts.first, p.ord, p.wkOrd)
					, layer[l - 1]
					, layer[l]
					, layer[l + 1]);
				nStates += layer[l][ts.first].size();//count the states associated with ts.first
			}//next task set (filter)
//-----------OMP-------------TASKS-------DONE-----------/
			{//all current-layer states' values computed; tasksets not wholly fathomed were expanded.

				slnTime.vlayerDone[l] = myClock::now();//get the current time
				//measure current memory use /w respect to last recorded
				auto memUse = t_memRec();
				//record it
				slnMem.emplace_back(memUse);
				nStatesTotal += nStates;//add this layer's state count to the total
				nFathomedStatesTotal += nFathomedStates;//add this layer's fathomed states' count to the total
				
				//brag(layerNumber:wall-clock:delta:worstState:bestState)
				slnLog.open(logName, std::ios_base::app);
				slnLog << mkReportL(l, time(NULL), slnTime._rel_time(0, l), slnTime._rel_time(l - 1, l),
					layer[l].size(), t_stateDesc(), t_stateDesc(), memUse, nStates, nFathomedStates) << "\n";
				slnLog.close();
			
				/*now, if there's nothing to expand anymore
				(every expansion is over p.UB), layer[l+1] is empty,
				and we're done; break the cycle and stop the solution*/
				if (layer.at(l + 1).empty()) {
					slnLog.open(logName, std::ios_base::app);
					slnLog << "\nDPBB_ALL_STATES_FATHOMED: impossible to get better than UB="<<this->UB << "\n";
					slnLog.close();
					break;
				}
			}
		}//next layer
		auto bfEndTime_t = time(NULL);
		t_stateDesc full;
/////////////FINAL////LAYER//////////////////////////////////////////////////////////////////////////////		
		if (l == p.dim)//if we didn't fathom all the states in view of UB and reached the final layer
		{
			//done the ordinary layers; proceed to the complete problem BWD:(0,\rg{1}{dim})/FWD:(\trm,\rg{1}{dim})
			//FWD: \trm==p.dim+1; BWD: (the) base==0
			ptag lastIntf = (D == FWD) ? (p.dim + 1) : (0);
			layer[p.dim].begin()->second.emplace(lastIntf, minmin(lastIntf, p.wkOrd.omask, layer[p.dim - 1], p, D));
			full = t_stateDesc{ layer[p.dim].begin()->first //taskset==omask
				, layer[p.dim].begin()->second.begin()->first //FWD: \trm, BWD: 0
				, layer[p.dim].begin()->second.begin()->second };//the whole problem's cost 



		//time this doing 
			slnTime.vlayerDone[p.dim] = myClock::now(); bfEndTime_t = time(NULL);
			//measure current memory use 
			auto memUse = t_memRec();
			//record it
			slnMem.emplace_back(memUse);
			//log the event
			slnLog.open(logName, std::ios_base::app);
			slnLog << mkReportL(p.dim
				, bfEndTime_t
				, slnTime._rel_time(0, p.dim)
				, slnTime._rel_time(p.dim - 1, p.dim)
				, layer[p.dim].size(), full, full
				, memUse
				, this->nStatesTotal) << "\n"; //it's the last layer, tell about the total states' number
			slnLog.close();
		}
		//----------BF----DONE----------------------------------/
		//============REPORTS,==RECOVERY,==ETC.=================/
		slnLog.open(logName, std::ios_base::app);
		slnLog << "\n" << "BF DONE: " << mkTimeStamp(bfEndTime_t) << "\n" <<
			"TOTAL DURATION: "
			<< mkMsecDurationFull(slnTime._rel_time(0, l)) << "\n"
			<< "TOTAL DURATION IN SECONDS: "
			<< mkMsecDurationBrief(slnTime._rel_time(0, l)) << "\n"
			<< "\n" << "RAM USAGE AT LAST LAYER: " << to_stringBytes(slnMem.back().physMem)
			<< "\n" << "VM USAGE AT LAST LAYER: " << to_stringBytes(slnMem.back().virtMem)
			<< "\n" << "TOTAL STATES PROCESSED:" << nStatesTotal
			<< "\n" << "RAM BYTES PER STATE(APPX):" << int(slnMem.back().physMem / nStatesTotal)
			<< "\n" << "DPBB TOTAL STATES FATHOMED:" << nFathomedStatesTotal;

		slnLog.close();
		
		if (l==p.dim)//if we didn't fathom all the states in view of UB and reached the final layer
		{
			//recover the solution
			this->result = recoverSln(full, layer, p, D);//can now destroy this->layer
			//time the event
			slnTime.recoveryDone = myClock::now();
			//report the event
			slnLog.open(logName, std::ios_base::app);
			slnLog << "\n" << "SOLUTION RECOVERY DONE: " << mkTimeStamp(time(NULL)) << "\n" <<
				"RECOVERY TOOK: "
				<< mkMsecDurationBrief(msecDuration(slnTime.vlayerDone[p.dim], slnTime.recoveryDone))
				<< "\n"
				<< "BF + RECOVERY DURATION: "
				<< mkMsecDurationFull(msecDuration(slnTime.vlayerDone[0], slnTime.recoveryDone)) << "\n"
				<< "BF + RECOVERY DURATION IN SECONDS: "
				<< mkMsecDurationBrief(msecDuration(slnTime.vlayerDone[0], slnTime.recoveryDone)) << "\n";

			slnLog.close();
			//====================FINAL==REPORT(DUMP)==================/
			//NB:relies on result being written to this->result before 
			slnDump.open(dumpName);
			slnDump << mkFullDump(p, result, D);
			slnDump.close();
		}
		else //all states fathomed: given UB is proven to be a lower bound
		{//consider not creating .dump in this case
			slnDump.open(dumpName);
			slnDump << "\n DPBB_ALL_STATES_FATHOMED: impossible to get better than UB=" << this->UB << "\n";
			slnDump.close();
		}
		
		return this->result;
	}

};


#endif // !DPBB_H_
