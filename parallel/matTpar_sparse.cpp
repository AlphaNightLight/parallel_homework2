#ifdef _OPENMP
#include <omp.h>
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

#include <stdlib.h>
#include <time.h>

//#include <chrono>

#define N_TRIALS 4
// To reduce spikes an averege will be performed

using namespace std;

// Sparse matrix represented in the CSR standard.
typedef struct Matrix
{
	vector<float> vals;
	vector<int> col_index;
	int* row_index;
	
	int rows;
	int cols;
	int nonzeroes;
} Matrix;

typedef struct mat_and_time
{
	Matrix M;
	float execution_time;
} mat_and_time;

Matrix allocate_matrix(int, int);
void deallocate_matrix(Matrix);

Matrix random_sparse_matrix(int, int, float);
mat_and_time matTpar(Matrix);

void print_matrix(Matrix, string);

int main()
{
	srand(time(NULL));
	ofstream report_file_strong("reports/parallel/report_matTpar_sparse_strong.csv", std::ios_base::app);
	ofstream report_file_weak("reports/parallel/report_matTpar_sparse_weak.csv", std::ios_base::app);
	float execution_time;
	int i, j, scaling_type;
	int num_threads;
	
	int ROW_N, COL_N;
	float DENSITY;
	// The ratio between the quantity of nonzero elements and the total number of elements.
	
	#ifdef _OPENMP
	num_threads = atoi(getenv("OMP_NUM_THREADS"));
	for (scaling_type=0;scaling_type<2;++scaling_type){
		for (i=0;i<3*3;++i){
			switch(i%3){
				case 0:
					ROW_N = 64;
					COL_N = 65536;
					break;
				case 1:
					ROW_N = 65536;
					COL_N = 64;
					break;
				case 2:
					ROW_N = 2048;
					COL_N = 2048;
					break;
			}
			switch(i/3){
				case 0: DENSITY = 0.2; break;
				case 1: DENSITY = 0.5; break;
				case 2: DENSITY = 0.8; break;
			}
			execution_time = 0.0;
			
			if (scaling_type == 1){
				ROW_N *= num_threads;
			}
			
			for (j=0;j<N_TRIALS;++j){
				Matrix A = random_sparse_matrix(ROW_N, COL_N, DENSITY);
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
				report_file_strong << num_threads << "," << ROW_N << "," << COL_N << "," << DENSITY << "," << execution_time << endl;
			} else {
				report_file_weak << fixed << setprecision(6);
				report_file_weak << num_threads << "," << ROW_N << "," << COL_N << "," << DENSITY << "," << execution_time << endl;
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
	int i;
	Matrix M;
	M.rows = rows;
	M.cols = cols;
	
	M.row_index = new int[rows+1];
	for (i=0;i<rows+1;++i) M.row_index[i] = 0;
	M.nonzeroes = 0;
	
	return M;
}

void deallocate_matrix(Matrix M)
{
	delete[] M.row_index;
}

Matrix random_sparse_matrix(int rows, int cols, float density)
{
	Matrix M;
	M = allocate_matrix(rows, cols);
	int threshold = density * 100; // The density expressed in percentage rounded to unit
	int i, j;
	
	for (i=0;i<rows;++i){
		M.row_index[i] = M.nonzeroes;
		for (j=0;j<cols;++j){
			if ((rand()%100) < threshold){
				M.vals.push_back((float)rand() / (float)rand());
				M.col_index.push_back(j);
				M.nonzeroes++;
			}
		}
	}	
	M.row_index[rows] = M.nonzeroes;
	
	return M;
}

mat_and_time matTpar(Matrix A)
{
	Matrix AT;
	AT = allocate_matrix(A.cols, A.rows);
	double start_time, end_time;
	float execution_time = 0.0;
	int i, j;
	
	AT.vals.resize(A.nonzeroes);
	AT.col_index.resize(A.nonzeroes);
	
	//auto start_time = chrono::high_resolution_clock::now();
	start_time = omp_get_wtime();
	
	//for (i=0;i<AT.rows+1;++i) AT.row_index[i] = 0; // Not needed, as already performed by allocate_matrix()
	for (i=0;i<A.nonzeroes;++i) AT.row_index[A.col_index[i]+1] += 1;
	for (i=2;i<AT.rows+1;++i) AT.row_index[i] += AT.row_index[i-1];
	
	/** Observation: the outer loop can't be parallelized. The reason is that the rows of AT can be accessed freely, but its columns are in
		a sense "pushed" inside a row accessing AT.row_index and then incrementing it.
		So, parallelizing the outer loop would mean to access many rows of A at the same time. In a situation in which 2 threads are accessing the
		same column of A, they wolud try to access the same row of AT, and as we don't know who will access it first this operation will resut in
		having the row elements of AT scrambled.
		
		BUT, if we access the rows sequentially, each element of that row can be pushed in parallel. The reason is that all the elements 
		in a row will have different column index, that will result in access to different rows of AT, that as said creates no clash 
		(It's even not necessary to make AT.row_index[A.col_index[j]]++ atomic as it will be a different location for each thread).
		
		Algorithmically speaking, A.col_index[j] will be different for each thread, as we are talking about elements of the same row, and also
		AT.row_index[A.col_index[j]] will be different, as two different rows have the same row_index only if the first is empty, but if it's
		empty it won't be accessed, so again no conflict.
	*/
	for (i=0;i<A.rows;++i){
		//cout << "current i: " << i << endl;
		#pragma omp parallel for private(j) shared(i,A,AT) schedule(static)
		for (j=A.row_index[i];j<A.row_index[i+1];++j){
			AT.vals[AT.row_index[A.col_index[j]]] = A.vals[j];
			AT.col_index[AT.row_index[A.col_index[j]]] = i;
			AT.row_index[A.col_index[j]]++;
			// Debug
			//cout << "THREAD " << omp_get_thread_num() << " read " << i << "-" << j << endl;
		}
	}
	
	for (i=AT.rows-1;i>0;--i) AT.row_index[i] = AT.row_index[i-1];
	AT.row_index[0] = 0;
	AT.nonzeroes = AT.row_index[AT.rows];
	
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
	int i, j, k;
	cout << fixed << setprecision(5);
	cout << "Matrix " << name << " " << M.rows << " x " << M.cols << endl;
	
	for (i=0;i<M.rows;++i){
		j = M.row_index[i];
		for (k=0;k<M.cols;++k){
			if (j < M.row_index[i+1]){
				if (k == M.col_index[j]){
					cout << M.vals[j] << " ";
					j++;
				} else {
					cout << "------- ";
				}
			} else {
				cout << "------- ";
			}
		}
		cout << endl;
	}
	
	cout << "CSR representation, " << M.nonzeroes << " nonzeroes" << endl;
	cout << "vals: ";
	for (i=0;i<M.nonzeroes;++i){
		cout << M.vals[i] << " ";
	}
	cout << endl;
	cout << "col_index: ";
	for (i=0;i<M.nonzeroes;++i){
		cout << M.col_index[i] << " ";
	}
	cout << endl;
	cout << "row_index: ";
	for (i=0;i<M.rows+1;++i){
		cout << M.row_index[i] << " ";
	}
	cout << endl << endl;
}
