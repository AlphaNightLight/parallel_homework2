# Parallel Computing Homework 2

## Files in this repository
In this repository you will find the following files:
- compile.sh: a bash script that compiles all the source codes, putting the executables into two *bin* subfolders,
one for *parallel* folder and another one for *serial* folder.
- benchmark.sh: a bash script that run all the executables in the *bin* subfolders, as well as initializing the report files that
will be put in a *report* folder.
- prepare_plot.sh: a bash script that creates the *plot* folder and prepares it to recieve the plots.
- clean.sh: a bash script that delete the executables as well as the reports. It is used to clean the workspace.
- plot_results.m: a GNU octave script that creates the plots form the report files and stores them into the *plot* folder.
- run.pbs: a pbs script needed to run on the cluster.
Inside *serial* folder:
- matMul_dense.cpp: the serial solution for the first exercise, designed for dense matrices.
- matMul_sparse.cpp: it performs matrix multiplicatin as matMul_dense.cpp do, but this code is designed for sparse matrices.
- matT_dense.cpp: the serial solution to the first request of the second exercise, designed for dense matrices.
- matT_sparse.cpp: the sparse version of matT_dense.cpp.
- matBlockT_dense.cpp: the serial solution to the second request of the second exercise, designed for dense matrices.
- matBlockT_sparse.cpp: the sparse counterpart of matBlockT_dense.cpp.
Inside *parallel* folder:
- matMulPar_dense.cpp: a program that uses OMP directives to parallelize matMul_dense.cpp.
- matMulPar_sparse.cpp: the parallelized version of matMul_sparse.cpp.
- matTpar_dense.cpp: the parallelized version of matT_dense.cpp.
- matTpar_sparse.cpp: the parallelized version of matT_sparse.cpp.
- matBlockTpar_dense.cpp: the parallelized version of matBlockT_dense.cpp.
- matBlockTpar_sparse.cpp:the parallelized version of matBlockT_sparse.cpp.

## How to run the experiment

Run this experiment is very simple thanks to the scripts that automante most of the passages.

if you want to execute a test locally, follow these steps:
1. Run the script compile.sh, it will produce all the executable files inside the respective *bin* folders.
2. Run the script benchmark.sh, it will execute all the programs causing the cretion of the *report* folder with the report files in .csv format.
3. Run the script prepare_plot.sh, it will create the plot folder with the structure that plot_results.m expects.
4. On a machine with installed GNU octave, you can run plot_results.m to create the plots. Alternativley, you can use any plot tool you want. In that
case read alex_pegoraro_227642_homework_2.pdf to understand the structure of the .csv files and be able to read them.
5. If you want to clean your workspace run clean.sh, it will delete all the executables and the report files (but not the plots).

If on the other hand you want to run the test on the cluster, you can submit run.pbs to the short_cpuQ. This file will execute compile.sh and benchmark.sh,
leading to the creation of the *report* folder. In such a situation, you can then perform steps from 3 to 5 of the previous list either on the cluster or
on your local machine (you need to download just the *report* folder in the latter case).

Note: this is the way to test all the programs one shot. If you want to benchmrk only a specific one, read alex_pegoraro_227642_homework_2.pdf to understand
the peculiarity and the logic of each program.
