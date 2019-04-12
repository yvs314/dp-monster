/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

bitset-base v.1.0 2017-01-09
			v.1.1 2018-02-09: custom hash function, for inter-implementation consistency

just bitsets & operations; no order yet
*/

#ifndef BITSETBASE_H_
#define BITSETBASE_H_

#include<cstdint>//for definite-length types and numeric_limits
#include<bitset>
#include<vector>
#include<algorithm>//for std::min
#include<string>//for output
#include<deque>//for lines, deque<string>
#include<sstream>//for output

//=========DIMENSION===CONSTANTS===================================/
//max intended size is 384=64*6; max size in TSPLIB-SOP is 380
const uint16_t refdim =384;
//const uint16_t refdim = 10;
//max cluster label~65535
using mtag = uint16_t;//in fact, 9 bits are enough
//max city (point) label ~65535
using ptag = uint16_t;
//----------------------------------------------------------------------/


using t_bin = std::bitset < refdim >;

const auto chunkSize = std::numeric_limits<unsigned long long>::digits;
//chunks for cutting t_bin into ullongs
using t_chunk = std::bitset <chunkSize >;

inline t_chunk getLeastSigChunk(const t_bin& inp)
{
	t_chunk out;
	for (auto pos = 0; pos < std::min(out.size(), inp.size()); pos++)
		out[pos] = inp[pos];
	return out;
}

std::vector<t_chunk> t_bin2vchunk(t_bin inp);

//write as 64-bit hexadecimal numbers; 64-bit chunks separated by dots; big-endian
std::string to_stringHex(const t_bin& inp);

/*lexicographical RIGHT-TO-LEFT (least-sig first) comparison function,
for using them in maps. Strict left < right;
*/
inline bool less_t_bin_orig(const t_bin& left, const t_bin& right)
{
	auto cleft = left; auto cright = right;

	//if [what remains is] equal, don't bother checking, say it's false
	while (cleft != cright)
	{
		auto lchunkUllong = getLeastSigChunk(cleft).to_ullong();
		auto rchunkUllong = getLeastSigChunk(cright).to_ullong();
		//if the left [least-sig] chunk is less than the right, say so
		if (lchunkUllong < rchunkUllong) return true;
		//else, cut these chunks away and continue; UNSAFE? for refdim not multiples of 64
		cleft >>= chunkSize;
		cright >>= chunkSize;
	} 
	//if we're here, cleft==cright; thus, it's FALSE that cleft<cright
	return false;
}
/*
Here's the dumb version, meant to be more definite and cross-platform stable.
Compare bitwise, starting with the most significant bit;
left < right -->  true  
left >= right --> false 
*/
inline bool less_t_bin(const t_bin& left, const t_bin& right)
{
	for (auto i = refdim - 1; i >= 0; i--)
	{
		/*if the bits do not match and the right is 1, then left < right;
		tertium non datur*/
		if (left.test(i) != right.test(i)) return right.test(i);
	}
	//if we're here, all the bits matched, then, it's false that left < right
	return false;
}

//function object wrapper for t_bin comparison
struct t_binComp
{
	inline bool operator()(const t_bin& left, const t_bin& right) const
	{
		return less_t_bin(left, right);
	}
};

inline t_bin mkBit0(void) { t_bin out; out.reset(); out.set(0); return out; }
inline t_bin mkEmptySet(void){ t_bin out; out.reset(); return out; }

const t_bin BIT0 = mkBit0();//least-significant bit=1 (is set); for shifting
const t_bin EmptySet = mkEmptySet();

//pure setMinus; returns left\setminus right
inline t_bin setMinus(const t_bin& left, const t_bin& right)
{
	//OR then XOR, to make sure they DIE
	t_bin out = left; out |= right; out ^= right;
	return out;
}

//undiluted side effects-setminus; removes elements of $right from $left
inline void ssetMinus(t_bin& left, const t_bin& right)
{
	left |= right; left ^= right; return;
}

//seek (next) set bit: return the next more significant than pos 1 bit or ``last''
//i.e., ``last'' means no bits set beyond pos
inline uint16_t  ssb(const t_bin input, const uint16_t pos, const uint16_t last)
{
	uint16_t ct = pos + 1;
	while (!input.test(ct) && ct <= last)
		ct++;
	return ct;
}

//=========CUSTOM====HASH===FUNCTION===============/
//custom hash function for t_bin (std::bitset), for a consistency test
struct t_binHash
{
	inline std::size_t operator() (const t_bin& inp) const
	//just XOR the words (chunks, expected to be 64-bit)
	{
		auto wat = t_bin2vchunk(inp);
		t_chunk out= 0;
		for (auto chunk : wat)
			out ^= chunk;
		
		return out.to_ullong();
	}
};
//-------------------------------------------------/

#define foreach_elt(x,X,crd)  for(auto x=ssb(X,0,crd);x<=crd;x=ssb(X,x,crd))
#define foreach_point(p,pfirst,plast) for (ptag p=pfirst; p<=plast;p++)

//TODO: inline to_uset: t_bin -> std::unordered_set<mtag>; also, mtag -> std::unordered_set<ptag> (?)

#endif

//macro for accessing, in a cycle, each element of X, where crd=|X|; requires a body!
//#define foreach_elt(x,X,crd)  for(auto x=ssb(X,0,crd);x<=crd;x=ssb(X,x,crd))
