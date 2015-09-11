mpic++ -fopenmp *.cpp && mpirun -np 1 --bind-to  none  ./a.out
