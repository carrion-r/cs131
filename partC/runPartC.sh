#!/bin/bash
#
#$ -cwd
#$ -j y
#$ -S /bin/bash
#$ -M dcarrion@uci.edu
#$ -pe openmpi 16
#$ -o ./PartC.out
#
# Use modules to setup the runtime environment
module load sge 
module load gcc/5.2.0
module load openmpi/1.10.2
#
# Execute the run
# If extra command line args given, $1 is input file and $2 is NSLOTS
#
if [ $# -eq 0 ]
    then
        export NSLOTS=4
        mpirun -np $NSLOTS ./PartC jjbesavi_Example.txt
    else
        export NSLOTS=$2
        mpirun -np $NSLOTS ./PartC $1
fi