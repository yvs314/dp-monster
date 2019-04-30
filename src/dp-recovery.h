/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

dp-recovery v.1.0 2017-01-16;

(G)TSP-PC DP solution recovery (from BF) & output;

*/



#ifndef DPREC_H_
#define DPREC_H_

#include "dp-base.h"
#include "reader-base.h"
#include <deque>

//=======ROUTE===RECOVERY===DATA==STRUCTURE=================/
using t_rstepDesc =
struct t_rstepDesc
{//s=(x,K)-->mpin-->mpout-->K'; ns=(y,K'), y\in M_m, m\in\Min[K], K'=K\setminus\{m\}
	t_stateDesc s;//this state
	ptag mpin, mpout;//in m-th cluster
	t_cost s2ncst;//cost of going ns-->s: cTotal(x, mpin, mpout,K')
	t_stateDesc ns;//next state =(y,K\setminus\{m\})
	t_rstepDesc(const t_stateDesc is, const ptag impin, const ptag impout,
		const t_cost is2ncst, const t_stateDesc ins)
		: s(is), mpin(impin), mpout(impout), s2ncst(is2ncst), ns(ins){	};
	t_rstepDesc() : s({ 0, 0, INF }), mpin(0), mpout(0), s2ncst(INF), ns({ 0, 0, INF }) {};
	std::string mkDump() const
	{
		std::ostringstream acc; std::string csp = ";";//colSep
		acc << s.to_string() << csp << mpin << csp << mpout << csp << s2ncst << csp << ns.to_string();
		return acc.str();
	}
};
using t_solution = std::deque<t_rstepDesc>;

//----------------------------------------------------/



//==========SOLUTION==RECOVERY=====================/
/*given a state s=(x,K) and v(x,K), find a covered s' that achieves the minmin in (BF)(s),
that is, m\in\Min[K], K'=K\setminus\{m\}, and (mpin,mpout)
such that v(x,K)=cTotal(x,mpin,mpout,K'); that is, s.v=minminRt(x,K,prevL) x->mpin->mpout->K'
*/
t_rstepDesc iminmin(const t_stateDesc s
					, const t_stLayer& prevL
					, const t_Instance& p //for order, functions, etc.
					, const t_Direction& D);//forward or backwards

t_solution recoverSln(const t_stateDesc& full
					  ,const t_vstLayer& L //the whole Bellman function
					  , const t_Instance& p //for dimension and precedences, &c
					  , const t_Direction& D);

//-----------------------------------------------/

//=========SOLUTION==OUTPUT=======================/
//intersperse with \n and 
std::string mkSlnDump(const t_solution& inp);

std::string mkRtDump(const t_Instance& p, const t_solution& inp, const t_Direction& D);

std::string mkValDump(const t_solution& inp);

//combines Value \n Route \n Dump
std::string mkFullDump(const t_Instance& p, const t_solution& sln, const t_Direction& D);
//------------------------------------------------/

#endif // !DPREC_H_
