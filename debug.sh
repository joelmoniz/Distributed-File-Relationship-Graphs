mpic++ -fopenmp -fno-stack-protector -g *.cpp && mpirun -np 4 --bind-to none xterm -e gdb ./a.out
