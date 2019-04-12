/*
(c) Yaroslav Salii & the Parliament of Owls, 2019+
License:sort of CC---if you go commercial, contact me.

parse-args.h v.1.0 2019-01-30 : tore parsing away from monster-main.cpp
			 v.1.0.1 2019-02-07 : =t_Direction defaults to FWD now;
								  =added default t_slnRequestState values as initializers
			v.1.1 2019-04-12 : added --noP key to disable preprocessing (defaults to DO preprocessing)

devices for parsing command-line arguments into "launch requests," e.g.
=problem type TSP | BTSP | TD_TSP[f,b] | etc
=exact or restricted dyn.prog -H [0,1,...]
=TODO: rewrite arg tests through std::find()
=TODO lower bound & its type
=TODO -UB $value::Int key to start DPBB procedure

NB! const std::string usage is still in monster-main.cpp

might also hold debug run descriptions
*/

//#include<cctype>


#ifndef PARSE_ARGS_H_
#define PARSE_ARGS_H_

#include"reader-base.h"


//chuck the command line arguments into t_lines (deque<string>)
inline t_lines args2lines(const int iargc, char** iargv)
{
	t_lines out(iargc);
	for (int i = 0; i < iargc; i++)
		out[i] = std::string(iargv[i]);
	return out;
}

//apparently I don't really need errors since these are fatal by design (terminate program)
//base 4 to make sure any error gets detected through mod 4 !=0
//originally error:1,absent:4,present:16; 
enum class e_parState :int { absent = 1, present = 4 };

/*
state struct: keeps the state as requested from command line (exact/heuristic, etc.);
note that filename is handled directly in the code, before this state is parsed
*/
using t_slnRequestState =
struct t_slnRequestState
{
	std::pair<e_parState, uint32_t> B{ e_parState::absent,0 };//-H 1 | 2 | ...; default to exact (same as -H 0), fail if not a nonnegative number
	std::pair<e_parState, t_Direction> D{ e_parState::absent,FWD };//-d FWD | BWD; default to FWD
	std::pair<e_parState, t_fAgr> Type{ e_parState::absent,TSP };//-t TSP | BTSP | ... default to TSP; fail if unrecognized or doesn't match direction
	std::pair<e_parState, t_cost> UB{ e_parState::absent,INF };//--UB 0 | 1 | ... given upper bound, to use with DPBB
	std::pair<e_parState, bool> noP{ e_parState::absent,false };//[--noP] flag to FORGO preprocessing; default to DOING preprocessing; 
	//std::pair<e_parState, mock_t_LB> LB_alg; //lower bound algorithm; default to elCheapo; if no upper bound given, don't do DP, just compute the LB
};

//checks if key -K --KK is present in inp, and returns its value (next line)
auto keyFound(const std::string key, const t_lines& iargs)
{//TODO: revamp through find_if &c; it's just a filter, except the position also counts
	int argCt = 1; //iargv[0] = executable name; iargv[1] = input file name, always;
	while (argCt < iargs.size() && iargs.at(argCt) != key)
		argCt++;
	if (argCt < iargs.size()-1 || //key found and it could have a value
		(argCt == iargs.size() - 1 && iargs.at(argCt) == "--noP") ) //unary key found
		return std::make_pair(iargs.at(argCt + 1), true);
	else if (argCt == iargs.size() - 1 )
		return std::make_pair("ERROR: key " + key + " has no value", false);
	else  //argCt==pos, key not found
		return std::make_pair("key " + key + " not found", false);
}
//check if a string is made entirely of digits; from StackOverflow::charles-salvia
bool is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

inline t_slnRequestState parseRequest(const t_lines& iargv)
{
	t_slnRequestState out;
	//first, try for breadth: search for the -H key
//---------------|-H|:BREADTH--------------------------------------/
	if (auto keyArg=keyFound("-H",iargv);keyArg.second)
		//"-H" key found; attempt to parse it (its "argument" is in keyArg.first)
	{
		if (!is_number(keyArg.first))
		{
			std::cerr << "Invalid breadth -H: only nonnegative integers accepted.\nTerminating.\n";
			exit(EXIT_FAILURE);
		}
		else //it's OK, read Breadth now; if 0, default to exact DP
			out.B = (std::stoul(keyArg.first) != 0)
			? (std::make_pair(e_parState::present, std::stoul(keyArg.first)))
			: (std::make_pair(e_parState::absent, 0UL));//not really necessary anymore, could just leave default
	}
	//-------------------next--numeric--parameter----------------------------------------/
	//---------------|--UB|:UPPER--BOUND--FOR--DP-BB-------------------------------/
	if (auto keyArg = keyFound("--UB", iargv); keyArg.second)
	//"--UB" key found; attempt to parse it (its "argument" is in keyArg.first )
	{
		// fail if --UB is not a nonnegative integer (STOPGAP); technically, it can be any t_cost number
		if (!is_number(keyArg.first))
		{
			std::cerr << "Invalid --UB: only nonnegative integers accepted.\nTerminating.\n";
			exit(EXIT_FAILURE);
		}
		else //it's OK, read UB now; 
			out.UB = { e_parState::present, std::stoul(keyArg.first) };
	}
	//-------------------next--parameter----------------------------------------/
	//----------------|-t|:PROBLEM--TYPE--CODE-------------------------------/
	if (auto keyArg = keyFound("-t", iargv); keyArg.second)
		//"-t" key found; attempt to parse it (its "argument" is in keyArg.first)
	{
		//will fail (exit(EXIT_FAILURE)) if the problem code is not recognized
		auto wat = getProblemTypeByCode(keyArg.first);
		//it' OK, write the code now
		out.Type = { e_parState::present, wat };
	}
	else;//"-t" key not found: default to TSP //out.Type = std::make_pair(e_parState::absent, TSP);
	//--------------------next-parameter----------------------------------------/
	//------------------|-d|:DIRECTION-----------------------------------/
	if (auto keyArg = keyFound("-d", iargv); keyArg.second)
		//"-d" key found; attempt to parse it (its "argument" is in keyArg.first)
	{
		//will fail (exit(EXIT_FAILURE)) if the direction code is not recognized
		auto iDirn = getDirectionByCode(keyArg.first);
		//now test if the direction is compatible with problem type (RELIES on "-t" being processed before)
		if (auto zzat = getCompatDrcnByType(out.Type.second);
			std::find(zzat.cbegin(), zzat.cend(), iDirn) == zzat.end())
		{	//given direction not found among compatible
			std::cerr << "Error: problem code -t" << keyArg.first
				<< " is incompatible with direction " << getDirectionCode(iDirn) << "\nTerminating.\n";
			exit(EXIT_FAILURE);
		}//if we didn't fail to be compatible, finally, write the direction
		out.D = { e_parState::present, iDirn };
	}
	if (auto keyArg = keyFound("--noP", iargv); keyArg.second)
	//if "--noP" flag is present, signal to disable preprocessing
	{
		out.noP = { e_parState::present,true };
	}//else default to DO preprocessing
//====================ALL==PARAMETERS==CHECKED================/
	return out;
}

#endif // !PARSE_ARGS_H_

//======BIT====BUCKET================/
//code without keyFound(key,iargs)
////first, try for breadth: search for the -H key
//int argCt = 1;//iargv[0]=executable name; iargv[1]=input file name;
////see what's next and isn't it -H
////---------------|-H|:BREADTH--------------------------------------/
//while (argCt < iargv.size())
//{
//	if (iargv.at(argCt) == "-H") break;
//	argCt++;
//}
////check if the "-H" key was found
//if (argCt < iargv.size())
//	//"-H" key found; attempt to parse it (it's in iargv.at(argCt+1) )
//{
//	bool badPar = false; auto wat = iargv.at(argCt + 1);
//	for (auto c : wat) if (!std::isdigit(c)) badPar |= true;
//
//	if (badPar)
//	{
//		std::cerr << "Invalid breadth -H: only nonnegative integers accepted.\nTerminating.\n";
//		exit(EXIT_FAILURE);
//	}
//	else
//		//it's OK, read Breadth now; if 0, default to exact DP
//		out.B = (std::stoul(wat) != 0)
//		? (std::make_pair(e_parState::present, std::stoul(wat)))
//		: (std::make_pair(e_parState::absent, 0UL));
//}
//else;//"-H" key not found; just retain the initialized case //out.B = std::make_pair(e_parState::absent, 0UL);
////-------------------next--numeric--parameter----------------------------------------/
////---------------|--UB|:UPPER--BOUND--FOR--DP-BB-------------------------------/
//argCt = 1;//refresh the counter
//while (argCt < iargv.size())
//{
//	if (iargv.at(argCt) == "--UB") break;
//	argCt++;
//}
////check if the "--UB" key was found
//if (argCt < iargv.size())
//	//"--UB" key found; attempt to parse it (it's in iargv.at(argCt+1) )
//{
//	//will fail (exit(EXIT_FAILURE)) if --UB is not a nonnegative integer (STOPGAP); technically, it can be any t_cost number
//	bool badPar = false; auto wat = iargv.at(argCt + 1);
//	for (auto c : wat) if (!std::isdigit(c)) badPar |= true;
//
//	if (badPar)
//	{
//		std::cerr << "Invalid --UB: only nonnegative integers accepted.\nTerminating.\n";
//		exit(EXIT_FAILURE);
//	}
//	else
//		//it's OK, read UB now; 
//		out.UB = std::make_pair(e_parState::present, std::stoul(wat));
//}
//else;//"--UB" key not found: default to ordinary exact DP //out.UB = std::make_pair(e_parState::absent, INF);	
////--------------------next-parameter----------------------------------------/
////------------------|-d|:DIRECTION-----------------------------------/
//argCt = 1;//refresh the counter
//while (argCt < iargv.size())//change to std::find/find_if
//{
//	if (iargv.at(argCt) == "-d") break;
//	argCt++;
//}
////check if the "-d" key was found
//if (argCt < iargv.size())
//	//"-d" key found; attempt to parse it (it's in iargv.at(argCt+1) )
//{
//	auto wat = iargv.at(argCt + 1);
//	bool badPar = (wat == ("FWD") || wat == ("BWD")) ? (false) : (true);
//	if (badPar)
//	{
//		std::cerr << "Invalid direction code -d: only BWD | FWD accepted.\nTerminating.\n";
//		exit(EXIT_FAILURE);
//	}
//	else
//		//it's OK, read Direction now
//		out.D = std::make_pair(e_parState::present, getDirectionByCode(wat));
//}
//else;//"-d" key not found: default to backwards DP //out.D = std::make_pair(e_parState::absent, FWD);
//
////-------------------next--parameter----------------------------------------/
////----------------|-t|:PROBLEM--TYPE--CODE-------------------------------/
//argCt = 1;//refresh the counter
//while (argCt < iargv.size())
//{
//	if (iargv.at(argCt) == "-t") break;
//	argCt++;
//}
////check if the "-t" key was found
//if (argCt < iargv.size())
//	//"-t" key found; attempt to parse it (it's in iargv.at(argCt+1) )
//{
//	//will fail (exit(EXIT_FAILURE)) if the code is not recognized
//	auto wat = getProblemTypeByCode(iargv.at(argCt + 1));
//	//it' OK, write the code now
//	out.Type = std::make_pair(e_parState::present, wat);
//}
//else;//"-t" key not found: default to TSP //out.Type = std::make_pair(e_parState::absent, TSP);
//
//
///*all parameters processed, now test their compatibility
//==>that is, if direction D is compatible with problem type code*/
////get the compatible directions	
//auto zzat = getCompatDrcnByType(out.Type.second);
////check if the given---out.D.second---is compatible
//if (std::find(zzat.cbegin(), zzat.cend(), out.D.second) == zzat.end())
////given direction not found among compatible
//{
//	std::cerr << "Error: problem code -t" << iargv.at(argCt + 1)
//		<< " is incompatible with direction " << getDirectionCode(out.D.second) << "\nTerminating.\n";
//	exit(EXIT_FAILURE);
//}
//
//return out;
