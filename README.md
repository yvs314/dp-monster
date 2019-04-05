# dp-monster
Dynamic programming and derived solvers, mostly for TSP with precedence constraints

## Merge-into-main checks:

### Check monster-main.cpp to feature
~~~
int debugRun=0; 
~~~
Not 1. Not 2. Not **anything**, just 0, ZERO!

### Check bitset-base.h:: refdim = 384 or more
~~~
//max intended size is 384=64*6; max size in TSPLIB-SOP is 380
const uint16_t refdim =384;
//const uint16_t refdim = 8;
~~~
