if [ ! -d serial/bin ];
then
	mkdir serial/bin
fi

if [ ! -d parallel/bin ];
then
	mkdir parallel/bin
fi



g++ -o serial/bin/matMul_dense serial/matMul_dense.cpp
g++ -o serial/bin/matMul_sparse serial/matMul_sparse.cpp

g++ -o serial/bin/matT_dense serial/matT_dense.cpp
g++ -o serial/bin/matT_sparse serial/matT_sparse.cpp

g++ -o serial/bin/matBlockT_dense serial/matBlockT_dense.cpp
g++ -o serial/bin/matBlockT_sparse serial/matBlockT_sparse.cpp



g++ -o parallel/bin/matMulPar_dense parallel/matMulPar_dense.cpp
g++ -o parallel/bin/matMulPar_sparse parallel/matMulPar_sparse.cpp

g++ -o parallel/bin/matTpar_dense parallel/matTpar_dense.cpp
g++ -o parallel/bin/matTpar_sparse parallel/matTpar_sparse.cpp

g++ -o parallel/bin/matBlockTpar_dense parallel/matBlockTpar_dense.cpp
g++ -o parallel/bin/matBlockTpar_sparse parallel/matBlockTpar_sparse.cpp
