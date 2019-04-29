/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

instance v.1.0 2017-01-09;
instance v.1.1 2017-01-16: functions gathered into a kit

(G)TSP-PC Instance & auxiliaries;

*/

#ifndef INSTANCE_H_
#define INSTANCE_H_

#include "function-kits.h"


using t_cityPopDsc =
struct t_cityPopDsc
{
	ptag pfirst, plast;
	ptag pop(void) const
	{
		return (plast - pfirst + 1);
	}
};

using t_vcityPopDsc = std::vector < t_cityPopDsc >;

//=============AUXILIARIES================/
inline std::vector<mtag> mkCityOfDefault(const mtag dim)
{
	std::vector<mtag> out;	out.resize(dim + 2);//0,1..dim,dim+1; size=dim+2
	for (mtag i = 0; i < out.size(); i++) out[i] = i; //just this point
	return out;
}

inline t_vcityPopDsc mkPopInfoDefault(const mtag dim)
{
	t_vcityPopDsc out; out.resize(dim + 2);
	for (int i = 0; i < out.size(); i++) { out[i].pfirst = i; out[i].plast = i; }
	return out;
}
//-----------------------------------------/

//=================INSTANCE========================/
//quite polymorphic (G)TSP-PC instance; user-specified agregation functions &c
using t_Instance =
struct t_Instance
{
	t_lines instDesc;//problem description
	const std::string instName;//just the name
	//the cost matrix
	t_cstMx cost;
	//the number of PROPER clusters, not counting base=0 and trm=dim+1
	const std::uint16_t dim;
	//order as [i]->Pred(less-than)List, [i]->Succ(gt-than)List for each clst [i]
	const t_vprecDsc ord;
	const t_binOrdWK wkOrd;
	//****cluster description:*****
	const std::vector<mtag> cityof;//cityof[point]=its city
	//clusters' population and point numbers per city through pfirst..plast
	const t_vcityPopDsc popInfo;
	
	//the functions kit: aggregation (interstep, Int+Ext, intJ, extM, &c)
	const t_fAgr f; //definitely won't wary during Instance's lifetime
	/******CONSTRUCTORS****************/
	//%%%%%TSPLIB-SOP:1-city clusters, plain TSP-PC
	t_Instance(const t_lines& dsc, const std::string& _instName,
			   const t_cstMx& inCst, const t_vprecDsc& inOrd)
			   : instDesc(dsc), instName(_instName)
			   , dim(inCst.size() - 2)
			   , cost(inCst)
			   , cityof(mkCityOfDefault(dim))
			   , popInfo(mkPopInfoDefault(dim))
			   , ord(inOrd)
			   , wkOrd(inOrd)
			   , f(TSP)//default to TSP
			   //, f(TD_TSP)
	{}
	t_Instance(const t_lines& dsc, const std::string& _instName, const t_fAgr& problemType,
			   const t_cstMx& inCst, const t_vprecDsc& inOrd)
			   : instDesc(dsc)
			   , instName(_instName+"-"+getProblemTypeCode(problemType))
			   , dim(inCst.size() - 2)
			   , cost(inCst)
			   , cityof(mkCityOfDefault(dim))
			   , popInfo(mkPopInfoDefault(dim))
			   , ord(inOrd)
			   , wkOrd(inOrd)
			   , f(problemType)//TSP, TD-TSP, BTSP, GTSP, etc.
			   //, f(TD_TSP)
	{
	}
};
/* Class Synopsis:
stores the COST (matrix; might swap for a function if&when)
--"-- DIMENSION --- the number of proper clusters (clusters by default);
--"-- POPULATION --- how many cities in each cluster
non-clustered goes as cluster population=1; default to non-clustered
in each cluster, cities are numbered consecutively (unlike Zverevitch's G-TSPLIB);
will implement renumbering as soon as I care
==in t_fAgr
--"-- COST AGGREGATION --- movement aggregation function
--"-- INT&EXT AGGREGATION --- default to +
--"-- EXTERIOR MOVEMENT --- how are exterior movements priced; default to cost[exit_a][entry_b]
--"-- INTERIOR JOBS --- how are interior jobs priced; default to cost[city_1][city_2]
//--"-- IO_PAIRS --- feasible entry-exit pairs for each megapolis (list); not implemented
NB! no way to swap min/max optimization (yet)
*/
#endif


//macro for iterating $p through each number from $pfirst to $plast
//#define foreach_point(p,pfirst,plast) for (ptag p=pfirst; p<=plast;p++)