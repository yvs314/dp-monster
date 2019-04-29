/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

heap-break v.1.0 2017-01-17;
heap-break v.2.0 2017-01-26; dumped heap break; 
will query the OS for this process' memory usage
directly
mem-rec v.2.1 2017-01-26; added alpha linux support;
mem-rec v.2.2 2017-05-03; write MEM_PHYS;MEM_VIRT separately

removed relMem; not that it did help at any time

should support unix & windows through conditional compilation
*/

//
//#ifdef __unix__
//#define HBRK_H_
//#endif
//
//

#ifndef HBRK_H_
#define HBRK_H_


#include<cstdint>
#include<string>
#include<sstream>
#include<stack>

#ifdef _WIN32
#include<Windows.h>
#include<Psapi.h>

//fst:RAM, snd:SWAP
inline std::pair<size_t,size_t> getMemUsage()
{
	//get the handle to this process
	auto myHandle = GetCurrentProcess();

	//to fill in the process' memory usage details
	PROCESS_MEMORY_COUNTERS pmc;
	//return the usage (bytes), if I may
	if (GetProcessMemoryInfo(myHandle, &pmc, sizeof(pmc)))
//RAM + Page file usage, just in case
		return std::make_pair(pmc.WorkingSetSize,pmc.PagefileUsage);
	else
		return std::make_pair(0,0);
}
#endif

#ifdef __linux__
#include <fstream>



//test if this string tells PROCFS phys.mem usage,  "VmRSS: $size kB"
bool isMemLine(const std::string inp)
{
	std::istringstream wat(inp); std::string lul;
	std::getline(wat, lul, ':');
	if (lul != "VmRSS")
		return false;	else		return true;
}

//test if this string tells PROCFS virt.mem usage,  "VmSize: $size kB"
bool isVMemLine(const std::string inp)
{
	std::istringstream wat(inp); std::string lul;
	std::getline(wat, lul, ':');
	if (lul != "VmSize")
		return false;	else		return true;
}

//read the phys.mem usage outta the input; return in bytes (<< 10)
uint64_t getMem(const t_lines& input)
{
	for (auto line : input)
		if (isMemLine(line))
		{
			uint64_t out;
			std::istringstream wat(line);
			std::string trash;
			std::getline(wat, trash, ':');//read up to ':' as std::string; discard ':'
			//wat.ignore(':');
			wat >> out;//read what remains as uint64_t
			// it's in kB,  return in bytes,  i.e.,  <<10ULL
			return (out << 10ULL);
		}
}


// 
uint64_t getVMem(const t_lines& input)
{
	for (auto line : input)
		if (isVMemLine(line))
		{
			uint64_t out;
			std::istringstream wat(line);
			std::string trash;
			std::getline(wat, trash, ':');//read up to ':' as std::string; discard ':'
			wat >> out;//read what remains as uint64_t
			// it's in kB,  return in bytes,  i.e.,  <<10ULL
			return (out << 10ULL);
		}
	return 0;//shan't be reached
}

inline std::pair<size_t,size_t> getMemUsage()
{
	std::ifstream ifStat;
	ifStat.open("/proc/self/status");//defaults to readonly
	if (ifStat.is_open())
	{
		auto procInp = readFile(ifStat);
		return (std::make_pair(getMem(procInp),getVMem(procInp)) );
	}
	else return std::make_pair(0,0);
}
#endif

/*
cut 64-bit number of bytes [occupied] into 10-bit chunks,
to make B-->KB-->MB-->GB-->PB representation; out.head is most-sig portion
*/
//head = GB;
//[3]=GB [2]=MB [1]=KB [0]=B; binary-decimal; inp BYTES-> GB*2^30 + MB*2^20 + KB*2^10+B
inline std::stack<uint16_t> toGMKbytes(uint64_t bytes)
{
	static const uint64_t KB = 1023;//0x3FF; bin: 10 ones
	std::stack<uint16_t> out;
	auto chunk = bytes & KB;
	do //for (int i = 1; i <= 4; i++)
	{
		auto chunk = ( bytes & KB);
		out.push(chunk);
		bytes >>= 10;//chuck 10 least-sig bits, 0..9; (B-->KB-->MB-->GB);
	} while (bytes != 0);//stop if zero

	return out;
}

//untested
inline std::string to_stringBytes(const uint64_t bytes)
{
	std::ostringstream acc; const std::string sep = "~";//10GB~2MB~3KB~123B
	std::deque<std::string> captions = { "GB", "MB", "KB", "B" };
	auto cuts = toGMKbytes(bytes);
	//retain only nonzero captions
	while (cuts.size() < captions.size()) captions.pop_front();
	do
	{
		acc << cuts.top() << captions.front() << sep;
		cuts.pop(); captions.pop_front();
	} while (!cuts.empty());
	//static 
	//std::string out=
	std::string out = acc.str(); out.pop_back();//remove the extraneous sep
	return out;
}

using t_memRec =
struct t_memRec
{
	
	size_t physMem;//physical memory (RAM) in use, absolute
	size_t virtMem;//``virtual'' memory (page/swap file), not including RAM, absolute
	
	//total memory usage
	size_t totalMem()
	{return physMem + virtMem;}
	
	
	//constructor: just get the current RAM and SWAP usage; make sure to only call getMemUsage once to avoid side effects
	t_memRec()
	{auto wat = getMemUsage(); this->physMem = wat.first; this->virtMem = wat.second;}
	
	//dumb copy constructor
	t_memRec(const size_t iPhysMem, const size_t iVirtMem)
		: physMem(iPhysMem)
		, virtMem(iVirtMem){	};

	std::string to_string () const
	{
		std::ostringstream wat; std::string s = ";";
		wat << to_stringBytes(physMem) << s << to_stringBytes(virtMem);
		return wat.str();
	}
};

#endif


//t_memRec relMemUsage(const t_memRec& prev)
//	//difference in memory usage compared with prev; always defined as 
//{
//	return t_memRec(std::max(this->physMem, prev.physMem) - std::min(this->physMem, prev.physMem))
//}

//step: compare with the previous
//t_memRec relMemUsage(const t_memRec& prev)
//	: physMem(getMemUsage())//how much now?
//	, virtMem(physMem-prev.physMem){}//what's the difference with the previous

