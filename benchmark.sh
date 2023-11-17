if [ ! -d reports ];
then
	mkdir reports
fi

if [ ! -d reports/serial ];
then
	mkdir reports/serial
fi

if [ ! -d reports/parallel ];
then
	mkdir reports/parallel
fi



echo >"reports/serial/report_matMul_dense.csv" ROW_N_A,COL_N_A,COL_N_B,time
./serial/bin/matMul_dense.exe
echo >"reports/serial/report_matMul_sparse.csv" ROW_N_A,COL_N_A,COL_N_B,DENSITY,time
./serial/bin/matMul_sparse.exe

echo >"reports/serial/report_matT_dense.csv" ROW_N,COL_N,time
./serial/bin/matT_dense.exe
echo >"reports/serial/report_matT_sparse.csv" ROW_N,COL_N,DENSITY,time
./serial/bin/matT_sparse.exe

echo >"reports/serial/report_matBlockT_dense.csv" ROW_N,COL_N,BLOCK_ROW_N,BLOCK_COL_N,time
./serial/bin/matBlockT_dense.exe
echo >"reports/serial/report_matBlockT_sparse.csv" ROW_N,COL_N,BLOCK_ROW_N,BLOCK_COL_N,DENSITY,time
./serial/bin/matBlockT_sparse.exe



echo >"reports/parallel/report_matMulPar_dense.csv" NUM_THREADS,ROW_N_A,COL_N_A,COL_N_B,time
./parallel/bin/matMulPar_dense.exe
echo >"reports/parallel/report_matMulPar_sparse.csv" NUM_THREADS,ROW_N_A,COL_N_A,COL_N_B,DENSITY,time
./parallel/bin/matMulPar_sparse.exe

echo >"reports/parallel/report_matTpar_dense.csv" NUM_THREADS,ROW_N,COL_N,time
./parallel/bin/matTpar_dense.exe
echo >"reports/parallel/report_matTpar_sparse.csv" NUM_THREADS,ROW_N,COL_N,DENSITY,time
./parallel/bin/matTpar_sparse.exe

echo >"reports/parallel/report_matBlockTpar_dense.csv" NUM_THREADS,ROW_N,COL_N,BLOCK_ROW_N,BLOCK_COL_N,time
./parallel/bin/matBlockTpar_dense.exe
echo >"reports/parallel/report_matBlockTpar_sparse.csv" NUM_THREADS,ROW_N,COL_N,BLOCK_ROW_N,BLOCK_COL_N,DENSITY,time
./parallel/bin/matBlockTpar_sparse.exe
