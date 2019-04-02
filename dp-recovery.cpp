/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

dp-recovery v.1.0 2017-01-16;

(G)TSP-PC DP solution recovery (from BF) & auxiliaries;

*/

#include"dp-recovery.h"

//==========SOLUTION==RECOVERY=====================/
/*given a state s=(x,K) and v(x,K), find a covered s' that achieves the minmin in (BF)(s),
that is, m\in\Min[K], K'=K\setminus\{m\}, and (mpin,mpout)
such that v(x,K)=cTotal(x,mpin,mpout,K'); that is, s.v=minminRt(x,K,prevL) x->mpin->mpout->K'
*/
t_rstepDesc iminmin(const t_stateDesc s
	, const t_stLayer& prevL
	, const t_Instance& p //for order, functions, etc.
	, const t_Direction& D)//forward or backwards
{
	t_rstepDesc dummy{ { 0, 0, INF }, 0, 0, INF, { 0, 0, INF } };//to signal an ERROR
	t_bin intfK = D.gII(s.K, p.ord, p.wkOrd);
	foreach_elt(m, intfK, p.dim)
	{
		t_cost wpreio = INF;
		t_bin Kprime = (s.K ^ (BIT0 << m));//say we went x->-m->-K\setminus\{m\}; x-->m_pin-->m_pout-->K\setminus\{m\}
		//get v(m_pout, K\setminus\{m\}) if it was retained; try for all (m_pin, m_pout)
		//if we've actually been there before
		if (prevL.find(Kprime) != prevL.end())
		{
			foreach_point(mpin, p.popInfo[m].pfirst, p.popInfo[m].plast)
				//for each state retained (let's see which interfaces--exit points were retained
			{//precState.first is an exit point pt\in M_m^{out}; .second is v(pt,Kprime), its cost
				foreach_point(mpout, p.popInfo[m].pfirst, p.popInfo[m].plast)
				{
					//if (mpout, Kprime=K\setminus\{m\}) was retained
					if (prevL.at(Kprime).find(mpout) != prevL.at(Kprime).cend())
						//AND it achieves the given s.v
						if (s.v == p.f.cAgr(p.f.cTotalD(D,s.x, mpin, mpout, Kprime, p.cost)
							, prevL.at(Kprime).at(mpout)))
						{
							return(t_rstepDesc{
								s	 //calling state
								, mpin //mpin
								, mpout//mpout
								, p.f.cTotalD(D,s.x, mpin, mpout, Kprime, p.cost)
								, { Kprime, mpout, prevL.at(Kprime).at(mpout) }//next state
							});
						}
				}//next mpout
			}//next mpin
		}//end if tryPrec found
	}//next m\in intfK
	return dummy;
}

t_solution recoverSln(const t_stateDesc& full
					  , const t_vstLayer& L //the whole Bellman function
					  , const t_Instance& p //for dimension and precedences, &c
					  , const t_Direction& D) //for iminminRt
{
	t_solution out;


	//t_stateDesc full{ L.at(p.dim).begin()->first
	//	, 0
	//	, L[p.dim].at(p.wkOrd.omask).begin()->second };
	auto wat = iminmin(full, L.at(p.dim - 1),p,D);
	auto s = full; auto l = p.dim;//start with the complete problem
	do
	{
		t_rstepDesc tmp = iminmin(s, L.at(l - 1),p,D);
		out.push_back(tmp);
		s = tmp.ns; l--;//one step back
	} while (l > 0);
	//now, s=(x,\varnothing) is down to the initial condition;
	/* construct the termination:
	 BWD: ns=(\trm,\varnothing); v(ns)=0 by definition
	 FWD: ns=(base,\varnothing); v(ns)=0 by definition
	*/
	ptag lastPt = (D == BWD) ? (p.dim + 1) : (0);//<BWD:\trm,FWD:base>
	t_stateDesc ns{ t_bin(0), lastPt, 0 };
	t_rstepDesc to_trm{ s, lastPt, lastPt, s.v, ns };
	out.push_back(to_trm);
	return out;
}

//------------------------------------------------------/


//=========SOLUTION==OUTPUT=======================/
std::string mkSlnDump(const t_solution& inp)
{
	std::ostringstream acc; std::string ln = "\n";
	static std::string dumpHeader = "(BASE,TSET,COST);MPIN;MPOUT;STEP_COST;(N_BASE,N_TSET,N_COST)";
	acc << dumpHeader << ln;
	for (auto step : inp)
		acc << step.mkDump() << ln;
	
	return acc.str();
}

//instance required for matching points to cities
std::string mkRtDump(const t_Instance& p, const t_solution& inp, const t_Direction& D)
{
	std::ostringstream acc; std::string sep = ",";
	static const std::string dumpHeader = "ROUTE:[0,city_1,city_2,\\ldots,city_dim,TERMINAL]";
	acc << dumpHeader << std::endl; acc << "[";
	
	/*it'd have been more readable to just reverse the BWD output*/
	if (D == BWD)
	{
		for (auto it = inp.cbegin(); it < inp.cend(); it++)
		{
			acc << p.cityof[it->s.x] << sep;
		}
		acc << (inp.cend() - 1)->ns.x << "]";
	}
	else
	{
		acc << (inp.crbegin())->ns.x ;
		for (auto it = inp.crbegin(); it < inp.crend(); it++)
		{
			acc << sep << p.cityof[it->s.x];
		}
		acc << "]";
	}
	
	/*for (auto step : inp)
		acc << p.cityof[step.s.x] << sep;*/
	//acc << (inp.cend() - 1)->ns.x << "]";
	
	return acc.str();
}

std::string mkValDump(const t_solution& inp)
{
	static std::string dumpHeader = "VALUE:";
	//value extracted as the cost of the top state
	return(dumpHeader + std::to_string(inp.front().s.v));
}

std::string mkFullDump(const t_Instance& p, const t_solution& sln, const t_Direction& D)
{
	std::string ln = "\n";
	return (mkValDump(sln) + ln +ln+  mkRtDump(p, sln,D) +ln+ ln + mkSlnDump(sln));
}

//-----------------------------------------------------/