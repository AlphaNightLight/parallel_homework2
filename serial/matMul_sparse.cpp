#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

#include <stdlib.h>
#include <time.h>

#include <chrono>

#define N_TRIALS 2
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
mat_and_time matMul(Matrix, Matrix);

void print_matrix(Matrix, string);

int main()
{
	srand(time(NULL));
	ofstream report_file("reports/serial/report_matMul_sparse.csv", std::ios_base::app);
	float execution_time;
	int i, j;
	
	int ROW_N_A, COL_N_A, COL_N_B;
	// For the matrices to be product compatible, if the first is ROW_N_A x COL_N_A,
	// the second must be COL_N_A x COL_N_B.
	float DENSITY;
	// The ratio between the quantity of nonzero elements and the total number of elements.
	
	for (i=0;i<3*3;++i){
		switch(i%3){
			case 0:
				ROW_N_A = 32;
				COL_N_A = 256;
				COL_N_B = 32;
				break;
			case 1:
				ROW_N_A = 128;
				COL_N_A = 16;
				COL_N_B = 128;
				break;
			case 2:
				ROW_N_A = 64;
				COL_N_A = 64;
				COL_N_B = 64;
				break;
		}
		switch(i/3){
			case 0: DENSITY = 0.2; break;
			case 1: DENSITY = 0.5; break;
			case 2: DENSITY = 0.8; break;
		}
		execution_time = 0.0;
		
		for (j=0;j<N_TRIALS;++j){
			Matrix A = random_sparse_matrix(ROW_N_A, COL_N_A, DENSITY);
			//print_matrix(A, "A"); // Debug
			Matrix B = random_sparse_matrix(COL_N_A, COL_N_B, DENSITY);
			//print_matrix(B, "B"); // Debug
			
			mat_and_time C_struct = matMul(A, B);
			Matrix C = C_struct.M;
			//print_matrix(C, "C"); // Debug
			
			execution_time += C_struct.execution_time * (1.0 / N_TRIALS);
			
			deallocate_matrix(A);
			deallocate_matrix(B);
			deallocate_matrix(C);
		}
		
		report_file << fixed << setprecision(6);
		report_file << ROW_N_A << "," << COL_N_A << "," << COL_N_B << "," << DENSITY << "," << execution_time << endl;
	}
	
	report_file.close();
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

mat_and_time matMul(Matrix A, Matrix B)
{
	Matrix C;
	C = allocate_matrix(A.rows, B.cols);
	float execution_time = 0.0;
	bool nonzero_flag;
	float C_value;
	int i, j, kA, kB;
	
	if (A.cols == B.rows){
		vector<float> B_CSC_vals(B.nonzeroes);
		vector<int> B_CSC_row_index(B.nonzeroes);
		int* B_CSC_col_index;
		B_CSC_col_index = new int[B.cols+1];
		
		auto start_time = chrono::high_resolution_clock::now();
		
		// Preprocessing: convert B into CSC format
		for (i=0;i<B.cols+1;++i) B_CSC_col_index[i] = 0;
		for (i=0;i<B.nonzeroes;++i) B_CSC_col_index[B.col_index[i]+1] += 1;
		for (i=2;i<B.cols+1;++i) B_CSC_col_index[i] += B_CSC_col_index[i-1];
		
		for (i=0;i<B.rows;++i){
			for (j=B.row_index[i];j<B.row_index[i+1];++j){
				B_CSC_vals[B_CSC_col_index[B.col_index[j]]] = B.vals[j];
				B_CSC_row_index[B_CSC_col_index[B.col_index[j]]] = i;
				B_CSC_col_index[B.col_index[j]]++;
			}
		}
		
		for (i=B.cols-1;i>0;--i) B_CSC_col_index[i] = B_CSC_col_index[i-1];
		B_CSC_col_index[0] = 0;
		
		// Product
		for (i=0;i<C.rows;++i){
			C.row_index[i] = C.nonzeroes;
			for (j=0;j<C.cols;++j){
				C_value = 0.0;
				nonzero_flag = false;
				kA = A.row_index[i];
				kB = B_CSC_col_index[j];
				while(kA < A.row_index[i+1] && kB < B_CSC_col_index[j+1]){
					if (A.col_index[kA] == B_CSC_row_index[kB]){
						nonzero_flag = true;
						C_value += A.vals[kA] * B_CSC_vals[kB];
						kA++;
						kB++;
					} else if (A.col_index[kA] < B_CSC_row_index[kB]){
						kA++;
					} else { // A.col_index[kA] > B_CSC_row_index[kB]
						kB++;
					}
				}
				if (nonzero_flag){
					C.vals.push_back(C_value);
					C.col_index.push_back(j);
					C.nonzeroes++;
				}
			}
		}
		C.row_index[C.rows] = C.nonzeroes;
		
		auto end_time = chrono::high_resolution_clock::now();
		auto difference_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
		execution_time = difference_time.count() * 1e-6;
		
		delete [] B_CSC_col_index;
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
