/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

reader-base v.1.0 2017-01-09;
			v.1.1 2019-02-12; getCstMxInf: nogo arcs are INF (\inf) instead of -1 as in TSPLIB; for preprocessing

Basic facilities for parsing (line-by-line) the input:
=cost (matrix)
=precedence/order (TSPLIB-SOP format)

reading TSPLIB-SOP is included; might later separate into a namespace/other module
*/

//#include"prec-ord.h"
#include"reader-base.h"

#include<algorithm> // for std::max

//===========PREPROCESS========================/

t_cstMx cwiseBinOp(const t_cstMx& L, const t_cstMx& R, const t_costBinOp f)
{
	auto out = L;
	for (auto row = 0; row < L.size(); row++) 
		for (auto col = 0; col < L[row].size(); col++)
			out[row][col] = f(L[row][col], R[row][col]);
	return out;
}

//this one is verbatim, 0 --> 0; 1--> 1
std::vector<t_cost> bin2ordVec(const t_bin& input, const ptag dim)//dim
{
	auto out = std::vector<t_cost>(dim + 2, 0); //start with a 0-filled vector 0..dim+1
	for (auto i=0; i< out.size(); i++)
		out[i] = input.test(i);//directly copy its contents (less efficient than through foreach_elt)
	return out;
}

//this one writes (-1) instead of 1's---for preprocessing
std::vector<t_cost> bin2ordVec_(const t_bin& input, const ptag dim)//dim
{
	auto out = std::vector<t_cost>(dim + 2, 0); //start with a 0-filled vector 0..dim+1
	for (auto i = 0; i < out.size(); i++)
		out[i] = (-1) * input.test(i);//directly copy its contents (less efficient than through foreach_elt)
	return out;
}

/* make a cstMx 0..size+1 \times 0..size+1, filled with zeroes
and (-1)s at prohibited moves (any, 0), and also INF at (0,size+1) 
 */
t_cstMx mkEmpty_cstMx(const ptag dim)
{
	//zero-filled 0..size+1 \times 0..size+1 matrix; relies on 0 being a feasible t_cost
	t_cstMx out = t_cstMx(dim+2,std::vector(dim+2, 0));
	for (auto row = 1; row < out.size(); row++) //for all rows 1 .. dim+1
		out[row][0] = -1; 
	out[0][dim + 1] = INF; //make 0 to dim+1=terminal cost INF
	return out;
}

/*auxiliary: make a "cost matrix" out of t_vprecDsc relation
put all (-1) matching given t_vprecDsc for proper cities (not 0, not term=n+1)
*/
t_cstMx to_cstMx(const t_vprecDsc& input) //"putPrec, a dual to getPrec"
//CAVEAT:not compatible with GTSP. TODO: add optional city-to-cluster mapping
{
	auto out = mkEmpty_cstMx(input.size()-2); //default precs also work with all of it
	//(i,j)=-1 means can't go i-->j because i <_P j; thus, just take i.receives_from
	for (auto row = 1; row < input.size() - 1; row++) //for all rows 1 .. dim
	{
		//out[row] = bin2ordVec(input[row].receives_from, input.size() - 2); // --- WRONG
		out[row] = bin2ordVec_(input[row].sends_to, input.size() - 2);
		out[row][0] = -1; //write NOGO to 0 just in case
	}

	return out;
}

//map INF to "-1" in the matrix, to make TSPLIB "nogo" edges cost infinite &c
t_cstMx mapInfMx(const t_cstMx& input)
{
	t_cstMx out = input;
	for (int row = 0; row < input.size(); row++)
	{
		for (int column = 0; column < input.size(); column++)
			out[row][column] = infiny(out[row][column]);
	}
	return out;
}

//auxiliary: test if t_vprecDsc's principal ideal and principal filter representations match
//TODO: cut preprocessing out, make a separate prep.h/ord-rel.h, dependent on prec-ord.h and reader-base.h (for t_cstMx)


/*given a raw TSPLIB-like cost matrix,
return a preprocessed cost matrix: all transitive arcs are INF, all dual arcs are INF
WARNING: assumes _transitive_ precedence constraints as input in t_cstMx*/
t_cstMx getPrepCstMx(const t_cstMx& input)
{
	
	auto P = getPrec(input); //get the precedence constraints as t_vprecDsc
	auto PP=addPrecDptTrm(P);//patch P to include all info about depot and terminal
	auto Psquare = compose(PP, PP);//find the _transitive_ edges (not part of trans.reduction) as t_vprecDsc
	//replace (-1) with INF: kill the transitive edges
	auto zat = mapInfMx(to_cstMx(Psquare));
	//replace (-1) with INF: kill the dual relation
	auto out = mapInfMx(input);
	//make sure INFs at _trans.edges_ propagate to the cost matrix
	out = cwiseBinOp(out, zat, [](const t_cost& l, const t_cost&r) {return std::max(l, r); });
		
	return out;
}




//--------------------------------------------- /


//=============READING========================/
/*read a file line by line into a deque*/
t_lines readFile(std::ifstream& fin)
{
	t_lines input;
	while (fin)
	{
		std::string wat;
		std::getline(fin, wat);
		input.push_back(wat);
	}
	return input;
}

//read the input as a row of t_cost; ASSUMPTION: only t_cost values on the line
std::vector<t_cost> getCstRow(const std::string input, const uint16_t dim)
{
	std::istringstream reader(input);
	std::vector<t_cost> out;
	out.resize(dim + 2);//the indices are 0..dim+1; dim+1 is the terminal
	//uint16_t column = 0;//start with zero---the base's index
	for (uint16_t column = 0; column < (dim + 2); column++)
		reader >> out[column];
	return out;
}

//read the cost matrix, line by line; starts at isEWSLine+1, dim+2 lines
t_cstMx getCstMx(const t_lines input)
{
	auto dim = getDim(input);//shorthand for calling it;
	t_cstMx out;
	bool cstRecStart = false;//signals it's time to start recording
	int16_t lineCt = -2;//must skip the line directly below EDGE_WEIGHT_SECTION
	for (auto line : input)
	{
		if (isEWSLine(line))
			cstRecStart = true;//prepare to start reading the costs
		//while we indeed look at the matrix's string representation
		if (lineCt >= 0 && lineCt <= (dim + 1))
		{
			//read the current line as the matrix's row
			out.push_back(getCstRow(line, dim));
		}
		if (lineCt > dim + 1)
			return out;
		if (cstRecStart)//count this line, if we did start recording
			lineCt++;
	}
	//std::cerr << "Can't read the costs. Terminating.\n";
	//exit(EXIT_FAILURE);
}



//========READING==ORDER==DATA===========================/

//yep, flip the rows and columns; CAVEAT:only for SQUARE matrices
t_cstMx transpose(const t_cstMx& input)
{
	//input is 0..dim+1\times0..dim+1
	t_cstMx out;
	//make sure out conforms; yes, I only care for square matrices
	out.resize(input.size());
	for (int row = 0; row<input.size(); row++)
		out[row].resize(input.size());
	//for every input[i][j], set out[j][i]=input[i][j]
	for (int row = 0; row < input.size(); row++)
		for (int column = 0; column < input.size(); column++)
			out[column][row] = input[row][column];

	return out;
}


//map unify to the whole matrix
t_cstMx unifyMx(const t_cstMx& input)
{
	//input is 0..dim+1\times0..dim+1
	t_cstMx out = input;
	for (int row = 0; row<input.size(); row++)
	{
		for (int column = 0; column < input.size(); column++)
			out[row][column] = unify(out[row][column]);
		//colEntry = unify(colEntry);
	}
	return out;
}

//read a matrix' row as a standard bitset, t_bin
t_bin ordVec2bin(const std::vector<t_cost>& input)
{
	t_bin out; out.reset();//make sure it's null at the start
	// if input[i]==1 then out |= (2<<i)
	for (int i = 0; i< input.size(); i++)
		if (input[i] == 1)
			out.set(i);

	return out;
}



//from TSPLIB-SOP cost matrix, read the precedence data
//cost[i][j]==-1 means i>j (transitive)--- a PREDECESSORS matrix
t_vprecDsc getPrec(const t_cstMx& input)
{
	t_vprecDsc out;
	//just in case, out[0] describes BASE, and out[dim+1] TERMINAL
	out.resize(input.size());
	//cost[i][j]==-1 --> i>j is a PREDECESSORS matrix 
	//rows are PREDECESSOR `lists'
	t_cstMx gtMx = unifyMx(input);
	//unify and transpose it to get a cost[i][j]==1 --> i<j
	//a SUCCESSORS matrix: rows are SUCCESSOR `lists'
	t_cstMx ltMx = transpose(unifyMx(input));
	//now, correct them: 0 precedes all; dim+1 precedes none
	//0 succeeds none, dim+1 succeeds all; however, they are treated separately
	//and will not be processed; 
	for (auto i = 0; i < input.size(); i++)
	{
		if (i != 0) ltMx[0][i] = 1;
		if (i != input.size() - 1) gtMx[i][input.size() - 1] = 1;
		gtMx[i][0] = 0; gtMx[i][input.size() - 1] = 0;
		if (i != input.size() - 1) ltMx[0][i] = 1;
		ltMx[i][input.size() - 1] = 0;
	}
	gtMx[0][0] = 0; ltMx[input.size() - 1][input.size() - 1] = 0;
	ltMx[0][0] = 0; gtMx[input.size() - 1][input.size() - 1] = 0;
	gtMx[input.size() - 1][0] = 1;

	//now, chuck the matrices into t_bin
	for (auto i = 0; i < input.size(); i++)
	{
		out[i].receives_from = ordVec2bin(gtMx[i]);
		out[i].sends_to = ordVec2bin(ltMx[i]);
	}
	return out;
}
//--------------------------------------------- /




//=================TSPLIB-SOP====================/
//a predicate to test if the line tells the DIMENSION
bool isDimLine(const std::string inp)
{
	std::istringstream wat(inp); std::string lul;
	std::getline(wat, lul, ':');
	if (lul != "DIMENSION")
		return false;	else		return true;
}

//test if this string tells TSPLIB-SOP name ("NAME: $name")
bool isNameLine(const std::string inp)
{
	std::istringstream wat(inp); std::string lul;
	std::getline(wat, lul, ':');
	if (lul != "NAME")
		return false;	else		return true;
}

//cut the name outta the input
std::string getName(const t_lines& input)
{
	for (auto line : input)
		if (isNameLine(line))
		{
			std::string out;
			std::istringstream wat(line);
			std::string trash;
			std::getline(wat, trash, ':');//read up to ':' as std::string; discard ':'
			//wat.ignore(':');
			wat >> out;//read what remains as uint16_t
			return out;
		}
}


//read the .SOP's DIMENSION:; decrease by 2 (don't count BASE or TERMINAL)
uint16_t getDim(const t_lines& input)
{
	for (auto line : input)
		if (isDimLine(line))
		{
			uint16_t out;
			std::istringstream wat(line);
			std::string trash;
			std::getline(wat, trash, ':');//read up to ':' as std::string; discard ':'
			wat >> out;//read what remains as uint16_t
			return (out - 2);
		}
	return 0;//shan't be reached
}

//cut .SOP's description (all up to and including DIMENSION:)
t_lines getDesc(const t_lines& input)
{
	t_lines out;
	auto it = input.cbegin();
	while (!isDimLine(*it))
	{
		out.emplace_back(*it);
		it++;
	}
	out.emplace_back(*it);
	return out;
}

//a predicate to test if the line marks the EDGE_WEIGHTS_SECTION
bool isEWSLine(const std::string inp)
//warning! whitespace, etc. after the mark kills the procedure;
//ought to be sanitized
{
	std::istringstream wat(inp);
	std::string lul;
	std::getline(wat, lul, '\n');
	if (lul != "EDGE_WEIGHT_SECTION")
		return false;	else		return true;
}
//--------------------------------------------------/