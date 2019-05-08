/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

reader-base v.1.0 2017-01-09; 

Basic facilities for parsing (line-by-line) the input:
=cost (matrix)
=precedence/order (TSPLIB-SOP format)

reading TSPLIB-SOP is included; might later separate into a namespace/other module
*/

#ifndef READER_H_
#define READER_H_

//#include"bitset-base.h"
#include"prec-ord.h"
#include<fstream>
#include<deque>
#include<string>
#include<functional> // for t_costBinOp

//strings in the container are thought to be separated by NEWLINE;
using t_lines = std::deque<std::string>;

/*read a file line by line into a deque*/
t_lines readFile(std::ifstream& fin);

/*
flatten the input list while interspersing with the provided separator;
only 1-char separators (at present too lazy to properly reimplement it)
*/
inline std::string isperse(const std::deque<std::string>& list, const std::string sep)
{
	std::string acc = "";
	for (std::string i : list)
	{
		//skip empty lines
		if(!i.empty()) acc = acc + i + sep;
	}
	acc.pop_back();
	return(acc);
}

//=========COST===CONSTANTS====================/
//int32_t for a more direct compatibility with TSPLIB ("-1" signals precedence)
using t_cost = int32_t;//might some day template it 
const t_cost INF = std::numeric_limits<t_cost>::max();
const t_cost MINF = std::numeric_limits<t_cost>::min();
//--------------------------------------------- /

using t_cstMx = std::vector < std::vector<t_cost> >;

//=========PREPROCESSING==================================/
//CAVEAT: not compatible with GTSPs yet

//any function on costs; with constant references to match std::max
using t_costBinOp = std::function<t_cost (const t_cost&, const t_cost&)>;

t_cstMx cwiseBinOp(const t_cstMx& L, const t_cstMx& R, const t_costBinOp f);

//dual to ordVec2bin
std::vector<t_cost> bin2ordVec(const t_bin& input, const ptag dim);

//this one writes (-1) instead of 1's---for preprocessing
std::vector<t_cost> bin2ordVec_(const t_bin& input, const ptag dim);

t_cstMx mkEmpty_cstMx(const ptag size);

t_cstMx to_cstMx(const t_vprecDsc& input);

inline t_cost infiny(const t_cost input)
{
	return (input == -1) ? (INF) : (input);
};

t_cstMx mapInfMx(const t_cstMx& input);

t_cstMx getPrepCstMx(const t_cstMx& input);//transitive arcs are INF, dual arcs are INF
//----------------------------------------------------------/

//read the input as a row of t_cost; ASSUMPTION: only t_cost values on the line
std::vector<t_cost> getCstRow(const std::string input, const uint16_t dim);

//read the cost matrix, line by line; starts at isEWSLine+1, dim+2 lines
t_cstMx getCstMx(const t_lines input);



//========READING==ORDER==DATA===========================/
//yep, flip the rows and columns; CAVEAT:only for SQUARE matrices
t_cstMx transpose(const t_cstMx& input);

//to retain only order data, expressed as (cost[i][j]==-1 EQ. i>j) in TSPLIB
inline t_cost unify(const t_cost input)
{	return (input == -1) ? (1) : (0);	}

//map unify to the whole matrix
t_cstMx unifyMx(const t_cstMx& input);

//read a matrix' row as a standard bitset, t_bin
t_bin ordVec2bin(const std::vector<t_cost>& input);



//from TSPLIB-SOP cost matrix, read the precedence data
//cost[i][j]==-1 means i>j (transitive)--- a PREDECESSORS matrix
t_vprecDsc getPrec(const t_cstMx& input);

//WARNING: was getPrec too (overloading)
inline t_vprecDsc readPrec(const t_lines& input)
{
	return getPrec(getCstMx(input));
}
//--------------------------------------------- /



//=================TSPLIB-SOP====================/
//a predicate to test if the line tells the DIMENSION
bool isDimLine(const std::string inp);

//test if this string tells TSPLIB-SOP name ("NAME: $name")
bool isNameLine(const std::string inp);

//cut the name outta the input
std::string getName(const t_lines& input);

//read the .SOP's DIMENSION:; decrease by 2 (don't count BASE or TERMINAL)
uint16_t getDim(const t_lines& input);

//cut .SOP's description (all up to  DIMENSION:)
t_lines getDesc(const t_lines& input);

//a predicate to test if the line marks the EDGE_WEIGHTS_SECTION
bool isEWSLine(const std::string inp);
//--------------------------------------------- /
#endif