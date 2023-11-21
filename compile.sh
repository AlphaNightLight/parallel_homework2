if [ ! -d serial/bin ];
then
	mkdir serial/bin
fi

if [ ! -d parallel/bin ];
then
	mkdir parallel/bin
fi



g++ -std=gnu++11 -o serial/bin/matMul_dense.exe serial/matMul_dense.cpp
g++ -std=gnu++11 -o serial/bin/matMul_sparse.exe serial/matMul_sparse.cpp

g++ -std=gnu++11 -o serial/bin/matT_dense.exe serial/matT_dense.cpp
g++ -std=gnu++11 -o serial/bin/matT_sparse.exe serial/matT_sparse.cpp

g++ -std=gnu++11 -o serial/bin/matBlockT_dense.exe serial/matBlockT_dense.cpp
g++ -std=gnu++11 -o serial/bin/matBlockT_sparse.exe serial/matBlockT_sparse.cpp



g++ -std=gnu++11 -fopenmp -o parallel/bin/matMulPar_dense.exe parallel/matMulPar_dense.cpp
g++ -std=gnu++11 -fopenmp -o parallel/bin/matMulPar_sparse.exe parallel/matMulPar_sparse.cpp

g++ -std=gnu++11 -fopenmp -o parallel/bin/matTpar_dense.exe parallel/matTpar_dense.cpp
g++ -std=gnu++11 -fopenmp -o parallel/bin/matTpar_sparse.exe parallel/matTpar_sparse.cpp

g++ -std=gnu++11 -fopenmp -o parallel/bin/matBlockTpar_dense.exe parallel/matBlockTpar_dense.cpp
g++ -std=gnu++11 -fopenmp -o parallel/bin/matBlockTpar_sparse.exe parallel/matBlockTpar_sparse.cpp
