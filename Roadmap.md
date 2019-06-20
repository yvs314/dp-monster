# Roadmaps, Results, and Papers

## Paper 1. Are 3TB an overkill? Shared-memory parallelism in DP/DPBB for TSP-PC and the like. 
 1. `DONE` OpenMP multithreaded implementation of DP for TSP-PC that scales _well enough_ 
 2. `WE'RE HERE` Multithreaded Bounded DP (DPBB) for TSP-PC, with best available upper bounds. Should _just work_, must be tested; see below. (2.1) To better compare DPBB with DP, make `dry-run` DPBB: run LB for each state, count fathomed states, but retain fathomed states. This comparison lets one see how well does each LB heuristic perform against _worst case_ of not fathoming anything at all.
 3. Test DP & DPBB on Bottleneck TSP-PC (`-t BTSP`). Testing DPBB would require some upper bounds, obtainable through Restricted DP (`-H [1,10,100,1000,100000]`) in reasonable time
 4. `DONE` ~Scratch~  Close `kro124p4.sop` (most probably through DPBB, but these 3TB, with the _new main data structure_, could take it as well)
 5. PROFIT! If we close something, the paper might make Q1 in, say, _Computers & Operations Research_. Actually, there's an [_Optimization Methods and Software_](https://www.scimagojr.com/journalsearch.php?q=28538&tip=sid) issue connected with MOTOR-2019. I'd rather we push _shared-memory_ results there, along with the general framework for testing DPBB, thereby saving better LBs and _distributed_ parallelization for sweet _C&OR_.
 6. option: add SOPLIB to consideration. Larger scale (100–600 cities), slightly different input format (just the matrix). YS: generate the missing parts of TSPLIB format for these, make complexity estimates 
 7. brutal option: brute-force improve _lower bounds_ for really hard problems through DPBB
 
 ### 2. Multithreaded Bounded DP. Experiment scripts
 1. TODO: refit `run.py` to support _per-instance_ `--UB` values. Here`s two samples launch lines
 ```
 dpm ESC07.sop -d FWD --UB 2125
 dpm ESC25.sop -d FWD --UB 1681
 ```
 My caveman run script  borrowed them (`grep $INSTANCE_NAME | cut -f2 -d';'`) from 
 [SOP-UB.csv](https://github.com/yvs314/dp-monster/blob/master/SOP-UB.csv), which I've just put in the root folder. Might want to relocate it to `/data`
 
 2. TODO: test scaling and general results with _best known_ upper bounds (as supplied in  [SOP-UB.csv](https://github.com/yvs314/dp-monster/blob/master/SOP-UB.csv))
 
 **Special mention**: DPBB may either _find a solution_ (output is the same as for ordinary DP) or _prove_ given `--UB XXX` to be a _lower_ bound. In this case, there is no solution to be had, and `.dump` has dummy value line
 ```
 VALUE:-1
 ```
 and also the “all states fathomed” message
 ```
 DPBB_ALL_STATES_FATHOMED: impossible to get better than UB=1675
 ```
 
 
