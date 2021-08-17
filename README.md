# DP-Monster
Dynamic programming and derived solvers, mostly for TSP with precedence constraints. 

If you use this software, please cite this repository and [(Salii, Sheka, 2020)](https://doi.org/10.1080/10556788.2020.1817447)
```
@article{salii2020improving,
	title={Improving dynamic programming for travelling salesman with precedence constraints: parallel {Morin}--{Marsten} bounding},
	author={Salii, Yaroslav V and Sheka, Andrey S},
	journal={Optimization Methods and Software},
	pages={1--27},
	year={2020},
	url={https://doi.org/10.1080/10556788.2020.1817447},
	publisher={Taylor \& Francis}
}
```

#### Mini-Glossary
* DP::= Dynamic Programming for traveling salesman problem (by Bellman / Held and Karp); adaptations for _Precedence Constraints_ derived in [(Salii, 2019)](https://doi.org/10.1016/j.ejor.2018.06.003), building upon [(Lawler, 1979)](https://www.persistent-identifier.nl/urn:nbn:nl:ui:18-9663) and [(Chentsov, Chentsov, 2004+)](http://mi.mathnet.ru/eng/timm597)
* DPBB::= DP hybridized with Branch-and-Bound scheme, also “Bounded DP” [(Morin, Marsten, 1976)](https://doi.org/10.1287/opre.24.4.611); adaptations for precedence contraints in [(Bianco, Mingozzi, Ricciardelli, Spadoni, 1994)](https://doi.org/10.1080/03155986.1994.11732235) and [(Salii, Sheka, 2020)](https://doi.org/10.1080/10556788.2020.1817447)
* hRtDP:: Restricted DP, a heuristic from [(Malandraki, Dial, 1996)](https://doi.org/10.1016/0377-2217(94)00299-1), adapted to Generalized TSP-PC in [(Salii, 2015)](http://ceur-ws.org/Vol-1513/paper-10.pdf)
---
* TSP-PC::= Traveling Salesman Problem with Precedence Constraints
* SOP::= Sequential Ordering Problem (same as TSP-PC)
* BTSP::= Bottleneck TSP, with `minmax` objective function, instead of `minsum` as in ordinary TSP
* PCs::= Precedence Constraints
* UB::= Upper Bound





## C++ core's interface
Say its executable is `dpm`
```
$dpm
usage: dpm filename.sop [--noP] [-H breadth] [-d FWD | BWD] [-t TSP | BTSP | TD_TSPb | etc] [--UB upper_bound]
```
When you call it, it acts as described below. See the implementation in [monster-main.cpp](https://github.com/yvs314/dp-monster/blob/master/src/monster-main.cpp).
1. Attempt to open _first_ argument as input file (`std::ifstream`). Fail miserably if it can't be read, cry foul into `std::cerr`.
2. Read _other_ arguments if present (if not, default to `-d FWD -t TSP` with exact DP). Fail if they conflict or make no sense (parse-args.h); 
3. Read the _input file_ as though it is a TSPLIB-formatted instance of Sequential Ordering Problem (SOP). Other formats, if implemented, ought to have different extensions.
4. Do the ~monkey~ computation as specified. 
   * Generate the `.log` and `.dump` output file names (see **Naming conventions** below)
    * Log the process step-by-step in the `.log` file. Log some total and final data there too
    * When it's done, dump the solution into the `.dump` file, in particular, a line `VALUE:XXXX`. 
       * Special case for DPBB: there may be _no_ solution, but there will be a `.dump` with a line `DPBB_ALL_STATES_FATHOMED: impossible to get better than UB=XXXX`

### Naming conventions
A _solution_ is characterized by
* INP::= input file name, first argument of `dpm`
* TYPE::= problem type `-t`
   * TSP::= _sum_ travel cost aggregation, TSP-PC
   * BTSP::= _max_ travel cost aggregation, BTSP-PC
   * TD_TSPf::= _sum_ aggregation, with time-dependent travel costs, multiplied through _traveling deliveryman_ coefficients as in TD-SOP [(Kinable, Cire, van Hoeve, 2017)](https://doi.org/10.1016/j.ejor.2016.11.035). _Must_ be used with `-d FWD`, fails otherwise
   * TD_TSPb::= _sum_ aggregation, with time-dependent travel costs, multiplied through _traveling deliveryman_ coefficients as in TD-SOP [(Kinable, Cire, van Hoeve, 2017)](https://doi.org/10.1016/j.ejor.2016.11.035). _Must_ be used with `-d BWD`, fails otherwise
* DRN::= solution direction `-d`
   * FWD:: forward (default), like Held and Karp's DP (Held, Karp, 1962)
   * BWD:: backward, like Bellman's DP (Bellman, 1962)
* MTH::= solution method code
   * DP::= _exact_ DP 
   * hRtDP::= _heuristic_ Restricted DP
      * H::= the sole parameter of hRtDP, a _natural number_ `-H XXXX`
   * DPBB::= _exact_ Bounded DP
      * UB::= 1st parameter of DPBB, a known upper bound on _solution value_ `--UB XXXX`
      * LB::= 2nd parameter of DPBB, a code of _method_ used to obtain the _lower bound_ 
         * CHP::= “el Cheapo,” a very dumb graph-based lower bound; no other LBs currently implemented

#### Solution name generation
1. Intersperse top-layer variables with hyphens `-`, to get `INP-TYPE-DRN-MTH`
2. MTH has sub-variables (method _parameters_), intersperse them with hyphens `-` too. Specific samples:
   * `INP-TYPE-DRN-DP`, plain exact DP has no further marameters
   * `INP-TYPE-DRN-hRtDP-XXXX`, where XXXX is a _natural number_
   * `INP-TYPE-FWD-DPBB-CHP`, the only implemented case for DPBB, see **Caveats** below


## Caveats
* all `/data` is stored with **git LFS**
* UB _value_ is not reflected in naming; intended scheme is e.g. `INP-TYPE-FWD-DPBB-CHP-XXXX` for `--UB XXXX`
* `--UB XXXX` has precedence over `-H YYYY`: the latter is ignored and DPBB is launched. 
* DPBB is only implemented for `-t [TSP | BTSP]`
* MTH sub-variables are interspersed with hyphens `-`; it sure helps that MTH is the _last_ top-layer variable, but I'd better switch _sub-variables_ interspersing to _tildes_ `~` some day
* input file names can't have (unescaped) spaces

## Pre-commit checks for the C++ core:
* monster-main.cpp::`int debugRun=0;` Unless it is **zero**, no command line arguments are read, not even _input file name_. If it is zero, the runtime prints the following to stderr: `This is not a drill.`
* bitset-base.h::`const uint16_t refdim = 384;` Maximum number of cities; must be at least 380 for all TSPLIB-SOP to fit. Runtime checks if the given input fits, otherwise it cries foul `std::cerr << getName(input) << "max dimension exceeded: "<< getDim(input) << "/" << refdim << "\n";` and crashes with `exit(EXIT_FAILURE);`

# Acknowledgements
This code uses `flat_hash_map` from [Abseil C++ Library](https://abseil.io/about) `parallel_hash_map` by [Gregory Popovich](https://github.com/greg7mdp/parallel-hashmap). 

The venerable [TSPLIB](https://doi.org/10.1287/ijoc.3.4.376) is by George Reinelt, with [SOP](https://doi.org/10.1023/A:1008779125567) instances by Norbert Ascheuer and Laureano Escudero.

**Yaroslav Salii** @yvs314 is the principal author of this solver and the methods it uses, as presented in [(Salii, 2019)](https://doi.org/10.1016/j.ejor.2018.06.003) and [(Salii, Sheka, 2020)](https://doi.org/10.1080/10556788.2020.1817447). 

**Andrey Sheka** @AndreySheka contributed the build script, batch execution and output data collation scripts, integration with `flat_hash_map` and `parallel_hash_map`, “garbage collect” improvements to the parallel scheme, Docker container integration, and time on Amazon AWS `x1.32xlarge` machine.

This solver was written in 2016–2019 when the authors were with [_Krasovskii Institute of Mathematics and Mechanics_ UB RAS](https://www.imm.uran.ru/) in Yekaterinburg, Russia.