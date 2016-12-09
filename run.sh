#!/bin/bash
#SBATCH -J WF_QUEUE
#SBATCH -o WF_QUEUE.%J.stdout
#SBATCH -e WF_QUEUE.%J.stderr
#SBATCH -p main
#SBATCH -N 3
#SBATCH -t 00:10:00

#Uncomment following lines before running on caliburn
#cd $HOME/wfqueue
#sleep 3

set -e

make clean

find . -type f -name '*.csv' -delete

make

thread_count=(1 2 4 8 16 24 32 40 48 56 64 72)
ops_count=(100000 200000 400000 600000 800000 1000000 1200000 160000 2000000)
file_prefix="stats"

for count in "${ops_count[@]}"
do
    for tcount in "${thread_count[@]}"
    do
        ./main $tcount $count $count >> ${file_prefix}"_"${count}".csv" 
    done
done
