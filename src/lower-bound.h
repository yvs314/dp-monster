/*
(c) Yaroslav Salii & the Parliament of Owls, 2019+
License:sort of CC---if you go commercial, contact me

lower-bound v.1.0 2019-01-24; 
                   v.1.1 2019-01-28; added el-Cheapo LB; TODO: replace + with ExtMt or whatever
compute lower bounds on problem instances
NB! v.1.0 is TSP(-PC)-only, might not handle BTSP etc.

=TODO:dumb Edmonds/Chu--Liu LB: MSAP with prohibited arcs that directly contradict PCs

*/

#ifndef LB_H_
#define LB_H_

#include"instance.h"

#include<forward_list>

//=====AUX==DATA==STRUCTURES=ETC.================/
using t_arc  //I'd rather have its fields immutable, but const fields wreak havok in some programming patterns
= struct t_arc
{
   ptag vxFrom;
   ptag vxTo;
   t_cost arcCost;

   std::string to_string() const
   {
	   std::ostringstream tmp; auto colSep = ";";
	   tmp << vxFrom << colSep << vxTo << colSep << arcCost;
	   return tmp.str();
   }
};

//forward_list: minimum overhead
using t_flArcs = std::forward_list<t_arc>;

std::string dumpArcs(const t_flArcs& inp)
{
	std::string out="";
	for (auto arc : inp)
	{
		out += arc.to_string(); out += '\n';
	}
	out.pop_back();//suppress the dangling newline
	return out;
}

const t_arc dummyArc {0,0,INF};

//initialize all to dummyArc
t_flArcs mkDummyV_Arcs(ptag arcNumber)
{
    return t_flArcs(arcNumber, dummyArc);
}
//-------------------------------------------/

//=====elCheapo-LB===========================/
/*
CAVEAT: non-GTSP, FWD-only
  not even MSAP: just a node-greedy solution; still a valid lower bound, and blazingly fast
    0. do not take the precedence-proscribed edges; either cost=INF or direct consideration of who sends where
    1. ignore the terminal for now; that's still a lower bound in most cases
    2. think about solution output / through Boost.Graph
    3. direction. FWD: path root r-->\trm; BWD: path base 0-->r;
    BOTH: r=x, V= \rgn\setminus K 
 */
/*
 * REFIT scenario:
 * 1. Kill GTSP-like foreach_point, for convenience. GTSP-compatible LBs are a totally different matter anyway
 * 2. Add transparent BWD support (might decrease performance
 * 3. Remove failsafe test (r\notin V)
 * NOTE: varying direction in elCheapo. If there's no time/seq-dependence,
 * the direction of MSAP traversal is irrelevant, so we can just choose the best one
 */

inline std::pair<t_cost,t_flArcs> elCheapoLB(const ptag r //MSAP root; not in V
                    , const t_bin& V // cities to be spanned by the MSAP
                    , const t_Instance& p
                    , const t_Direction& D) //FWD/BWD
{

ptag startPt,endPt;
t_bin V_Min=getMin(V,p.ord,p.wkOrd); //can receive  arcs directed from startPt
t_bin V_Max=getMax(V,p.ord,p.wkOrd);//can be the source of arcs directed to endPt

if(D == FWD)//this is for proper direction, which could yield proper KCvH traveling deliveryman costs
    {// it's FWD:
    startPt=r;//start at the given vertex
    endPt=p.dim+1;//end at the terminal
    }
else// D == BWD
{//it's BWD:
    startPt=0;//start at the base
    endPt=r;//end at r
}

//we'll have one arc into  every vertex from V and one into endPt, V.count()+1
t_flArcs sln {}; //start as empty solution list
t_cost out=0;//
    //find least-cost arcs into each city x  in V_Min, which may start at V\setminus\{v\}\cup\{startPt\}
    foreach_elt(x, V,p.dim )
    {
        t_arc minArc_y_to_x = dummyArc;
        t_bin V_from = (V_Min.test(x)) ? (setMinus(V, BIT0 << x).set(startPt)) : (setMinus(V, BIT0 << x));
        foreach_elt(y, V_from, p.dim)
        {
            if (p.cost[y][x] < minArc_y_to_x.arcCost)
                minArc_y_to_x = t_arc{y, x, p.cost[y][x]};
        }
        //now we've got the minimal arc from y to x; record it
        sln.push_front(minArc_y_to_x);
        out=p.f.cAgr(out,minArc_y_to_x.arcCost);
    }
    //finally, find and add the min-arc to endPt
    t_arc minArc_x_to_endPt=dummyArc;
    foreach_elt(x, V_Max, p.dim)
    {
        if (p.cost[x][endPt] < minArc_x_to_endPt.arcCost)
            minArc_x_to_endPt = t_arc{x, endPt, p.cost[x][endPt]};
    }
    //now we've got the minimal arc from y to x; record it
    sln.push_front(minArc_x_to_endPt);
    out=p.f.cAgr(out,minArc_x_to_endPt.arcCost);
    
    return std::make_pair(out,sln); 
}
//---------------------------------------------------/

//=========NAMECALLING,=CODES,=ETC.===============/
//getLowerBoundCode
//say, elCheapo is CHP; would extend later


//---------------------------------------------------/
#endif


//====BIT=====BUCKET===========//


/* computes Edmonds-MSAP LB; MSAP is Minimum Spanning Arborescence P
    1. ignore the terminal for now; that's still a lower bound in most cases
    2. think about solution output
    3. direction. FWD: path *from* root r; BWD: path *to* root r; 
    BOTH: r=x, V= \rgn\setminus K 
 */
//compute Edmonds-MSAP LB: wrapper;
// inline t_cost edmLB(const ptag x //agent's position; interface into K
//                     , const t_bin& K // agent's task set
//                     , const t_Instance& p
//                     , const t_Direction D) //FWD/BWD
// {
//     if (! (D == FWD)) exit(EXIT_FAILURE); //crash if not FWD; STOPGAP
//     /* OK, we're going forward now. Find MSAP with root x over the set \rgn\setminus K
//      */
//     
//     
//     return 0; // "undefined" STOPGAP
// }

/*
    std::tuple <mtag, ptag, t_cost> bestArcFromRoot (0,0,INF);
    //mtag best_m; ptag best_x; t_cost fromRoot_to_x=INF;// x\in V, for r--> x arc
    //m\in \Min[V], x\in M_m; and a poor man's min-fold directly below
    foreach_elt(m, getMin(V,p.ord,p.wkOrd),p.dim )
            foreach_point(x, p.popInfo[m].pfirst, p.popInfo[m].plast)
            {
                (p.cost[r][x]< std::get<2>(bestArcFromRoot))?(bestArcFromRoot={m,x,p.cost[r][x]});
            }
    const auto V1 = setMinus(V,BIT0 << std::get<0>(bestArcFromRoot));*/

//const auto V1 = setMinus(V, BIT0 << p.cityof[sln.front().vxTo]);
