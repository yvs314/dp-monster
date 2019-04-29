/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

prec-ord v.1.0 2017-01-09

order descriptions & precedence constraints
*/

//#include"bitset-base.h"
#include"prec-ord.h"

//set bits 0..dim+1; 0 is base, dim+1 is terminal
t_bin mkMask(const uint16_t dim)
{
	t_bin out = 0;
	//set all 0..dim+1; dim+1 means terminal
	for (int i = 0; i <= (dim + 1); i++)
		out.set(i);
	return out;
}

//set bits 1..dim; proper cities only
t_bin mkOmask(const uint16_t dim)
{
	t_bin out = mkMask(dim);
	//reset base=0 and terminal=dim+1
	out.reset(0); out.reset(dim + 1);
	return out;
}

//base and terminal are not considered
//this->first=Receivers (Max, non-isolated); this->second=Senders (Min, non-isolated)
std::pair<t_bin, t_bin> getRcvSnd(const t_vprecDsc& input)
{
	t_bin rcvs, snds; rcvs.reset(); snds.reset();
	//for each proper cluster
	for (int i = 1; i <= input.size() - 2; i++)
	{
		if (input[i].receives_from.any())
			rcvs.set(i);
		if (input[i].sends_to.any())
			snds.set(i);
	}

	return std::make_pair(rcvs, snds);
}

//Isolated: totally incomparable (always belong to both Min and Max)
t_bin getIsld(const t_vprecDsc& input)
{
	t_bin out = 0;
	for (int i = 1; i <= input.size() - 2; i++)
		//if it does not neither send nor receive, it is ISOLATED
	{
		if (input[i].receives_from.none() && input[i].sends_to.none())
			out.set(i);
	}
	return out;
}

t_bin getNonRcvs(const t_vprecDsc& input)
{
	t_bin rcvs = getRcvSnd(input).first;
	return setMinus(mkOmask(input.size() - 2), rcvs);
}

t_bin getNonSnds(const t_vprecDsc& input)
{
	t_bin snds = getRcvSnd(input).second;
	return setMinus(mkOmask(input.size() - 2), snds);
}