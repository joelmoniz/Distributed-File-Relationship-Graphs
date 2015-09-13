mpic++ -fopenmp *.cpp && mpirun -np 3 --bind-to  none  ./a.out
