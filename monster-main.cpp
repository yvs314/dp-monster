/*
(c) Yaroslav Salii & the Parliament of Owls, 2018+
License:sort of CC---if you go commercial, contact me.

This is TSP-PC mod.3 BB, an implementation of
Morin--Marsten branch-and-bound scheme for dynamic programming,
primarily aimed at precedence constrained TSP (TSP-PC).

A third update of the original **2011 solver**'s 2016 modular rewrite.
*/


#include<cctype>

#include"DPBB.h" //exact DP with B&B scheme \`a la Morin--Marsten
#include"RTDP.h" //exact & restricted DP
#include"reader-base.h"
#include"parse-args.h"
//tmp-debug:
#include"lower-bound.h"

const std::string usage = " filename.sop [--noP] [-H breadth] [-d FWD | BWD] [-t TSP | BTSP | TD_TSPb | etc] [--UB upper_bound]";


/* wat.exe $name.sop -H $breadth
--> open name.sop for input, assume TSPLIB input;
solve it by RT-DP with at most $breadth arguments
exit if refdim is less than the problem's size
NB!: refdim must be set in the code, i.e, makefile-build if I ever get to automate it
--noP : disable preprocessing (default: do preprocessing, i.e., set costs of transitive and dual edges to INF)
-H breadth: 1..inf, if absent then exact DP
-d FWD | BWD, if absent default to BWD, if erroneous, terminate
-t TSP | BTSP | TD_TSPb | TD_TSPf | GTSP | BGTSP, if absent default to TSP, if erroneous, terminate
also, test if requested type matches direction: (*b <-> BWD), (*f <-> FWD)
--UB 0 | 1 | ... given upper bound, to use with DPBB; "long" option
DO NOT COMBINE -H  and --UB yet; right now, --UB is silently given priority
*/
//


int main(int argc, char* argv[])
{
//	bool exactRequested = false;
	//bool debugRun = true;
	int debugRun = 0;
	//int debugRun = 1;
	int myargc = argc; 
	t_lines myargv = args2lines(argc,argv);
	
	switch (debugRun)
	{
	case 1:	
		std::cerr << "Requested debug run no."<<debugRun<<"\n";
		myargc = 4;
		myargv.resize(myargc);
		myargv[0] = argv[0];
		myargv[1] = "ESC07.sop";//"br17.12.sop";//"p43.4.sop";//"ry48p.4.sop";//"toy05.sop";
		myargv[2] = "-d";
		myargv[3] = "FWD";
		break;
	case 2:
		std::cerr << "Requested debug run no." << debugRun << "\n";
		myargc = 6;
		myargv.resize(myargc);
		myargv[0] = argv[0];
		myargv[1] = "ESC07.sop";//"br17.12.sop";//"p43.4.sop";//"ry48p.4.sop";//"toy05.sop";
		myargv[2] = "-d";
		myargv[3] = "FWD";
		myargv[4] = "--UB";
		myargv[5] = "3000";
		break;
	default: std::cerr << "This is not a drill.\n";
	}

	//=========PARSE===COMMAND===LINE==&==EXECUTE===================/
	//fail if input file name is not specified or there are too many arguments (not combining -H $breadht and --UB $bound yet)
	if (myargc>9 || myargc==1) { std::cerr << myargv[0] << usage; exit(EXIT_FAILURE); }
	
	std::string ifName = myargv[1];
	std::ifstream fin;
	fin.open(ifName);//fin.open("ry48p.2.sop");

	if (!fin.is_open())
	{
		std::cerr << "Can't open " << ifName << ".\nTerminating.\n";
		exit(EXIT_FAILURE);
	}
	//read the input line by line
	auto input = readFile(fin);

//process command line args;
	const t_slnRequestState request{ parseRequest(myargv) };
//start reading the input file (if we're here, the command line args were correct)
	std::cerr << "Turns out, there are " << getDim(input) << " cities here.\n";
	if (refdim < getDim(input) + 2)
	{
		std::cerr << getName(input) << "max dimension exceeded: "
			<< getDim(input) << "/" << refdim << "\n";
		exit(EXIT_FAILURE);
	}
	
	//==Instance=with=preprocessing--rm-trans====
	t_Instance testInstP = t_Instance(
		getDesc(input)
		, getName(input)
		, request.Type.second
		, getPrepCstMx(getCstMx(input)) //cost matrix is _preprocessed_: dual and transitive are removed
		, getPrec(getCstMx(input)));
	//std::exit(EXIT_SUCCESS); //dangerous!
	
 //BACKUP: no preprocessing; e.g. for GTSP; might adapt later
	t_Instance testInst = t_Instance(
		getDesc(input)
		, getName(input)
		, request.Type.second 
		, getCstMx(input) //to be replaced with preprocess.getCstMx input
		, getPrec(getCstMx(input)));
	
	t_Instance theInstance = (request.noP.second == true) //if --noP key is set
		? (testInst)//ABANDON preprocessing
		: (testInstP);//if not, then DO preprocessing

	if (request.noP.second == false) //if --noP key was NOT set, say we're doing it!
		std::cerr << "\n Preprocessing engaged.\n";

	//TMP===DEBUG-LB==BLK
	//{//it's such a dirty hack, but it works; run a.out WAT.sop > WAT.sop.dump.lb
	//	auto lbP = elCheapoLB(0, testInst.wkOrd.omask, testInstP, FWD); // with preprocessing & INFs
	//	auto lb = elCheapoLB(0, testInst.wkOrd.omask, testInst, FWD); // without that
	//	std::cout << "\nVALUE-P;VALUE-NO-P:" << lbP.first << ";" << lb.first
	//		<< "\n\n" << "SOLUTION-P:vertexFrom;vertexTo;arcCost\n" << dumpArcs(lbP.second)
	//		<< "\n\n" << "SOLUTION-NO-P:vertexFrom;vertexTo;arcCost\n" << dumpArcs(lb.second) << "\n\n";
	//	return 0;
	//}
	//-------------------/

	//first try DP-BB; would consider restricted DP-BB m*u*c*h later
	if (request.UB.first==e_parState::present)
	{//note testInstP, with P for Preprocessing (transitive edges removed through cost=INF)
		t_BBDP testSln(theInstance, request.D.second, request.UB.second, "CHP");
		std::cerr << "Requested dynamic programming with branch and bound, UB=" << request.UB.second
			<< "\nDirection:" << getDirectionCode(request.D.second) << "\n"; 
		testSln.solve();
	}
	//if breadth was specified (not zero)
	else if (request.B.first==e_parState::present)
	{
		t_hRtDP testSln(theInstance, request.D.second, request.B.second);
		std::cerr << "Requested heuristic: hRtDP, breadth=" << request.B.second
			<< "\nDirection:" <<getDirectionCode(request.D.second) <<"\n";
		testSln.solve();
	}
	else//breadth was not specified: exact DP requested
	{
		t_DP testSln(theInstance, request.D.second);
		std::cerr << "Requested exact dynamic programming" << "\nDirection:" << getDirectionCode(request.D.second) << "\n"; 
		testSln.solve();
	}
	

	std::cerr << "\nlolwut.\n";
	
	return 0;
}
//------------------------------------/
//======BIT========BUCKET=============/
//{//it's such a dirty hack, but it works; run a.out WAT.sop > WAT.sop.dump.lb
	//	auto lb = elCheapoLB(0, testInst.wkOrd.omask, testInst, FWD);
	//	std::cout<<"VALUE:"<<lb.first<<"\n\n"<<"SOLUTION:vertexFrom;vertexTo;arcCost\n"<<dumpArcs(lb.second);
	//	return 0;
	//}	
















//if (debugRun==1)
	//{
	//	/*myargc = 2;
	//	myargv[0] = argv[0];
	//	myargv[1] = "ESC07.sop";*/
	//	myargc = 4; 
	//	myargv.resize(myargc);
	//	myargv[0] = argv[0];
	//	myargv[1] = "ESC07.sop";//"br17.12.sop";//"p43.4.sop";//"ry48p.4.sop";//"ESC07.sop";
	//	myargv[2] = "-d";
	//	myargv[3] = "FWD";
	//}
	//else if (debugRun == 2)
	//{
	//	myargc = 6;
	//	myargv.resize(myargc);
	//	myargv[0] = argv[0];
	//	myargv[1] = "ESC12.sop";
	//	myargv[2] = "-H";
	//	myargv[3] = "10";
	//	myargv[4] = "-d";
	//	myargv[5] = "FWD";
	//}
	//else if (debugRun == 3)
	//{
	//	myargc = 8;
	//	myargv.resize(myargc);
	//	myargv[0] = argv[0];
	//	myargv[1] = "ESC07.sop";
	//	myargv[2] = "-H";
	//	myargv[3] = "0";
	//	myargv[4] = "-d";
	//	myargv[5] = "BWD";
	//	myargv[6] = "-t";
	//	myargv[7] = "TD_TSPf";//looking for trouble: incomp. with direction
	//}
	//else if (debugRun == 4)
	//{
	//	myargc = 4;
	//	myargv.resize(myargc);
	//	myargv[0] = argv[0];
	//	myargv[1] = "toy05.sop";//"br17.12.sop";//"p43.4.sop";//"ry48p.4.sop";//"ESC07.sop";
	//	myargv[2] = "-d";
	//	myargv[3] = "FWD";
	//}
	//else if (debugRun == 5)//testing msvc17-gcc.6.3.0 inconsistency
	//{
	//	myargc = 6;
	//	myargv.resize(myargc);
	//	myargv[0] = argv[0];
	//	myargv[1] = "br17.10.sop";
	//	myargv[2] = "-H";
	//	myargv[3] = "1";
	//	myargv[4] = "-d";
	//	myargv[5] = "BWD";
	//}
	//else if (debugRun == 6)//testing tachyonTD-TSP, damn it
	//{
	//	myargc = 8;
	//	myargv.resize(myargc);
	//	myargv[0] = argv[0];
	//	myargv[1] = "br17.10.sop";
	//	myargv[2] = "-H";
	//	myargv[3] = "1";
	//	myargv[4] = "-d";
	//	myargv[5] = "BWD";
	//	myargv[6] = "-t";
	//	myargv[7] = "TachTD_TSPb";
	//}
