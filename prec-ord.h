/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

prec-ord v.1.0 2017-01-09
prec-ord v.1.1 2019-02-25: added relation composition on t_vprecDsc and explicit null-init to t_precDsc
prec-ord v.1.1.1 2019-03-06: added relation difference on t_vprecDsc

order descriptions & precedence constraints
*/

#ifndef ORDPREC_H_
#define ORDPREC_H_

#include"bitset-base.h"

#include<iostream>//for cerr<<
//one bitset for successors, one bitset for predecessors
using t_precDsc =
struct t_precDsc
{//std::pair<t_bin, t_bin> anyone?
	t_bin receives_from{ 0 }; //x.receives_from = all LESS than x, almost principal ideal
	t_bin sends_to{ 0 };//x.sends_to = all GT than x, almost principal filter
};

using t_vprecDsc = std::vector < t_precDsc >;

//using t_mxPrec = std::vector< std::vector< int16_t> >;

//set bits 0..dim+1; 0 is base, dim+1 is terminal
t_bin mkMask(const uint16_t dim);

//set bits 1..dim; proper cities only
t_bin mkOmask(const uint16_t dim);

//base and terminal are not considered
//this->first=Receivers (Max, non-isolated); this->second=Senders (Min, non-isolated)
std::pair<t_bin, t_bin> getRcvSnd(const t_vprecDsc& input);

//Isolated: totally incomparable (always belong to both Min and Max)
t_bin getIsld(const t_vprecDsc& input);

t_bin getNonRcvs(const t_vprecDsc& input);

t_bin getNonSnds(const t_vprecDsc& input);

//'well-known' precedences
//mask, nonsenders, nonreceivers, etc;
using t_binOrdWK =
struct t_binOrdWK
{
	const mtag dim;//the number of proper cities
	const t_bin mask, omask, isld, snds, rcvs, nonSnds, nonRcvs;
	/*E for all 0..trm = 0..dim+1; Islt:isolated, totally incomparable
	mask= E\cup\{0\}\cup\{\trm\}; all clusters including base=0 and terminal=dim+1
	omask= E; all proper clusters
	isld= isolated; previously, `precfree' (totally incomparable)
	nonsndrs are Max; (yes, Islt too)
	nonrcvrs are Min; (yes, Islt too)
	sndrs are E\setminus Max; (Max contains Islt)
	rcvrs are E\setminus Min; (Min contains Islt)
	*/
	t_binOrdWK(const t_vprecDsc& input)
		: rcvs(getRcvSnd(input).first)
		, snds(getRcvSnd(input).second)
		, mask(mkMask(input.size() - 2))
		, omask(mkOmask(input.size() - 2))
		, isld(getIsld(input))
		, nonRcvs(getNonRcvs(input))
		, nonSnds(getNonSnds(input))
		, dim(input.size() - 2)
	{}
};

//===========RELATIONS:==COMPOSITION==AND==SETMINUS=================/
//assume L[0],R[0],L[size],R[size], size=dim+1, are special base/terminal; do not consider them

//assume L[0],R[0],L[size],R[size], size=dim+1, are special base/terminal; do not consider them
/*relation composition; base L[0] and terminal L[size()-1] are ignored*/
inline t_vprecDsc compose(const t_vprecDsc& L, const t_vprecDsc& R)
{
	if (L.size() != R.size())
	{
		std::cerr << "\nERROR: Non-square relation composition: " << L.size() << " ; " << R.size() << "\nTerminating.";
		std::exit(EXIT_FAILURE);
	}
	t_vprecDsc out (L.size());//the new one is the same size
	for (auto i = 1; i < L.size()-1; i++) //i=L.size()-1 is terminal, let's just forget about it
	{
		//if there's at least one (a,i)\in L and (i,c)\in R, add all (a,c) to out
		if (t_precDsc newPairs{ L[i].receives_from,R[i].sends_to };
			newPairs.receives_from.any() && newPairs.sends_to.any())
		{
			//add all c's as "above" this a
			foreach_elt(a, newPairs.receives_from, L.size()) out[a].sends_to |= newPairs.sends_to;
			//add all a's as "below" this c
			foreach_elt(c, newPairs.sends_to, L.size()) out[c].receives_from |= newPairs.receives_from;
		}
	}
	return out;
}

/*relation set difference (as for sets of pairs)*/
inline t_vprecDsc relSetMinus(const t_vprecDsc& L, const t_vprecDsc& R)
{
	if (L.size() != R.size())
	{
		std::cerr << "\nERROR: Non-square relation difference: " << L.size() << " ; " << R.size() << "\nTerminating.";
		std::exit(EXIT_FAILURE);
	}
	t_vprecDsc out (L.size());//the new one is the same size
	for (auto i = 0; i < out.size();i++)//for each city in L (works transparently over 0 and last item
	{
		out[i].sends_to = setMinus(L[i].sends_to, R[i].sends_to);//remove R's pairs from "L" as <_P; (principal ideals)
		out[i].receives_from = setMinus(L[i].receives_from, R[i].receives_from); //do it again, for "L" as >_P (principal filters)
	}
	return out;
}
//========MIN===AND===MAX======================================/
//get the set of minimal elements of K with respect to P; 
inline t_bin getMin(const t_bin& K, const t_vprecDsc& P, const t_binOrdWK& Pwk)
{
	t_bin out = K;
	t_bin TS_snds = K&Pwk.snds;
	//for each $sndr in $K
	for (auto sndr = ssb(TS_snds, 0, Pwk.dim); sndr <= Pwk.dim; sndr = ssb(TS_snds&out, sndr, Pwk.dim))
		//erase its receivers --- those that are greater
		ssetMinus(out, P[sndr].sends_to);
	return out;
}
//get the set of min. els of K's complement, Min[1..dim\setminus K]
inline t_bin get_coMin(const t_bin& K, const t_vprecDsc& P, const t_binOrdWK& Pwk)
{
	return(getMin( setMinus(Pwk.omask, K),P,Pwk));
}

//get the set of maximal elements of K with respect to P; 
inline t_bin getMax(const t_bin& K, const t_vprecDsc& P, const t_binOrdWK& Pwk)
{
	t_bin out = K;
	t_bin TS_rcvs = K&Pwk.rcvs;
	//foreach_elt(rcvr,TS_rcvs,Pwk.dim)
	for (auto rcvr = ssb(TS_rcvs, 0, Pwk.dim); rcvr <= Pwk.dim; rcvr = ssb(TS_rcvs&out, rcvr, Pwk.dim))
		ssetMinus(out, P[rcvr].receives_from);
	return out;
}

//get the set of max. els of K's complement, Max[1..dim\setminus K]
inline t_bin get_coMax(const t_bin& K, const t_vprecDsc& P, const t_binOrdWK& Pwk)
{
	return(getMax(setMinus(Pwk.omask, K), P, Pwk));
}

/*
choose whether to generate ideals (forward) or filters (backward);
backward: getIntf_into=getMin; getExp(K)=getMax(1..dim\setminus K)
----(x-->K) for minimal m\in K;-BASE->-UNDONE-----y-->(x,K)--->TERMINAL
forward: getIntf_into=getMax; getExp(K)=getMin(1..dim\setminus K)
-----(K-->x) for max m\in K;---BASE-->(K,x)--->UNDONE-->--TERMINAL
*/
using t_Direction =
struct t_Direction
{//\mathbb{I} is traditional ~getFEP, feasible entry (BWD)/ exit (FWD) points
	t_bin(*gII) (const t_bin& K, const t_vprecDsc& P, const t_binOrdWK& Pwk);
	//\mathbf{E} for Expansion; 
	t_bin(*gE) (const t_bin& K, const t_vprecDsc& P, const t_binOrdWK& Pwk);
	bool operator==(const t_Direction other) const
	{
		if (this->gE == other.gE
			&& this->gII == other.gII)
			return true;
		else
			return false;
	}
	// bool operator !=(const t_Direction other) const //delegate to NOT == 
};

const t_Direction BWD = t_Direction{ getMin, get_coMax };
const t_Direction FWD = t_Direction{ getMax, get_coMin };

inline std::string getDirectionCode(const t_Direction input)
{
	if (input == BWD)
		return("BWD");
	else if (input == FWD)
		return("FWD");
	else return ("UFO");//say, Unrecognized Function Object
}

inline t_Direction getDirectionByCode(const std::string& dCode)
{
	if (dCode == "BWD")
		return(BWD);
	else if (dCode == "FWD")
		return(FWD);
	else
	{ 
		std::cerr << "Invalid direction code: " << dCode << "\nTerminating.\n";
		exit(EXIT_FAILURE);
	}
}

#endif
