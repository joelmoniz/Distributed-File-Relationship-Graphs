mpic++ -fopenmp -g *.cpp && mpirun -np 3 --bind-to none xterm -e gdb ./a.out .
