#ifdef _OPENMP
#include <omp.h>
#endif

#include <iostream>
#include <fstream>
#include <iomanip>

#include <stdlib.h>
#include <time.h>

//#include <chrono>

#define N_TRIALS 2
// To reduce spikes an averege will be performed

using namespace std;

typedef struct Matrix
{
	float** vals;
	int rows;
	int cols;
	int block_rows;
	int block_cols;
} Matrix;

typedef struct mat_and_time
{
	Matrix M;
	float execution_time;
} mat_and_time;

Matrix allocate_matrix(int, int, int, int);
void deallocate_matrix(Matrix);

Matrix random_dense_matrix(int, int, int, int);
mat_and_time matBlockTpar(Matrix);

void print_matrix(Matrix, string);

int main()
{
	srand(time(NULL));
	ofstream report_file_strong("reports/parallel/report_matBlockTpar_dense_strong.csv", std::ios_base::app);
	ofstream report_file_weak("reports/parallel/report_matBlockTpar_dense_weak.csv", std::ios_base::app);
	float execution_time;
	int i, j, scaling_type;
	int num_threads;
	
	int ROW_N, COL_N, BLOCK_ROW_N, BLOCK_COL_N;
	// Each block will be a marix BLOCK_ROW_N x BLOCK_COL_N
	
	#ifdef _OPENMP
	num_threads = atoi(getenv("OMP_NUM_THREADS"));
	for (scaling_type=0;scaling_type<2;++scaling_type){
		for (i=0;i<3;++i){
			switch(i){
				case 0:
					ROW_N = 512;
					COL_N = 512;
					BLOCK_ROW_N = 8;
					BLOCK_COL_N = 8;
					break;
				case 1:
					ROW_N = 512;
					COL_N = 512;
					BLOCK_ROW_N = 32;
					BLOCK_COL_N = 32;
					break;
				case 2:
					ROW_N = 512;
					COL_N = 512;
					BLOCK_ROW_N = 128;
					BLOCK_COL_N = 128;
					break;
			}
			execution_time = 0.0;
			
			if (scaling_type == 1){
				ROW_N *= num_threads;
				BLOCK_ROW_N *= num_threads;
			}
			
			for (j=0;j<N_TRIALS;++j){
				Matrix A = random_dense_matrix(ROW_N, COL_N, BLOCK_ROW_N, BLOCK_COL_N);
				//print_matrix(A, "A"); // Debug
				
				mat_and_time AT_struct = matBlockTpar(A);
				Matrix AT = AT_struct.M;
				//print_matrix(AT, "AT"); // Debug
				
				execution_time += AT_struct.execution_time * (1.0 / N_TRIALS);
				
				deallocate_matrix(A);
				deallocate_matrix(AT);
			}
			
			if (scaling_type == 0){
				report_file_strong << fixed << setprecision(6);
				report_file_strong << num_threads << "," << ROW_N << "," << COL_N << ","
								   << BLOCK_ROW_N << "," << BLOCK_COL_N << "," << execution_time << endl;
			} else {
				report_file_weak << fixed << setprecision(6);
				report_file_weak << num_threads << "," << ROW_N << "," << COL_N << ","
								 << BLOCK_ROW_N << "," << BLOCK_COL_N << "," << execution_time << endl;
			}
		}
	}
	#else
	cout << "Error: You must compile with -fopenmp flag in parallel codes!" << endl;
	#endif
	
	report_file_strong.close();
	report_file_weak.close();
	return 0;
}

Matrix allocate_matrix(int rows, int cols, int block_rows, int block_cols)
{
	Matrix M;
	M.rows = rows;
	M.cols = cols;
	M.block_rows = block_rows;
	M.block_cols = block_cols;
	int i;
	
	M.vals = new float*[rows];
	for (i=0;i<rows;++i){
		M.vals[i] = new float[cols];
	}
	
	return M;
}

void deallocate_matrix(Matrix M)
{
	int i;
	for (i=0;i<M.rows;++i){
		delete[] M.vals[i];
	}
	delete[] M.vals;
}

Matrix random_dense_matrix(int rows, int cols, int block_rows, int block_cols)
{
	Matrix M;
	M = allocate_matrix(rows, cols, block_rows, block_cols);
	int i, j;
	
	for (i=0;i<rows;++i){
		for (j=0;j<cols;++j){
			M.vals[i][j] = (float)rand() / (float)rand();
		}
	}
	
	return M;
}

mat_and_time matBlockTpar(Matrix A)
{
	Matrix AT;
	AT = allocate_matrix(A.cols, A.rows, A.block_cols, A.block_rows);
	double start_time, end_time;
	float execution_time = 0.0;
	int outer_rows, outer_cols;
	int i, j, ib, jb;
	
	if (A.rows%A.block_rows == 0 && A.cols%A.block_cols == 0){
		outer_rows = A.rows / A.block_rows;
		outer_cols = A.cols / A.block_cols;
		
		Matrix B, BT;
		B = allocate_matrix(A.block_rows, A.block_cols, A.block_rows, A.block_cols);
		BT = allocate_matrix(A.block_cols, A.block_rows, A.block_cols, A.block_rows);
		
		//auto start_time = chrono::high_resolution_clock::now();
		start_time = omp_get_wtime();
		
		#pragma omp parallel for collapse(2) private(i,j,ib,jb) firstprivate(B,BT) shared(A,AT,outer_rows,outer_cols) schedule(static)
		for (i=0;i<outer_rows;++i){ // for each block
			for (j=0;j<outer_cols;++j){
				
				for (ib=0;ib<B.rows;++ib){ // extract the block
					for (jb=0;jb<B.cols;++jb){
						B.vals[ib][jb] = A.vals[i*B.rows+ib][j*B.cols+jb];
					}
				}
				
				for (ib=0;ib<BT.rows;++ib){ // transpose the block
					for (jb=0;jb<BT.cols;++jb){
						BT.vals[ib][jb] = B.vals[jb][ib];
					}
				}
				
				for (ib=0;ib<BT.rows;++ib){ // re-insert the block
					for (jb=0;jb<BT.cols;++jb){
						AT.vals[j*BT.rows+ib][i*BT.cols+jb] = BT.vals[ib][jb];
					}
				}
				// Debug
				//#pragma omp critical
				//cout << "THREAD " << omp_get_thread_num() << " read " << i << "-" << j << endl;
			}
		}
		
		/* This produces the same resut in a more efficient way, but I assumed we have as a requirement to explicitley 
		extract and re-insert the block B, that's why I implemented the previous for *
		for (i=0;i<outer_rows;++i){ // for each block
			for (j=0;j<outer_cols;++j){
				for (ib=0;ib<A.block_rows;++ib){ // for each element of the block
					for (jb=0;jb<A.block_cols;++jb){
						AT.vals[j*A.block_cols+jb][i*A.block_rows+ib] = A.vals[i*A.block_rows+ib][j*A.block_cols+jb];
					}
				}
			}
		}
		/**/
		
		//auto end_time = chrono::high_resolution_clock::now();
		//auto difference_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
		//execution_time = difference_time.count() * 1e-6;
		end_time = omp_get_wtime();
		// To be coherent with the serial cases, I convert execution_time to float
		execution_time = (double)(end_time-start_time);
		
		deallocate_matrix(B);
		deallocate_matrix(BT);
	} else {
		cout << "Error: invalid block division!" << endl;
	}
	
	mat_and_time retval;
	retval.M = AT;
	retval.execution_time = execution_time;
	return retval;
}

void print_matrix(Matrix M, string name)
{
	int i, j;
	cout << fixed << setprecision(5);
	cout << ">> Matrix " << name << " " << M.rows << " x " << M.cols << ", blocks "  << M.block_rows << " x " << M.block_cols << endl;
	
	for (i=0;i<M.rows;++i){
		if (i%M.block_rows == 0 && i!=0) cout << endl;
		for (j=0;j<M.cols;++j){
			if (j%M.block_cols == 0 && j!=0) cout << "  ";
			cout << M.vals[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}
