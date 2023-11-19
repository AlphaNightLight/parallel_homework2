if [ ! -d plots ];
then
	mkdir plots
fi

if [ ! -d plots/matMul_dense ];
then
	mkdir plots/matMul_dense
fi
if [ ! -d plots/matMul_sparse ];
then
	mkdir plots/matMul_sparse
fi

if [ ! -d plots/matT_dense ];
then
	mkdir plots/matT_dense
fi
if [ ! -d plots/matT_sparse ];
then
	mkdir plots/matT_sparse
fi

if [ ! -d plots/matBlockT_dense ];
then
	mkdir plots/matBlockT_dense
fi
if [ ! -d plots/matBlockT_sparse ];
then
	mkdir plots/matBlockT_sparse
fi
