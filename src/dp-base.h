/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

dp-base v.1.0 2017-01-11;
dp-base v.1.1 2018-02-27: added total lexicographic v->x->K order on states

(G)TSP-PC dynamic programming structures & auxiliaries,
the Bellman Function (minminRt)


*/

/*
my previous Restricted DP used ORDERED maps, for more consistent memory footprint.

however, unless I want to write a comparison function for bitset>64-bit, I'll stick
to hashes---unordered_map

*/

#ifndef DPBASE_H_
#define DPBASE_H_

#include "bitset-base.h"//for t_bin
#include "prec-ord.h"//for getMin, getMax, t_Direction
#include "reader-base.h"//for t_cost & to_stringHex
#include "instance.h"
#include <map>
#include <unordered_map>
#include<cstdint>
#include "parallel_hashmap/phmap.h"

//inline t_bin getMin(const t_bin& K, const t_vprecDsc& P, const t_binOrdWK& Pwk)


//a literal state representation v(x,K), for comparing the states by cost &c.
using t_stateDesc
= struct t_stateDesc
{//OUGHT TO BE const; CONST AREN'T ASSIGNABLE --> WON'T BE CONTAINED by vectors,etc.;
	//K, the task set
	t_bin K;
	//`interface'; base for filters, `last visited'/terminal for ideals
	ptag x; 
	t_cost v;//v(x,K), the state's cost

	t_stateDesc()//especially when irrelevant
		: K(EmptySet)
		, x(0)
		, v(0) {}

	t_stateDesc(t_bin _K, ptag _x, t_cost _v)
		: K(_K)
		, x(_x)
		, v(_v) {}
	
	//states are compared costwise; assumed to be of same rank, i.e., K.size()
	inline bool operator <(const t_stateDesc& other) const
	{
		//return (this->v < other.v);
		return(this->less_totalOrd(other));
	}
	
	inline bool operator ==(const t_stateDesc& other) const
	//just check if all the data fields match
	{
		return (this->v == other.v
			&& this->K == other.K
			&& this->x == other.x);
	}
	bool operator >(const t_stateDesc& other) const
		//delegated through  <
	{
		return (!(this->operator==(other)) 
			&& !(this->operator<(other)) );
		//return (this->v > other.v);
	}

		/*lexicographic, v -> x -> K; "safe" with respect to "natural" state order,
		defined by the underlying data structure that stores the states and their values;
		right now, hash table*/
	inline bool less_totalOrd(const t_stateDesc& other) const
	{
		if ((this->v < other.v)
			|| (this->v == other.v && this->x < other.x)
			|| (this->v == other.v && other.x == this->x && less_t_bin(this->K, other.K)))
			return true;
		else
			return false;
	}
		
	//write (x,HEX_K,v)
	std::string to_string() const
	{
		std::ostringstream tmp;
		tmp << '(' << x << ',' << to_stringHex(K) << ',' << v << ')';
		return tmp.str();
	}
};

/*lexicographic, v -> x -> K; "safe" with respect to "natural" state order,
defined by the underlying data structure that stores the states and their values;
right now, hash table*/
inline bool less_totalOrd(const t_stateDesc& left, const t_stateDesc& right) 
{
	if ((left.v < right.v)
		|| (left.v == right.v && left.x < right.x)
		|| (left.v == right.v && right.x == left.x && less_t_bin(left.K, right.K)))
		return true;
	else
		return false;
}

//==========BF==STRUCTURE===TYPES=======================/
/*
STD::MAP as the principal type, with my homebrew bitset lexicographical comparison
using t_stLayer = std::map < t_bin, t_tsCvdInfo, t_binComp >;

preliminary data: at most 5% memory use improvement,
several times speed loss: 
ry48p.3.sop-FWD-hRtDP-100000
layer 6 took         1 second (unordered_map)
layer 6 took BLOODY 66 seconds (map)

it may be me (bitset-base.h::less_t_bin())
or it may be the logarithm, but it sure ain't worth it,
I wouldn't even get close to checking if it saves memory for large problems due to sheer loss in speed.
dixi.
*/


//using t_tsCvdInfo = std::map < ptag, t_cost,std::less<ptag> >;
//using t_tsCvdInfo = std::map < ptag, t_cost >;
//using t_tsCvdInfo = std::unordered_map < ptag, t_cost >;
using t_tsCvdInfo = phmap::flat_hash_map < ptag, t_cost >;
//using t_tsCvdInfo = phmap::node_hash_map < ptag, t_cost >;
//all states of a layer; maps a TASKSET:t_bin to its states \w cost (t_tsCvdInfo)
//using t_stLayer = std::unordered_map < t_bin, t_tsCvdInfo >;
using t_stLayer = phmap::flat_hash_map < t_bin, t_tsCvdInfo >;
//using t_stLayer = phmap::parallel_flat_hash_map < t_bin, t_tsCvdInfo >;
//using t_stLayer = phmap::node_hash_map < t_bin, t_tsCvdInfo >;
//using t_stLayer = phmap::parallel_node_hash_map < t_bin, t_tsCvdInfo >;
//test: with custom hash bitset-base.h::t_binHash (plain by-word XOR)
//using t_stLayer = std::unordered_map < t_bin, t_tsCvdInfo,t_binHash >;
//principal structure: the vectory of layers, 0..dim
using t_vstLayer = std::vector < t_stLayer >;
//--------------------------------------------------------/

//==========BF==Functions==============================/

//solve the BF, v(i,T)=\min_{m\in \Min[T]}\min_{pt\in M_m^{out}}(step+previous) for p
inline t_cost minmin(const ptag x
	, const t_bin& K //x:interface-base, K:taskset-filter
	, const t_stLayer& prevL //won't affect layer[...] directly
	, const t_Instance& p //for order, functions, etc.
	, const t_Direction& D)//forward or backwards
{
	t_cost wpre = INF;
	//where could we go from x? (BWD)/ where from could we go to x (FWD)
	t_bin intfK = D.gII(K, p.ord, p.wkOrd);
	foreach_elt(m, intfK, p.dim)
	{
		t_cost wpreio = INF;
		t_bin tryPrec = (K ^ (BIT0 << m));//say we went x->-m->-K\setminus\{m\}; x-->m_pin-->m_pout-->K\setminus\{m\}
		//get v(m_pout, K\setminus\{m\}) if it was retained; try for all (m_pin, m_pout)
		//if we've actually been there before
		if (prevL.find(tryPrec) != prevL.end())
		{
			foreach_point(mpin, p.popInfo[m].pfirst, p.popInfo[m].plast)
				//for each state retained (let's see which interfaces--exit points were retained
			{//precState.first is an exit point pt\in M_m^{out}; .second is v(pt,tryPrec), its cost
				foreach_point(mpout, p.popInfo[m].pfirst, p.popInfo[m].plast)
				{
					if (prevL.at(tryPrec).find(mpout) != prevL.at(tryPrec).cend())//if (mpout, K\setminus\{m\}) was retained
						wpreio = std::min(wpreio
											, p.f.cAgr(p.f.cTotalD(D,x, mpin, mpout, tryPrec, p.cost)
														, prevL.at(tryPrec).at(mpout)
														) );
				}//next pred.state; next pred.point (precState.first)
			}//next entry point into m
			wpre = std::min(wpreio, wpre);//retain the better version
		}//end if tryPrec found
	}//next m\in MinK
	return wpre;
}

#endif
