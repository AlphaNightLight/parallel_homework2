echo >"report_matMul_dense.csv" ROW_N_A,COL_N_A,COL_N_B,time
./matMul_dense.exe
echo >"report_matMul_sparse.csv" ROW_N_A,COL_N_A,COL_N_B,DENSITY,time
./matMul_sparse.exe

echo >"report_matT_dense.csv" ROW_N,COL_N,time
./matT_dense.exe
echo >"report_matT_sparse.csv" ROW_N,COL_N,DENSITY,time
./matT_sparse.exe

echo >"report_matBlockT_dense.csv" ROW_N,COL_N,BLOCK_ROW_N,BLOCK_COL_N,time
./matBlockT_dense.exe
echo >"report_matBlockT_sparse.csv" ROW_N,COL_N,BLOCK_ROW_N,BLOCK_COL_N,DENSITY,time
./matBlockT_sparse.exe
