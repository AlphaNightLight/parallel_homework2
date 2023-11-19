#ifdef _OPENMP
#include <omp.h>
#endif

#include <iostream>
#include <fstream>
#include <iomanip>

#include <stdlib.h>
#include <time.h>

//#include <chrono>

#define N_TRIALS 4
// To reduce spikes an averege will be performed

using namespace std;

typedef struct Matrix
{
	float** vals;
	int rows;
	int cols;
} Matrix;

typedef struct mat_and_time
{
	Matrix M;
	float execution_time;
} mat_and_time;

Matrix allocate_matrix(int, int);
void deallocate_matrix(Matrix);

Matrix random_dense_matrix(int, int);
mat_and_time matMulPar(Matrix, Matrix);

void print_matrix(Matrix, string);

int main()
{
	srand(time(NULL));
	ofstream report_file_strong("reports/parallel/report_matMulPar_dense_strong.csv", std::ios_base::app);
	ofstream report_file_weak("reports/parallel/report_matMulPar_dense_weak.csv", std::ios_base::app);
	float execution_time;
	int i, j, scaling_type;
	int num_threads;
	
	int ROW_N_A, COL_N_A, COL_N_B;
	// For the matrices to be product compatible, if the first is ROW_N_A x COL_N_A,
	// the second must be COL_N_A x COL_N_B.
	
	#ifdef _OPENMP
	num_threads = atoi(getenv("OMP_NUM_THREADS"));
	for (scaling_type=0;scaling_type<2;++scaling_type){
		for (i=0;i<3;++i){
			switch(i){
				case 0:
					ROW_N_A = 64;
					COL_N_A = 1024;
					COL_N_B = 64;
					break;
				case 1:
					ROW_N_A = 512;
					COL_N_A = 16;
					COL_N_B = 512;
					break;
				case 2:
					ROW_N_A = 128;
					COL_N_A = 256;
					COL_N_B = 128;
					break;
			}
			execution_time = 0.0;
			
			if (scaling_type == 1){
				ROW_N_A *= num_threads;
			}
			
			for (j=0;j<N_TRIALS;++j){
				Matrix A = random_dense_matrix(ROW_N_A, COL_N_A);
				//print_matrix(A, "A"); // Debug
				Matrix B = random_dense_matrix(COL_N_A, COL_N_B);
				//print_matrix(B, "B"); // Debug
				
				mat_and_time C_struct = matMulPar(A, B);
				Matrix C = C_struct.M;
				//print_matrix(C, "C"); // Debug
				
				execution_time += C_struct.execution_time * (1.0 / N_TRIALS);
				
				deallocate_matrix(A);
				deallocate_matrix(B);
				deallocate_matrix(C);
			}
			
			if (scaling_type == 0){
				report_file_strong << fixed << setprecision(6);
				report_file_strong << num_threads << "," << ROW_N_A << "," << COL_N_A << "," << COL_N_B << "," << execution_time << endl;
			} else {
				report_file_weak << fixed << setprecision(6);
				report_file_weak << num_threads << "," << ROW_N_A << "," << COL_N_A << "," << COL_N_B << "," << execution_time << endl;
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

Matrix allocate_matrix(int rows, int cols)
{
	Matrix M;
	M.rows = rows;
	M.cols = cols;
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

Matrix random_dense_matrix(int rows, int cols)
{
	Matrix M;
	M = allocate_matrix(rows, cols);
	int i, j;
	
	for (i=0;i<rows;++i){
		for (j=0;j<cols;++j){
			M.vals[i][j] = (float)rand() / (float)rand();
		}
	}
	
	return M;
}

mat_and_time matMulPar(Matrix A, Matrix B)
{
	Matrix C;
	C = allocate_matrix(A.rows, B.cols);
	double start_time, end_time;
	float execution_time = 0.0;
	int depth;
	int i, j, k;
	
	if (A.cols == B.rows){
		depth = A.cols;
		//auto start_time = chrono::high_resolution_clock::now();
		start_time = omp_get_wtime();
		
		#pragma omp parallel for collapse(2) private(i,j,k) shared(A,B,C,depth) schedule(static)
		for (i=0;i<C.rows;++i){
			for (j=0;j<C.cols;++j){
				C.vals[i][j] = 0.0;
				for(k=0;k<depth;++k){
					C.vals[i][j] += A.vals[i][k] * B.vals[k][j];
				}
				// Debug
				//#pragma omp critical
				//cout << "THREAD " << omp_get_thread_num() << " read " << i << "-" << j << endl;
			}
		}
		
		//auto end_time = chrono::high_resolution_clock::now();
		//auto difference_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
		//execution_time = difference_time.count() * 1e-6;
		end_time = omp_get_wtime();
		// To be coherent with the serial cases, I convert execution_time to float
		execution_time = (double)(end_time-start_time);
	} else {
		cout << "Error: not compatible matrices!" << endl;
	}
	
	mat_and_time retval;
	retval.M = C;
	retval.execution_time = execution_time;
	return retval;
}

void print_matrix(Matrix M, string name)
{
	int i, j;
	cout << fixed << setprecision(5);
	cout << "Matrix " << name << " " << M.rows << " x " << M.cols << endl;
	
	for (i=0;i<M.rows;++i){
		for (j=0;j<M.cols;++j){
			cout << M.vals[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}
