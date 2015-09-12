mpic++ -fopenmp *.cpp && mpirun -np 2 --bind-to  none  ./a.out
