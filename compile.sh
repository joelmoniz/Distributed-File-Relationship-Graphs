mpic++ -fopenmp -fno-stack-protector *.cpp && mpirun -np 3 --bind-to  none  ./a.out .
