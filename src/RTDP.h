/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

RTDP v.1.0 2017-01-30;
RTDP v.1.1 2018-02-14; adding tie-breaking code (less bit mask is better &c) to cacheRtBF
RTDP v.1.2 2019-02-06; added states' counter to DP code & its logging (only exact DP, not hRtDP)
RTDP v.1.2.1 2019-02-07; added states' total counter (a field in t_DP; not used in hRtDP), removed OMP pragmas


restricted & exact dynamic programming solutions



*/

#ifndef RTDP_H_
#define RTDP_H_

#include<iostream>
#include<fstream>
#include<queue>
#include<list>

#include"dp-base.h"
#include"dp-recovery.h"
#include"instance.h"
#include"log-aux.h"
#include"reader-base.h"

//========NAMECALLING=&=AUXILIARIES==============================/

/*
all fields separated with "-"
methodDirn: method direction (BWD:backwards/filters or FWD:forwards/ideals)
methodCode: name of method (DP for exact DP; hRtDP for heuristic restricted DP)
methodNote: method-specific notes; hRtDP specifies its breadth, for instance
*/
inline std::string mkSlnCode
		(const std::string methDirn
		, const std::string methCode
		, const std::string methNote)
{
	std::string sep = "-";
	return isperse(t_lines {methDirn, methCode, methNote},sep);
}

inline std::string mk_hRtDP_Code(const t_Direction DIR, const uint32_t ibreadth)
{
	return mkSlnCode(getDirectionCode(DIR), "hRtDP", std::to_string(ibreadth));
}

inline std::string mk_DP_Code(const t_Direction DIR)
{
	return mkSlnCode(getDirectionCode(DIR), "DP", "");
}
//--------------------------------------------------------------/

//========SOLUTION===OBJECT==========================/
/*
FILE NAMING CONVENTION:
log: slnName.log
dump: slnName.dump (dump, more compact, not too pretty output)
*/
using t_DP =
struct t_DP
{
	const t_Instance& p;//the PROBLEM we solve; a constant reference, to avoid the pointer syntax
	//order direction: BWD(feasible tsets:order filters)/FWD(feasible tsets:order ideals)
	const t_Direction D;
	
	//$type-$parameters; 
	const std::string slnCode; //FWD-DP, etc.
	const std::string slnName;//p.instName + "-" + slnCode
	const std::string logName;//slnName.log
	const std::string dumpName;//slnName.dump

	//dedicated logging ofstream; would use cerr for what.log
	std::ofstream slnLog;//slnName.log
	std::ofstream slnDump;//slnName.dump
	
	t_stopwatch slnTime;//how long did it take to (solve, etc.)
	std::deque<t_memRec> slnMem; t_memRec baselineMem;

	t_solution result; //would line up and report in another procedure
	size_t nStatesTotal{ 0 };//the total number of states, will be computed during solution by this->solve()
	t_vstLayer layer;//the Bellman Function

	t_DP(const t_Instance& iproblem, const t_Direction iDIR, const std::string islnCode)
		: p(iproblem)
		, D(iDIR)
		, slnCode(islnCode)//, slnCode(mk_DP_Code(D))
		, slnName(iproblem.instName + "-" + slnCode)
		, logName(slnName + ".log")
		, dumpName(slnName + ".dump")
	{
		//==================INIT==LOG=&=STOPWATCH=====================/
		//prep the layer end times vector
		slnTime.vlayerDone.resize(p.dim + 1);
		//prep the memory usage counter: get the baseline (without any BF values)
		baselineMem = t_memRec();
		//bind the log file, purge it (further calls would ios_base::append there)
		slnLog.open(logName, std::ios_base::out);
		//start counting the time
		slnTime.start = time(NULL); //
		slnTime.vlayerDone[0] = myClock::now();
		std::ostringstream tmp;
		tmp << "This is " + slnName + ". Started on " << mkTimeStamp(slnTime.start) << "\n"
			<<"baseline memory use: "<<baselineMem.to_string()<<"\n";
		//brag about starting, into log and stdout
		slnLog << tmp.str(); std::cout << tmp.str();
		//write a header into the log and close it for now
		slnLog << logHeader<<"\n"; slnLog.close();
		//-------------------------------------------------------------/
		
		//===================INIT==SLN=&=BF(LAYERS)==DATA==STRUCTURES==/
		//prepare the result deque: make sure base and terminal fit in
		result.resize(p.dim + 2);//dim=proper clusters; 0||1..dim||\trm
		layer.resize(p.dim + 1);//layers 0..dim; l[dim]=\{(0,\rg{1}{dim})\}

		
		//	for each ultimate element (BWD: max/they don't send; FWD: min/don't receive)
		//gE(EmptySet,...): expansions for empty set---the ultimate elements
		foreach_elt(m, D.gE(EmptySet, p.ord, p.wkOrd), p.dim)
		{	//for each point therein (should be BWD: each exit point FWD: each entry point
			foreach_point(pt, p.popInfo[m].pfirst, p.popInfo[m].plast)
			{
				/*FWD initial conditions: v(x,\{\varnothing\})=extMtCost(base,x,\{\varnothing\})
				BWD initial conditions: v(x,\{\varnothing\})=extMtCost(x,terminal,\{\varnothing\})*/
				t_cost costIntlCnd = (D == FWD)
					? (p.f.cExtMt(0, pt, 0, p.cost))
//BWD: had to skip to cost directly to support TD-TSP-cost by KCvH
					: /*(p.cost[pt][p.dim + 1]);*/(p.f.cExtMt(pt, p.dim + 1, 0, p.cost)); 
				layer[0][0].emplace(std::make_pair(pt, costIntlCnd) );
			}
			//prime cardinality-1 tasksets (ord.Max singletons)
			layer[1][BIT0 << m].clear();// layer[1] <- \{m\}
		}//next ultimate element
	
		 //count the initial states
		nStatesTotal += layer[0][0].size();
		 //measure the current memory usage and record that;
		slnMem.emplace_back(t_memRec(baselineMem));
	}//----------DONE----CONSTRUCTION-&---SOLUTION--PRIMING---------/
	/*delegate constructor: it “knows” its own SLNCODE function; 
	necessary only for this---base---class*/
	t_DP(const t_Instance& iproblem, const t_Direction iDIR)
		: t_DP(iproblem, iDIR, mk_DP_Code(iDIR)){};
	//====================BF====COMPUTATION===========================/
	void compExpandBF(const t_bin& K //taskset
		, const t_bin& EK //its feasible expansions:its interfaces
		, const t_stLayer& prevL//prev. layer, for computing BF
		, t_stLayer& thisL//this layer, to fill in the BF's values
		, t_stLayer& nextL)//next layer, for generating the next taskset(s)
	{
		foreach_elt(m, EK, p.dim)//for each expanding city
		{
			//for each (exit) point of the expanding city
			foreach_point(x, p.popInfo[m].pfirst, p.popInfo[m].plast)
			{
				//find its cost in view of prevL through (BF) and put K->x->cost
				thisL[K].emplace(x, minmin(x, K, prevL, p, D));
			}//next (exit) x from the city m 
			//expand the next layer
//            #pragma omp critical(statewrite)
			nextL[K | (BIT0 << m)].clear();
		}//next expanding city m\in EK
		
		return;
	}

	t_solution solve()
	{
		for (mtag l = 1; l < p.dim; l++)//for layers 1..dim-1;
		{
			//for each task set (ideal/filter) of cardinality l
			size_t nStates = 0;//to count the states at this layer
//=================OMP=========TASKS====================/
            #pragma omp parallel default (shared)
            #pragma omp single nowait
			for (auto ts : layer[l])//implemented as ts.first; .second is for the states
			{
				/*recall: FWD:coMin, BWD:coMax
				t_bin fexp = D.gE(ts.first, p.ord, p.wkOrd);*/
				//compute (BF) for all (x,K): x\in fexp, K=ts.first;
                #pragma omp task untied firstprivate(ts)
				{
					compExpandBF(ts.first
							, D.gE(ts.first, p.ord, p.wkOrd)
							, layer[l - 1]
							, layer[l]
							, layer[l + 1]);
					nStates += layer[l][ts.first].size();//count the states associated with ts.first
				}
			}//next task set (filter)
            #pragma omp taskwait
//-----------OMP-------------TASKS-------DONE-----------/

			//all current-layer states' values computed; all current-layer tasksets expanded.
			{
				nStatesTotal += nStates;//add this layer's state count to the total
				slnTime.vlayerDone[l] = myClock::now();//get the current time
				//measure current memory use /w respect to last recorded
				auto memUse = t_memRec();
				//record it
				slnMem.emplace_back(memUse);
				//brag(layerNumber:wall-clock:delta:worstState:bestState)
				slnLog.open(logName, std::ios_base::app);
				slnLog << mkReportL(l, time(NULL), slnTime._rel_time(0, l), slnTime._rel_time(l - 1, l),
					layer[l].size(), t_stateDesc(), t_stateDesc(), memUse, nStates) << "\n";
				slnLog.close();
			}
		}//next layer
		
		//done the ordinary layers; proceed to the complete problem BWD:(0,\rg{1}{dim})/FWD:(\trm,\rg{1}{dim})
		//FWD: \trm==p.dim+1; BWD: (the) base==0
		ptag lastIntf = (D == FWD) ? (p.dim + 1) : (0);
		layer[p.dim].begin()->second.emplace(lastIntf,	minmin(lastIntf, p.wkOrd.omask, layer[p.dim - 1],p,D));
		t_stateDesc full{ layer[p.dim].begin()->first //taskset==omask
			, layer[p.dim].begin()->second.begin()->first //FWD: \trm, BWD: 0
			, layer[p.dim].begin()->second.begin()->second };//the whole problem's cost

////////////////////////////////////////////////////////////////////////////////////////////////////////
		//time this doing 
		slnTime.vlayerDone[p.dim] = myClock::now(); auto bfEndTime_t = time(NULL);
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
		//----------BF----DONE----------------------------------/
		//============REPORTS,==RECOVERY,==ETC.=================/
		//auto wat = iminmin(full, layer[p.dim - 1], p, D);
		slnLog.open(logName, std::ios_base::app);
		slnLog << "\n" << "BF DONE: " << mkTimeStamp(bfEndTime_t) << "\n" <<
			"TOTAL DURATION: "
			<< mkMsecDurationFull(slnTime._rel_time(0, p.dim)) << "\n"
			<< "TOTAL DURATION IN SECONDS: "
			<< mkMsecDurationBrief(slnTime._rel_time(0, p.dim)) << "\n"
			<< "\n" << "RAM USAGE AT LAST LAYER: " << to_stringBytes(slnMem.back().physMem) 
			<< "\n" << "VM USAGE AT LAST LAYER: " << to_stringBytes(slnMem.back().virtMem)
			<< "\n" << "TOTAL STATES PROCESSED:" << nStatesTotal
			<< "\n" << "RAM BYTES PER STATE(APPX):"<< int(slnMem.back().physMem / nStatesTotal);
			
			//mkDuration(getRelTime(slnTime.vlayerDone[p.dim], slnTime.start));
		slnLog.close();
////////////////////////////////////////////////////////////////////////////////////////////////////////
		//recover the solution
		this->result = recoverSln(full,layer,p,D);//can now destroy this->layer
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
///////////////////////////////////////////////////////////////////////////////////////////////////////
		//====================FINAL==REPORT(DUMP)==================/
		//NB:relies on result being written to this->result before 
		slnDump.open(dumpName); slnDump << mkFullDump(p, result,D);
		slnDump.close();
				
		return this->result;
	}
};
//--------------------------------------------------------------/

//==============RESTRICTED===DP=================================/
/*auxiliary priority queue, for keeping states sorted by value*/
using t_sortaid =
//struct t_sortaid : std::priority_queue < t_stateDesc, std::vector<t_stateDesc>, std::less<t_stateDesc> >
struct t_sortaid : std::priority_queue < t_stateDesc, std::vector<t_stateDesc>, std::less<t_stateDesc> >
{
	t_sortaid(size_t reserve_size)
	{
		this->c.reserve(reserve_size);
	}
};
//--------------------------------/
using t_hRtDP =
struct t_hRtDP : public t_DP
{
	const uint32_t B;//breadth
	//everything as it was before, plus initialize the breadth parameter; re-initialize slnCode (sorry!)
	t_hRtDP(const t_Instance& iproblem, const t_Direction iDIR, uint32_t ibreadth)
		: t_DP(iproblem, iDIR,mk_hRtDP_Code(iDIR,ibreadth))
		, B(ibreadth){	};
	
	//calculate the cost of states (x,K), where x\in EK; keep only B best in pri_queue wpl
	inline void cacheRtBF(
		const t_bin& K
		, const t_bin& EK
		, const t_stLayer& prevL
		, t_sortaid& wpl
		) const //side-effect the workplace, but doesn't change the object
		//relies on this->B,D,p.dim,ord,wkOrd,popInfo,cost; 
	{
		foreach_elt(m, EK, p.dim)//for each expanding city
		{
			//for each (exit) point of the expanding city
			foreach_point(x, p.popInfo[m].pfirst, p.popInfo[m].plast)
			{
				t_cost xK_cst = minmin(x, K, prevL,p,D);//find its cost in view of prevL through (BF)
				//	t_stateDesc newSt{ K, x, xK_cst };
				if (wpl.size() < this->B)//breadth not exceeded
					wpl.emplace(K, x, xK_cst);
				else//breadth exceeded
				{
					//if (x,K) is better than the worst retained, wpl.top(); wired through t_stateDesc.operator<
					/*(x,K) is better than the worst retained
					or (tie-breaker) the cost is the same but new intf (x) is less than the known
					or cost is the same and intf (x) is the same but task set (K) is less than known*/
					if ( wpl.top() > t_stateDesc(K,x,xK_cst ) )//(x,K) is better than the worst retained
					{
						//test if it's already there? nah, K's are disjoint!
						wpl.pop();//expel the worser one
						//wpl.emplace(t_stateDesc{ K, x, xK_cst });//add the newly found

						wpl.emplace(K, x, xK_cst);
					}
				}
			}//next (exit) x from the city m 
		}//next expanding city m\in EK
		return;
	}

	t_solution solve()
	{
		for (mtag l = 1; l < p.dim; l++)//for layers 1..dim-1;
		{
			//there are at most B best states by definition; reserve B "cells" in advance
			t_sortaid bestStates(this->B);

			//for each task set (ideal/filter) of cardinality l
			for (auto ts : layer[l])//implemented as ts.first; .second is for the states
			{

				{
					/*recall: FWD:coMin, BWD:coMax
					t_bin fexp = D.gE(ts.first, p.ord, p.wkOrd);*/
					//compute (BF) for all (x,K): x\in fexp, K=ts.first; keep B best
					cacheRtBF(
						ts.first
						, D.gE(ts.first, p.ord, p.wkOrd)
						, layer[l - 1]
						, bestStates);
				}//END::OMP_TASK
			}//next task set (filter)


			{
				//all current-layer states' values computed; best B remain in bestStates;
				auto wSt = bestStates.top();
				//one---the best---must remain to be reported
				while (bestStates.size() > 1)//write to permanent structure; expand the corresponding taskset
				{
					auto st = bestStates.top(); bestStates.pop();
					layer[l + 1][st.K | (BIT0 << p.cityof[st.x])].clear();//make a next-layer taskset
					layer[l][st.K].emplace(st.x, st.v);//write v(st.K,st.inf)=st.v
				}
				//copy the best state for report
				auto bSt = bestStates.top();
				//write the best state's information into the next layer
				layer[l + 1][bSt.K | (BIT0 << p.cityof[bSt.x])].clear();//make a next-layer taskset
				layer[l][bSt.K].emplace(bSt.x, bSt.v);//write v(bSt.K,bSt.inf)=bSt.v

				slnTime.vlayerDone[l] = myClock::now();// time(NULL);//get the current time
				//measure current memory use /w respect to last recorded
				auto memUse = t_memRec();
				//record it
				slnMem.emplace_back(memUse);
				//brag(layerNumber:wall-clock:delta:worstState:bestState)
				slnLog.open(logName, std::ios_base::app);
				slnLog << mkReportL(l, time(NULL), slnTime._rel_time(0, l), slnTime._rel_time(l - 1, l),
									layer[l].size(), bSt, wSt, memUse) << "\n";
				slnLog.close();
			}//END::OMP_MASTER

		}//next layer

		//done the ordinary layers; proceed to the complete problem BWD:(0,\rg{1}{dim})/FWD:(\trm,\rg{1}{dim})
		//FWD: \trm==p.dim+1; BWD: (the) base==0
		ptag lastIntf = (D == FWD) ? (p.dim + 1) : (0);
		layer[p.dim].begin()->second.emplace(lastIntf, minmin(lastIntf, p.wkOrd.omask, layer[p.dim - 1], p, D));
		t_stateDesc full{ layer[p.dim].begin()->first //taskset==omask
			, layer[p.dim].begin()->second.begin()->first //FWD: \trm, BWD: 0
			, layer[p.dim].begin()->second.begin()->second };//the whole problem's cost

////////////////////////////////////////////////////////////////////////////////////////////////////////
		//time this doing 
		slnTime.vlayerDone[p.dim] = myClock::now(); auto bfEndTime_t = time(NULL);
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
			, memUse) << "\n";
		slnLog.close();
		//----------BF----DONE----------------------------------/
		//============REPORTS,==RECOVERY,==ETC.=================/
		//auto wat = iminmin(full, layer[p.dim - 1], p, D);
		slnLog.open(logName, std::ios_base::app);
		slnLog << "\n" << "BF DONE: " << mkTimeStamp(bfEndTime_t) << "\n" <<
			"TOTAL DURATION: "
			<< mkMsecDurationFull(slnTime._rel_time(0, p.dim)) << "\n"
			<< "TOTAL DURATION IN SECONDS: "
			<< mkMsecDurationBrief(slnTime._rel_time(0, p.dim)) << "\n"
			<< "\n" << "RAM USAGE AT LAST LAYER: " << to_stringBytes(slnMem.back().physMem)
			<< "\n" << "VM USAGE AT LAST LAYER: " << to_stringBytes(slnMem.back().virtMem);
		slnLog.close();
////////////////////////////////////////////////////////////////////////////////////////////////////////
		//recover the solution
		this->result = recoverSln(full,layer, p, D);//can now destroy this->layer
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
//////////////////////////////////////////////////////////////////////////////////////////////////////
		//====================FINAL==REPORT(DUMP)==================/
		//NB:relies on result being written to this->result before 
		slnDump.open(dumpName); slnDump << mkFullDump(p,result,D);
		//slnDump << std::endl << mkRtDump(p, result);
		slnDump.close();

		return this->result;
	}

};
//----------------------------------------------------------------------------/


#endif

