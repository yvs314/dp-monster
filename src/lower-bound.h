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
//#include<tuple>

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

using t_vArcs = std::vector<t_arc>;

std::string dumpArcs(const t_vArcs& inp)
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
t_vArcs mkDummyV_Arcs(ptag arcNumber)
{
    return t_vArcs(arcNumber, dummyArc);
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

inline std::pair<t_cost,t_vArcs> elCheapoLB(const ptag r //MSAP root; not in V 
                    , const t_bin& V // cities to be spanned by the MSAP
                    , const t_Instance& p
                    , const t_Direction& D) //FWD/BWD
//instead of thinking about INF, there are other ways to avoid 
{
    if (! (D == FWD)) exit(EXIT_FAILURE); //crash if not FWD; a STOPGAP
	if (V.test(p.cityof[r])) exit(EXIT_FAILURE);//crash if r belongs to V
//ptag startPt,endPt; t_bin fsbEntryV;
//if(D == FWD)//this is for proper direction, which could yield proper KCvH traveling deliveryman costs
//{// it's' FWD:
//    startPt=r;//start at the given vertex
//    fsbEntryV= getMin(V,p.ord,p.wkOrd);//we can only go r--> Min[V]=Min[1..n\setminus(K\cup\{r})], where (K,r) is a state
//    endPt=p.dim+1;//end at the terminal
//    //last arc is  from getMax[V] anyway
//}
//else{
//    startPt=0;//start at the base
//    fsbEntryV= getMin(V,p.ord,p.wkOrd);//it's still Min[V], we can only go 0 --> Min[V]
//    endPt=r;//end at r
//    //last arc is from getMax[V] anyway
//}


	/* it's FWD;
 * 1. get min-arc *exiting* r and entering V // only r--> MIN[V]
 * 2. get min-arcs *entering* vertices from V
 * 3. get min-arc *entering* the terminal from some vertex from V */
// no.1 get min-arc *exiting* r and entering V // only r--> MIN[V]
    t_vArcs sln = mkDummyV_Arcs(V.count()+1); t_cost out=0;
    //m\in \Min[V], x\in M_m; and a poor man's min-fold directly below
    foreach_elt(m, getMin(V,p.ord,p.wkOrd),p.dim )
            foreach_point(x, p.popInfo[m].pfirst, p.popInfo[m].plast)
                if (p.cost[r][x]< sln.front().arcCost)
                    sln.front()=t_arc{r,x,p.cost[r][x]};
    
//     out+=sln.front().arcCost;//add to the accumulator
    out = p.f.cAgr(out,sln.front().arcCost);
    //done with first arc; now find minimum |V|-1 arcs to V\setminus cityof[sln[0].vxTo
    
    ptag currVx=1; //how many proper (non-root, non-trm) vertices are processed, including the upcoming one
    const auto V1 = setMinus(V,BIT0 << p.cityof[sln.front().vxTo]);
//find min. arc to every l\in V1 from V1\setminus (those l sends to)
    foreach_elt(l,V1, p.dim)
    {
        foreach_point(y, p.popInfo[l].pfirst, p.popInfo[l].plast)
        {
            const auto into_l = setMinus(V1,p.ord[l].sends_to).reset(l);//reset(l):don't forget to remove l-->l
            foreach_elt(m,into_l,p.dim)
                foreach_point(x, p.popInfo[m].pfirst, p.popInfo[m].plast)
                    if (p.cost[x][y]< sln[currVx].arcCost)
                        sln[currVx]=t_arc{x,y,p.cost[x][y]};
        }
        out = p.f.cAgr(out,sln.at(currVx).arcCost); //add this min arc's cost to the accumulator
        //out+=sln[currVx].arcCost; //add this min arc's cost to the accumulator
        currVx++;//ok, processed vertex number l
    }
    //now find the last arc, which goes to the terminal p.popInfo[p.dim+1].pfirst from \Max[V]
    foreach_elt(m, getMax(V, p.ord, p.wkOrd),p.dim)
        foreach_point(x, p.popInfo[m].pfirst, p.popInfo[m].plast)
            if (p.cost[x][p.popInfo[p.dim+1].pfirst]< sln.back().arcCost)
                    sln.back()=t_arc{x,p.popInfo[p.dim+1].pfirst,p.cost[x][p.popInfo[p.dim+1].pfirst]};
    
    
    out = p.f.cAgr(out,sln.back().arcCost); //add the cost of the final V->\trm arc 
    //out += sln.back().arcCost; //add the final arc to the terminal
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
