mpic++ -fopenmp -fno-stack-protector *.cpp && mpirun -np 5 --bind-to  none  ./a.out .
