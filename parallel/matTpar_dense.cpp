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
mat_and_time matTpar(Matrix);

void print_matrix(Matrix, string);

int main()
{
	srand(time(NULL));
	ofstream report_file_strong("reports/parallel/report_matTpar_dense_strong.csv", std::ios_base::app);
	ofstream report_file_weak("reports/parallel/report_matTpar_dense_weak.csv", std::ios_base::app);
	float execution_time;
	int i, j, scaling_type;
	int num_threads;
	
	int ROW_N, COL_N;
	
	#ifdef _OPENMP
	num_threads = atoi(getenv("OMP_NUM_THREADS"));
	for (scaling_type=0;scaling_type<2;++scaling_type){
		for (i=0;i<3;++i){
			switch(i){
				case 0:
					ROW_N = 2;
					COL_N = 8;
					break;
				case 1:
					ROW_N = 8;
					COL_N = 2;
					break;
				case 2:
					ROW_N = 8;
					COL_N = 8;
					break;
			}
			execution_time = 0.0;
			
			if (scaling_type == 1){
				ROW_N *= num_threads;
			}
			
			for (j=0;j<N_TRIALS;++j){
				Matrix A = random_dense_matrix(ROW_N, COL_N);
				//print_matrix(A, "A"); // Debug
				
				mat_and_time AT_struct = matTpar(A);
				Matrix AT = AT_struct.M;
				//print_matrix(AT, "AT"); // Debug
				
				execution_time += AT_struct.execution_time * (1.0 / N_TRIALS);
				
				deallocate_matrix(A);
				deallocate_matrix(AT);
			}
			
			if (scaling_type == 0){
				report_file_strong << fixed << setprecision(6);
				report_file_strong << num_threads << "," << ROW_N << "," << COL_N << "," << execution_time << endl;
			} else {
				report_file_weak << fixed << setprecision(6);
				report_file_weak << num_threads << "," << ROW_N << "," << COL_N << "," << execution_time << endl;
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

mat_and_time matTpar(Matrix A)
{
	Matrix AT;
	AT = allocate_matrix(A.cols, A.rows);
	double start_time, end_time;
	float execution_time = 0.0;
	int i, j;
	
	//auto start_time = chrono::high_resolution_clock::now();
	start_time = omp_get_wtime();
	
	#pragma omp parallel for collapse(2) private(i,j) shared(A,AT)
	for (i=0;i<AT.rows;++i){
		for (j=0;j<AT.cols;++j){
			AT.vals[i][j] = A.vals[j][i];
		}
	}
	
	//auto end_time = chrono::high_resolution_clock::now();
	//auto difference_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
	//execution_time = difference_time.count() * 1e-6;
	end_time = omp_get_wtime();
	// To be coherent with the serial cases, I convert execution_time to float
	execution_time = (double)(end_time-start_time);
	
	mat_and_time retval;
	retval.M = AT;
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
