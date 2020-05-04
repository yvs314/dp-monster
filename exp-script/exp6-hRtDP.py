import csv #output
#t-SOPLIB06 file names, sorted
ns = ['R.200.100.1.sop',
 'R.200.100.15.sop',
 'R.200.100.30.sop',
 'R.200.100.60.sop',
 'R.200.1000.1.sop',
 'R.200.1000.15.sop',
 'R.200.1000.30.sop',
 'R.200.1000.60.sop',
 'R.300.100.1.sop',
 'R.300.100.15.sop',
 'R.300.100.30.sop',
 'R.300.100.60.sop',
 'R.300.1000.1.sop',
 'R.300.1000.15.sop',
 'R.300.1000.30.sop',
 'R.300.1000.60.sop',
 'R.400.100.1.sop',
 'R.400.100.15.sop',
 'R.400.100.30.sop',
 'R.400.100.60.sop',
 'R.400.1000.1.sop',
 'R.400.1000.15.sop',
 'R.400.1000.30.sop',
 'R.400.1000.60.sop',
 'R.500.100.1.sop',
 'R.500.100.15.sop',
 'R.500.100.30.sop',
 'R.500.100.60.sop',
 'R.500.1000.1.sop',
 'R.500.1000.15.sop',
 'R.500.1000.30.sop',
 'R.500.1000.60.sop',
 'R.600.100.1.sop',
 'R.600.100.15.sop',
 'R.600.100.30.sop',
 'R.600.100.60.sop',
 'R.600.1000.1.sop',
 'R.600.1000.15.sop',
 'R.600.1000.30.sop',
 'R.600.1000.60.sop',
 'R.700.100.1.sop',
 'R.700.100.15.sop',
 'R.700.100.30.sop',
 'R.700.100.60.sop',
 'R.700.1000.1.sop',
 'R.700.1000.15.sop',
 'R.700.1000.30.sop',
 'R.700.1000.60.sop']
#=SEC.==PARAMETERS==========
#set the principal parameters
cores = [1] #hRtDP is fine with a single core
drns=['FWD','BWD'] #both directions
types=['TSP','BTSP'] #both problem types
runs = 3 #run each instance 3 times
pref = "exp6-hRtDP" #experiment name
#set hRtDP's parameter range
Hs = [10**x for x in range(6)] #1,10^1,...,10^5
memDefault="2G" #should be enough even for R.700* at H=10^5

#=SEC.==TASK==FABRICATION===
#big whopping List Comprehension, one *dict* entry for each parameter set
to_csv = [dict(
    data_dir="../data/t-SOPLIB06",
    prefix=pref+"_%s" % nThr,
    task=n,
    threads=nThr,
    d=d,t=t,H=H,
#    docker=1,
    slurm=1,
    mem=memDefault,
    part='apollo',
    time='1:0:0' #should be enough for hRtDP 
)for n in ns
 for nThr in cores 
 for t in types 
 for d in drns 
 for H in Hs]
#=SEC.==CSV==OUTPUT=========
csv_file="%s.csv" % pref #.csv output named by the prefix
csv_columns = to_csv[0].keys() #columns just as in the dict-list above
#now write it 
with open(csv_file, 'w', newline='') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=csv_columns,delimiter=';')
    writer.writeheader()
    writer.writerows(to_csv)
