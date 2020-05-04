import csv #output
#t-SOPLIB06 file names, sorted
#*.1.sop suppressed: got no precedence constraints
ns = [#'R.200.100.1.sop',
 'R.200.100.15.sop',
 'R.200.100.30.sop',
 'R.200.100.60.sop',
 'R.200.1000.1.sop',
 'R.200.1000.15.sop',
 'R.200.1000.30.sop',
 'R.200.1000.60.sop',
# 'R.300.100.1.sop',
 'R.300.100.15.sop',
 'R.300.100.30.sop',
 'R.300.100.60.sop',
 #'R.300.1000.1.sop',
 'R.300.1000.15.sop',
 'R.300.1000.30.sop',
 'R.300.1000.60.sop',
# 'R.400.100.1.sop',
 'R.400.100.15.sop',
 'R.400.100.30.sop',
 'R.400.100.60.sop',
# 'R.400.1000.1.sop',
 'R.400.1000.15.sop',
 'R.400.1000.30.sop',
 'R.400.1000.60.sop',
# 'R.500.100.1.sop',
 'R.500.100.15.sop',
 'R.500.100.30.sop',
 'R.500.100.60.sop',
# 'R.500.1000.1.sop',
 'R.500.1000.15.sop',
 'R.500.1000.30.sop',
 'R.500.1000.60.sop',
# 'R.600.100.1.sop',
 'R.600.100.15.sop',
 'R.600.100.30.sop',
 'R.600.100.60.sop',
# 'R.600.1000.1.sop',
 'R.600.1000.15.sop',
 'R.600.1000.30.sop',
 'R.600.1000.60.sop',
# 'R.700.100.1.sop',
 'R.700.100.15.sop',
 'R.700.100.30.sop',
 'R.700.100.60.sop',
# 'R.700.1000.1.sop',
 'R.700.1000.15.sop',
 'R.700.1000.30.sop',
 'R.700.1000.60.sop']
#=SEC.==PARAMETERS==========
#set the principal parameters
cores = [16] #16 should be fine for plain DP
drns=['FWD'] #just FWD, should be no significant difference
types=['TSP'] #just TSP is enough
runs = 1 #run once, just see if it works
pref = "exp6.1-DP-FWD-TSP" #experiment name
memDefault="251G" #request as much mem as we can

#=SEC.==TASK==FABRICATION===
#big whopping List Comprehension, one *dict* entry for each parameter set
to_csv = [dict(
    prefix=pref+"_%s" % nThr,
    task=n,
    threads=nThr,
    d=d,t=t,
    docker=0, #no need for docker at this time
    slurm=1,
    mem=memDefault,
    part='apollo',
    time='2:0:0' #up to 2h, should be enough for DP to finish or die on mem limit 
)for n in ns
 for nThr in cores 
 for t in types 
 for d in drns 
]
#=SEC.==CSV==OUTPUT=========
csv_file="%s.csv" % pref #.csv output named by the prefix
csv_columns = to_csv[0].keys() #columns just as in the dict-list above
#now write it 
with open(csv_file, 'w', newline='') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=csv_columns,delimiter=';')
    writer.writeheader()
    writer.writerows(to_csv)
