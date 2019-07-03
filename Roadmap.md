# Roadmaps, Results, and Papers

## Paper 1. Are 3TB an overkill? Shared-memory parallelism in DP/DPBB for TSP-PC and its bottelneck version. 
### Stages
1. `DONE` OpenMP multithreaded implementation of DP for TSP-PC that scales _well enough_ 
 2. `WE'RE HERE` Multithreaded Bounded DP (DPBB) for TSP-PC, with best available upper bounds. Should _just work_, must be tested; see below. (2.1) To better compare DPBB with DP, make `dry-run` DPBB: run LB for each state, count fathomed states, but retain fathomed states. This comparison lets one see how well does each LB heuristic perform against _worst case_ of not fathoming anything at all.
 3. Test DP & DPBB on Bottleneck TSP-PC (`-t BTSP`). Testing DPBB would require some upper bounds, obtainable through Restricted DP (`-H [1,10,100,1000,100000]`) in reasonable time
 4. `DONE` ~Scratch~  Close `kro124p4.sop` (most probably through DPBB, but these 3TB, with the _new main data structure_, could take it as well)
 5. PROFIT! If we close something, the paper might make Q1 in, say, _Computers & Operations Research_. Actually, there's an [_Optimization Methods and Software_](https://www.scimagojr.com/journalsearch.php?q=28538&tip=sid) issue connected with MOTOR-2019. I'd rather we push _shared-memory_ results there, along with the general framework for testing DPBB, thereby saving better LBs and _distributed_ parallelization for sweet _C&OR_.
 6. ~option: add SOPLIB to consideration. Larger scale (100–600 cities), slightly different input format (just the matrix). YS: generate the missing parts of TSPLIB format for these, make complexity estimates~
 7. ~brutal option: brute-force improve _lower bounds_ for really hard problems through DPBB~
 
 ### Experiment script
 1. Rerun _Restricted DP_ for `H=1,10,100,1000,10000,100000` with `-d FWD` and `-d BWD` for `-t TSP` and `-t BTSP` on URAN, 3 runs for each parameter set. Collect (a) average run time; (b) values; (c) average memory use in MB. Run single-threaded, multi-threaded heuristic is a subject for another paper since this heuristic requires sorting.
 2. Test _shared-memory_ scaling for DP, compare FWD/BWD, TSP/BTSP; logarithmic steps are fine, URAN or other machines at your discretion. I expect no significant difference in scaling for these. There is no sense to work with instances that are either too easy or too hard, so let's get only those (original memory use above 100MB): `ESC25, ft70.4, kro124p4, p43.3, rbg174b, rbg253a, ry48p.3`. Collect (a) average run times; (b) values for `-t BTSP`; (c) average memory use in MB; (d) total states processed.
 3. Test _shared-memory_ scaling for DPBB, compare FWD/BWD, TSP/BTSP. Compare with scaling for DP. I expect DPBB to scale "better" than DP, i.e., say, DP will stop gaining performance at 16 cores whereas DPBB will stop at 32; but, for unlucky instances, DPBB at 32 cores will not be faster than DP at 16. Same instance list `ESC25, ft70.4, kro124p4, p43.3, rbg174b, rbg253a, ry48p.3`. In the `-t TSP` case, the ``--UB`` values are known ([SOP-UB.csv](https://github.com/yvs314/dp-monster/blob/master/SOP-UB.csv)). In the bottleneck case `-t BTSP`, these come from the best of __step 1__ and __step 2__. Collect (a) average run times; (b) values for `-t BTSP`; (c) average memory use in MB; (d) total states processed.
 4. Attempt to attack all instances through DPBB, full speed ahead, as many cores as found to help at __step 3__. Again, both `-d FWD` and `-d BWD`, both `-t TSP` and `-t BTSP`. See __step 3__ on ``--UB`` values. No need for multiple reruns here.
 
 ### 2. Multithreaded Bounded DP. Experiment scripts
 1. TODO: refit `run.py` to support _per-instance_ `--UB` values. Here's two sample launch lines
 ```
 dpm ESC07.sop -d FWD --UB 2125
 dpm ESC25.sop -d FWD --UB 1681
 ```
 My caveman run script  borrowed them (`grep $INSTANCE_NAME | cut -f2 -d';'`) from 
 [SOP-UB.csv](https://github.com/yvs314/dp-monster/blob/master/SOP-UB.csv), which I've just put in the root folder. Might want to relocate it to `/data`
 
 2. TODO: test scaling and general results with _best known_ upper bounds (as supplied in  [SOP-UB.csv](https://github.com/yvs314/dp-monster/blob/master/SOP-UB.csv))
 
 **Collecting run time in DPBB** DPBB does not always _recover the solution_ (see the point above). To better compare run times, we should switch `agg.py` to collect, instead of `BF + RECOVERY DURATION IN SECONDS:`, the time listed at `TOTAL DURATION IN SECONDS:` __in all cases, both DP and DPBB__.
 
 **Collecting `VALUE:` in DPBB** Look for `VALUE:` in `*.dump`. DPBB may either _find a solution_ (output is the same as for ordinary DP) or _prove_ given `--UB XXX` to be a _lower_ bound. In this case, there is no solution to be had, and `.dump` has dummy value line
 ```
 VALUE:-1
 ```
 and also the “all states fathomed” message
 ```
 DPBB_ALL_STATES_FATHOMED: impossible to get better than UB=1675
 ```
 **Collecting memory usage** Look for `RAM USAGE AT LAST LAYER:` in `*.log`. It is tilde-separated, in KB, MB, or GB as necessary, with 1GB=1024MB, 1MB=1024KB, and 1KB=1024B, all good and binary. 
 
 **Collecting total states procesed** Look for `TOTAL STATES PROCESSED:` in `*.log`.
 
 
 
