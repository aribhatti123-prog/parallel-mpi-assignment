#!/bin/sh
#SBATCH --job-name=MPI_assignment          # Job name
#SBATCH --output=MPI_test_%j.log           # Standard out and error log
#SBATCH --mail-type=NONE                   # Specify when to mail (NONE, BEGIN, END, FAIL, ALL)
#SBATCH --mail-user=uid@york.ac.uk         # NB change uid to your username if wanting to send mail

#need ONE of the following two lines:
#SBATCH --account=PET-TEACH-2026         # priority queue for class work
##SBATCH --account=dept-proj-year           # specify your project account if NOT doing class work

#customise these according to job size and time required:
#SBATCH --ntasks=4                         # Run 4 MPI tasks...
#SBATCH --cpus-per-task=1                  # ...with each task using 1 core
#SBATCH --time=00:05:00                    # Time limit hrs:min:sec

#actual executable info now:
EXEC="./a.out"

#tell user what is going on:
echo My working directory is `pwd`
echo Running job on host:
echo -e '\t'`hostname` at `date`
echo -e '\t'using $SLURM_NTASKS MPI tasks
echo

srun --mpi=pmix -n ${SLURM_NTASKS} $EXEC

echo
echo Job completed at `date`

