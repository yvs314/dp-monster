/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

function-kits v.1.0 2017-01-09;
function-kits v.1.1 2017-01-16: gathered the kit into a struct
function-kits v.1.2 2017-03-22: KCvH/Abeledo time dependence, both FWD and BWD
function-kits v.1.3 2018-02-11: tachyon traveling deliveryman---for reviewer's sake

various aggregation functions &c, for use with instances



*/

#ifndef FKITS_H_
#define FKITS_H_



#include"reader-base.h"//for t_cost
#include"prec-ord.h"//for t_Direction, etc.

//***TSP****
inline t_cost plusAgr(const t_cost left, const t_cost right)
{
	return left + right;
}
//***BTSP***
inline t_cost maxAgr(const t_cost left, const t_cost right)
{
	return std::max(left, right);
}

inline t_cost plusIEAgr(const t_cost extm, const t_cost intj)
{
	return extm + intj;
}

//for non-clustered: no interior jobs at all
inline t_cost dummyIEAgr(const t_cost extm, const t_cost intj)
{
	return extm;
}

inline t_cost baseExtMt(const ptag this_pout, const ptag next_pin, const t_bin taskset, const t_cstMx& cst)
{
	return cst[this_pout][next_pin];
}

/*the Kinable--Cire--van Hoeve (with regards to Abeledo) cost; BWD only (no need for dimension)!!!
also, it kills (\times 0) the initial conditions (distance to terminal): TSPLIB only
*/
inline t_cost timeDepExtMtBWD(const ptag this_pout, const ptag next_pin, const t_bin taskset, const t_cstMx& cst)
{//NB! this is BWD: doesn't have to know problem size
	//BWD:return (taskset.count()+1)*cst[this_pout][next_pin];
	//FWD: n+1-t; return (16 - taskset.count())*cst[this_pout][next_pin];
	//return (taskset.count() + 1)*cst[this_pout][next_pin];
	
	return (next_pin==cst.size()-1)
		?(cst[this_pout][next_pin])
		:(taskset.count() + 2)*cst[this_pout][next_pin];
}

inline t_cost timeDepExtMtFWD(const ptag this_pout, const ptag next_pin, const t_bin taskset, const t_cstMx& cst)
{//NB! this is FWD: must know problem size
	const auto dim = cst.size()-2;//say n=42; terminal is 43-d, base is 0
	return (this_pout == 0) 
		? ((dim + 1 - taskset.count())*cst[this_pout][next_pin]) 
		: ((dim - taskset.count())*cst[this_pout][next_pin]);
	//return (next_pin == cst.size() - 1) ? (cst[this_pout][next_pin]) : (taskset.count() + 2)*cst[this_pout][next_pin];
}

/*the ``tachyon'' traveling deliveryman (cf. KCvH/Abeledo costs).
Because the reviewer asked to verify if backward hRtDP could indeed be better
than forward were the cost multipliers **reversed**

upd: it works!
*/

inline t_cost tachTimeDepExtMtBWD(const ptag this_pout, const ptag next_pin, const t_bin taskset, const t_cstMx& cst)
{//NB! this is BWD: must know problem size
 //BWD:return (taskset.count()+1)*cst[this_pout][next_pin];
 //return (taskset.count() + 1)*cst[this_pout][next_pin];
	const auto dim = cst.size() - 2; //size is how many(0,1,\ldots,dim,dim+1=\trm)

	return (next_pin == cst.size() - 1)
		? ((dim+1)*cst[this_pout][next_pin])
		: (dim-taskset.count() )*cst[this_pout][next_pin];
}

inline t_cost tachTimeDepExtMtFWD(const ptag this_pout, const ptag next_pin, const t_bin taskset, const t_cstMx& cst)
{//NB! this is FWD: doesn't need to know problem size
	const auto dim = cst.size() - 2;//say n=42; terminal is 43-d, base is 0
	return (this_pout == 0)
		? (cst[this_pout][next_pin])
		: ((taskset.count() +2)*cst[this_pout][next_pin]);
	//return (next_pin == cst.size() - 1) ? (cst[this_pout][next_pin]) : (taskset.count() + 2)*cst[this_pout][next_pin];
}

inline t_cost baseIntJ(const ptag this_pin, const ptag this_pout, const t_bin taskset, const t_cstMx& cst)
{
	return cst[this_pin][this_pout];
}

//for non-clustered problems
inline t_cost dummyIntJ(const ptag this_pin, const ptag this_pout, const t_bin taskset, const t_cstMx& cst)
{
	return 0;
}

//======COST==FUNCTION==KIT=========================================/
using t_fAgr =
struct t_fAgr
{	//COST AGGREGATION; called from the BF procedure (i.e., dp-base.h::minmin)
	t_cost(*cAgr)(const t_cost left, const t_cost right); 
	
	//INT&EXT AGG; default to intJob+extMt
	t_cost(*cIEAgr)(const t_cost extm, const t_cost intj) ;
	
	/*EXT.MVT; BWD: from x_point_out to TS_point_in; default to cst[x][TS_pin]
			   FWD: from TS_point_out to x_point_in; should do to cst[TS_pout][x]
	*/
	t_cost(*cExtMt)(const ptag x_pout, const ptag TS_pin, const t_bin taskset, const t_cstMx& cst);
	
	/*INT.JBS; BWD&FWD: from TS_point_in to TS_point_out; default to cst[TS_pin][TS_pout]*/
	t_cost(*cIntJ)(const ptag TS_pin, const ptag TS_pout, const t_bin taskset, const t_cstMx& cst);
	
	
	//total cost, “extM+intJ”;default to the BWD version, x->TS_pin->TS_pout
	inline t_cost cTotal(const ptag x_pout, //the interface, x
						 const ptag TS_pin, //into neighbouring cluster, BWD:m^{in}, FWD:m^{out}
						 const ptag TS_pout,//out of neighbouring cluster, BWD:m^{out}, FWD:m^{in}
						 const t_bin& taskset,//K
						 const t_cstMx& cst) const
	{
		return(cIEAgr(cExtMt(x_pout, TS_pin, taskset, cst)
			, cIntJ(TS_pin, TS_pout, taskset, cst)));
	}

	
	inline t_cost cTotalBWD(const ptag x_pout, //the interface, x
		const ptag TS_pin, //into neighbouring cluster, BWD:m^{in}, FWD:m^{out}
		const ptag TS_pout,//out of neighbouring cluster, BWD:m^{out}, FWD:m^{in}
		const t_bin& taskset,//K
		const t_cstMx& cst) const
	{
		return(cIEAgr(cExtMt(x_pout, TS_pin, taskset, cst)
			, cIntJ(TS_pin, TS_pout, taskset, cst)));
	}
	
	//FWD:swap the ExtMt: take (TS_pout,x) instead of (x,TS_pin); i.e., TS_pin->TS_pout->x
	inline t_cost cTotalFWD(const ptag x_pin, //the interface, x
		const ptag TS_pout, //into neighbouring cluster, 
		const ptag TS_pin,//out of neighbouring cluster, 
		const t_bin& taskset,//K
		const t_cstMx& cst) const
	{
		return(cIEAgr(cExtMt(TS_pout, x_pin, taskset, cst)
			, cIntJ(TS_pin, TS_pout, taskset, cst)));
	}

	/*POLYMORPH: test the direction and return the appropriate cost
	IF SLOW: replace with non-testing versions; hardwire direction into minmin & iminmin or something
	*/
	inline t_cost cTotalD(const t_Direction& D
		, const ptag x_pout //the interface, x
		, const ptag TS_pin //into neighbouring cluster, BWD:m^{in}, FWD:m^{out}
		, const ptag TS_pout//out of neighbouring cluster, BWD:m^{out}, FWD:m^{in}
		, const t_bin& taskset//K
		, const t_cstMx& cst) const
	{
		if (D == BWD)
			return cTotalBWD(x_pout, TS_pin, TS_pout, taskset, cst);
		else
			return cTotalFWD(x_pout, TS_pin, TS_pout, taskset, cst);
	}

	bool operator==(const t_fAgr other) const
	{
		if (this->cAgr == other.cAgr
			&& this->cExtMt == other.cExtMt
			&& this->cIEAgr == other.cIEAgr
			&& this->cIntJ == other.cIntJ)
			return true;
		else
			return false;
	}
};
/*<BWD;FWD>
//K:taskset(TS)<filter;ideal>, x:interface<base,terminal>;<out:in>
COST AGGREGATION: interstep, cost(a,b) `cAgr` cost(b,c) = cost(a,b,c)
INT&EXT AGGREGATION: intrastep, -prev_pout->--extM->-pin--intJ-->--pout
EXTERIOR MOVEMENT: default to cost[exit_a][entry_b]
INTERIOR JOBS: default to cost[local_city_1][local_city_2]
TOTAL COST: IEAgr stuffed with ExtMt and IntJ
*/


const t_fAgr TSP{ plusAgr, dummyIEAgr, baseExtMt, dummyIntJ };
const t_fAgr GTSP{ plusAgr, plusIEAgr, baseExtMt, baseIntJ };
const t_fAgr BTSP{ maxAgr, dummyIEAgr, baseExtMt, dummyIntJ };
const t_fAgr BGTSP{ maxAgr, plusIEAgr, baseExtMt, baseIntJ };

/*
Kinable--Cire--van Hoeve/Abeledo time-dependent cost; 
non-transparent by direction because diferent ExtMt functions must be
supplied; could by rectified by passing direction to ExtMt through
*/
const t_fAgr TD_TSPb{ plusAgr, dummyIEAgr, timeDepExtMtBWD, dummyIntJ };
const t_fAgr TD_TSPf{ plusAgr, dummyIEAgr, timeDepExtMtFWD, dummyIntJ };

//``reverse'' KCvH/A traveling deliveryman. Because the reviewer asked
const t_fAgr TachTD_TSPb{ plusAgr, dummyIEAgr, tachTimeDepExtMtBWD, dummyIntJ };
const t_fAgr TachTD_TSPf{ plusAgr, dummyIEAgr, tachTimeDepExtMtFWD, dummyIntJ };

//damn boilerplate; still, I won't have a const std::string member probTypeName
inline std::string getProblemTypeCode(const t_fAgr input)
{
	if (input == TSP)
		return("TSP");
	else if (input == GTSP)
		return("GTSP");
	else if (input == BTSP)
		return("BTSP");
	else if (input == BGTSP)
		return("BGTSP");
	else if (input == TD_TSPb)
		return("TD_TSPb");
	else if (input == TD_TSPf)
		return("TD_TSPf");
	else if (input == TachTD_TSPb)
		return("TachTD_TSPb");
	else if (input == TachTD_TSPf)
		return("TachTD_TSPf");
	else return ("UFO");//say, Unrecognized Function Object
}

inline t_fAgr getProblemTypeByCode(const std::string input)
{
	if (input == "TSP")
		return(TSP);
	else if (input == "GTSP")
		return(GTSP);
	else if (input == "BTSP")
		return(BTSP);
	else if (input == "BGTSP")
		return(BGTSP);
	else if (input == "TD_TSPb")
		return(TD_TSPb);
	else if (input == "TD_TSPf")
		return(TD_TSPf);
	else if (input == "TachTD_TSPb")
		return(TachTD_TSPb);
	else if (input == "TachTD_TSPf")
		return(TachTD_TSPf);
	//cry int cerr
	else 
	{
		std::cerr << "Invalid problem type code."
			<<"Available types:\n TSP | BTSP | TD_TSPb | TD_TSPf | TachTD_TSP[b|f]\n"
			<<"Terminating.\n";
		exit(EXIT_FAILURE);
		return TSP;//pretend to default to TSP
	}
}

/*returns directions compatible with the given problem type:
XXXb-->{BWD}, XXXf-->{FWD}, _-->{BWD,FWD}*/
inline std::vector<t_Direction> getCompatDrcnByType(const t_fAgr inp)
{
	//std::vector<t_Direction> out{FWD,BWD};//default to both allowed
	auto codeTail = getProblemTypeCode(inp).back();
	if (codeTail == 'f')//only FWD is supported
		return std::vector<t_Direction> { FWD };
	else if (codeTail == 'b')//only BWD supported
		return std::vector<t_Direction>{BWD};
	return std::vector<t_Direction>{ FWD, BWD };
}

#endif

//inline t_cost timeDepExtMtFWD42(const ptag this_pout, const ptag next_pin, const t_bin taskset, const t_cstMx& cst)
//{//NB! this is FWD: must know problem size
//	const auto dim = 42;//say n=42; terminal is 43-d, base is 0
//	return (this_pout == 0)
//		? ((dim + 1 - taskset.count())*cst[this_pout][next_pin])
//		: ((dim - taskset.count())*cst[this_pout][next_pin]);
//	//return (next_pin == cst.size() - 1) ? (cst[this_pout][next_pin]) : (taskset.count() + 2)*cst[this_pout][next_pin];
//}
