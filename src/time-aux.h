/*
(c) Yaroslav Salii & the Parliament of Owls, 2017+
License:sort of CC---if you go commercial, contact me

time-aux v.1.0 2017-01-30; added cond.cmpl, to swap localtime_s/_r, etc.

timing auxiliaries; for logging etc.

everything measured in seconds, that should be enough
well one reviewer wants ms, let's indulge
*/

#ifdef _WIN32
#define mylocaltime(tmp,input) localtime_s(tmp,input)
#define mygmtime(tmp,input) gmtime_s(tmp,input)
#endif

#ifdef __linux__
#define mylocaltime(tmp,input) localtime_r(input,tmp)
#define mygmtime(tmp,input) gmtime_r(input,tmp)
#endif

#ifndef TIME_AUX_H_
#define TIME_AUX_H_

#include<cmath>
#include<ctime>
#include<vector>
#include<iomanip>
#include<sstream>
#include<chrono> // for millisecond-precise measurements

#include"bitset-base.h" //for mtag

using myClock = std::chrono::steady_clock;//the simplest in use
using t_timeStamp = myClock::time_point; //time stamps
using t_msec = std::chrono::milliseconds;//milliseconds duration for more precise measurements

t_msec msecDuration(const t_timeStamp from, const t_timeStamp till)
{
	return std::chrono::duration_cast<t_msec>(till - from);
}
										 
//==========STOPWATCH======for timekeeping 
using t_stopwatch =
struct t_stopwatch
{
	//abs.time, start:calculation started, eopt:found optimal route
	time_t start;
	t_timeStamp recoveryDone;//to time solution recovery
	//timestamps. vlayerDone[0] doubles as DP start time, vlayerDone[dim] doubles as DP end time
	std::vector<t_timeStamp> vlayerDone;
	//rel.time, returns time in milliseconds 
	t_msec _rel_time(const mtag from, const mtag till) const
	{
		//return msecDuration(this->vlayerDone.at(till) - this->vlayerDone.at(from));
			return std::chrono::duration_cast<t_msec>
			(this->vlayerDone[till] - this->vlayerDone[from]);
	}

}; 

inline time_t getRelTime(const time_t from, const time_t till)
{	 return std::lround(std::difftime(from,till));	}

inline std::string mkTimeStamp(const time_t input)//YYYY/MM/DD HH:MM:SS
{
	struct tm* tmp = new struct tm; mylocaltime(tmp, &input); std::ostringstream foo;
	foo << std::put_time(tmp, "%Y/%m/%d %H:%M:%S");// std::string out(foo.str());
	delete tmp; return (foo.str());
}

//HH:MM:SS; modulo 1 day, sort of; ain't designed for more than 1 day-long jobs
inline std::string mkDuration(const time_t input)
{	//GMT for starting from zero
	struct tm* tmp = new struct tm; mygmtime(tmp, &input);
	std::ostringstream foo;
	foo << std::put_time(tmp, "%H:%M:%S");
	delete tmp; return (foo.str());
}

//from a duration in milliseconds, make a string formatted as HH:MM:SS.MSS
inline std::string mkMsecDurationFull(const t_msec inp)
{
	//shouldn't be more than 24 hours anyway, so not checking for days etc.
	std::chrono::hours hrs = 
		std::chrono::duration_cast<std::chrono::hours>(inp);
	std::chrono::minutes mns = 
		std::chrono::duration_cast<std::chrono::minutes>(inp % std::chrono::hours(1));
	std::chrono::seconds sec = 
		std::chrono::duration_cast<std::chrono::seconds>(inp % std::chrono::minutes(1));
	t_msec msec =
		std::chrono::duration_cast<t_msec>(inp % std::chrono::seconds(1));
	std::string sep = ":";
	std::ostringstream foo; 
	foo << std::setfill('0') << std::setw(2)<<hrs.count()<<sep
							 << std::setw(2)<<mns.count()<<sep
							 << std::setw(2)<<sec.count()<<"."
							 << std::setw(3)<<msec.count();
	return foo.str();
}
//SS.MSS
inline std::string mkMsecDurationBrief(const t_msec inp)
{
	std::chrono::seconds sec =
		std::chrono::duration_cast<std::chrono::seconds>(inp);
	t_msec msec =
		std::chrono::duration_cast<t_msec>(inp % std::chrono::seconds(1));
	std::string sep = ":";
	std::ostringstream foo;
	foo << sec.count() << "." << std::setfill('0')<< std::setw(3) << msec.count();
	return foo.str();
}

#endif


//t_msec layer_took(const mtag nlayer) const
//{
//	//filter layer no.0, just in case
//	return (nlayer > 0) ? (_rel_time(nlayer - 1, nlayer)) : (t_msec(0));
//}

//AN ATTEMPT AT std::chrono; aborted: must still use time_t for output
/*steady_clock may not be as precise but is guaranteed against
adjustments that might lead to unexpected negative time differences;
time goes ==steadily== forward */
//using clock = std::chrono::steady_clock;//the simplest
//using seconds = std::chrono::seconds;
//
//using t_sw2 =
//struct t_sw2
//{
//	//startedAt:program start time; doneRecovery: recovered the (opt) sln from BF
//	clock::time_point startedAt, doneRecovery;
//	/*records when is the vdoneLayer[l] was computed, including relative to previous
//	vdoneLayer[0] just might be used for timing input, etc;*/
//	std::vector<t_timeRec> vdoneLayer;
//};
//
//using t_timeRec =
//struct t_timeRec
//{
//	clock::time_point totalTime;
//	seconds relTime;//duration; since last layer, etc.
//
//	//starting point: take the current time, relTime is irrelevant
//	t_timeRec()
//		: totalTime(clock::now())
//		, relTime(0)
//	{
//	};
//
//	t_timeRec(const t_timeRec& prev)
//		: totalTime(clock::now())
//		, relTime((totalTime - prev.totalTime).count())
//	{
//	};
//};

