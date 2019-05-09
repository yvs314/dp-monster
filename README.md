# DP-Monster
Dynamic programming and derived solvers, mostly for TSP with precedence constraints. 

#### Some abbreviations
* DP::= dynamic programming (kudos to Bellman, Held, and Karp); PCs treated derived in (Salii, 2019) from (Lawler, 1979) and (Chentsov, Chentsov, 2004)
* DPBB::= DP hybridized with Branch-and-Bound scheme, also “bounded DP” (kudos to Morin and Marsten)
* hRtDP:: restricted DP, a heuristic (kudos to Malandraki and Dial)
---
* TSP-PC::= Traveling Salesman Problem with Precedence Constraints
* SOP::= Sequential Ordering Problem (same as TSP-PC)
* BTSP:: Bottleneck TSP, with `minmax` objective function, instead of `minsum` as in ordinary TSP
* PCs::= Precedence Constraints
* UB::= Upper Bound





## C++ core's interface
Say its executable is `dpm`
```
$dpm
usage: dpm filename.sop [--noP] [-H breadth] [-d FWD | BWD] [-t TSP | BTSP | TD_TSPb | etc] [--UB upper_bound]
```
Basically, it works like this:
1. Attempt to open _first_ argument as input file (`std::ifstream`). Fail miserably if it can't be read, cry foul into `std::cerr`.
2. Read _other_ arguments if present (if not, default to `-d FWD -t TSP`). Fail if they conflict or make no sense (parse-args.h); 
3. Read the _input file_ as though it is a TSPLIB-formatted instance of Sequential Ordering Problem (SOP). Other formats may be added at some point in future; let's say they may be differentiated by extension.
4. Do the ~monkey~ computation as specified. 
    * Generate the `.log` and `.dump` output file names (see **Naming conventions** below)
    * Log the process step-by-step in the `.log` file. Log some total and final data there too
    * When it's done, dump the solution into the `.dump` file, in particular, a line `VALUE:XXXX`. 
       * Special case for DPBB: there may be _no_ solution, but there will be a `.dump` with a line `DPBB_ALL_STATES_FATHOMED: impossible to get better than UB=XXXX`

### Naming conventions
A _solution_ is characterized by
* INP::= input file name, first argument of `dpm`
* DRN::= solution direction `-d`
   * FWD:: forward (default), like Held and Karp's DP (Held, Karp, 1962)
   * BWD:: backward, like Bellman's DP (Bellman, 1962)
* TYPE::= problem type `-t`
   * TSP::= _sum_ travel cost aggregation, TSP-PC
   * BTSP::= _max_ travel cost aggregation, BTSP-PC
* MTH::= solution method code
   * DP::= _exact_ DP 
   * hRtDP::= _heuristic_ Restricted DP
      * H::= the sole parameter of hRtDP, a _natural number_ `-H XXXX`
   * DPBB::= _exact_ Bounded DP
      * UB::= 1st parameter of DPBB, a known upper bound on _solution value_ `--UB XXXX`
      * LB::= 2nd parameter of DPBB, a code of _method_ used to obtain the _lower bound_ 
         * CHP::= “el Cheapo,” a very dumb graph-based lower bound; used by default, FWD-only
     


## Caveats
* input file name can't have (unescaped) spaces

### Merge-into-main checks for the C++ core:

#### Check *build* parameters before committing
* monster-main.cpp::`int debugRun=0;` Unless it is **zero**, no command line arguments are read, not even _input file name_. If it is zero, say `This is not a drill.`
* bitset-base.h::`const uint16_t refdim = 384;` Maximum number of cities; must be at least 380 for all TSPLIB-SOP to fit. Runtime checks if the given input fits, otherwise it cries foul `std::cerr << getName(input) << "max dimension exceeded: "<< getDim(input) << "/" << refdim << "\n";` and crashes with `exit(EXIT_FAILURE);`

