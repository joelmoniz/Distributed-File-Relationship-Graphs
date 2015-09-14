mpic++ -fopenmp *.cpp && mpirun -np 4 --bind-to  none  ./a.out
