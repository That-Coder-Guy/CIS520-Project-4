#!/bin/bash

echo "nodes,tasks_per_node,total_tasks,time_sec"

for nodes in 1 2 4
do
    for tpn in 1 2 4 8
    do
        total=$((nodes * tpn))

        out=$(srun --constraint=mole \
            -N $nodes \
            --ntasks-per-node=$tpn \
            --mem-per-cpu=1G \
            ./build/hw4_mpi \
            ~eyv/cis520/wiki_dump.txt \
            4000 0)

        time=$(echo "$out" | grep "Elapsed" | awk '{print $3}')

        echo "$nodes,$tpn,$total,$time"
    done
done