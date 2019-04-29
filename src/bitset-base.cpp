/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

bitset-base v.1.0; 2017-01-09.

This is bitset-base.cpp: all regarding bitsets, no orders yet;
=listing the next 1 bit
=writing out hexadecimal mask, dotted for every 64 bits
=H=refdim:max bit count in bitset

*/

#include "bitset-base.h"

//got inlined--->written in "bitset-base.h"
//t_chunk getLeastSigChunk(const t_bin& inp)
//{
//	t_chunk out;
//	for (auto pos = 0; pos < std::min(out.size(), inp.size()); pos++)
//		out[pos] = inp[pos];
//	return out;
//}

//cut a bitset into (64-bit) chunks
std::vector<t_chunk> t_bin2vchunk(t_bin inp)
{
	int16_t bits_left = refdim;//no, I don't think I'll ever exceed 2^15
	std::vector<t_chunk> out;
	while (bits_left > 0)
	{
		t_chunk chunk = getLeastSigChunk(inp);
		out.push_back(chunk);
		inp >>= chunkSize;//shr by chunkSize (longest unsigned int)
		bits_left -= chunkSize;//one chunk less to go
	}
	return out;
}

//write as 64-bit hexadecimal numbers; 64-bit chunks separated by dots; big-endian
std::string to_stringHex(const t_bin& inp)
{
	std::deque<std::string> sacc; std::string chunkSep = "."; std::string out = "";
	//int16_t bits_left = refdim;//no, I don't think I'll ever exceed 2^16
	auto chunks = t_bin2vchunk(inp);
	//we've got a forward_list; works as a stack, it does
	for (auto chunk : chunks)
	{
		std::ostringstream hexConv;
		hexConv << std::hex << chunk.to_ullong();
		sacc.push_front(hexConv.str());
		sacc.push_front(chunkSep);
	}//deepest in sacc is the least-significant chunk; top is chunkSep, then the least-significant
	sacc.pop_front();//front was an extraneous chunkSep
	auto concat = [&out](std::string line){out += line; return out; };
	std::for_each(sacc.cbegin(), sacc.cend(), concat);
	//[&out](std::string line){out += line; return out; });
	return out;

}