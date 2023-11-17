#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <list>

#include <stdlib.h>
#include <time.h>

#include <chrono>

#define N_TRIALS 1
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

Matrix random_sparse_matrix(int, int, int, int, float);
mat_and_time matBlockT(Matrix);

void print_matrix(Matrix, string);

int main()
{
	srand(time(NULL));
	ofstream report_file("reports/serial/report_matBlockT_sparse.csv", std::ios_base::app);
	float execution_time;
	int i, j;
	
	int ROW_N, COL_N, BLOCK_ROW_N, BLOCK_COL_N;
	// Each block will be a marix BLOCK_ROW_N x BLOCK_COL_N
	float DENSITY;
	// The ratio between the quantity of nonzero elements and the total number of elements.
	
	for (i=0;i<3*3;++i){
		switch(i%3){
			case 0:
				ROW_N = 8;
				COL_N = 8;
				BLOCK_ROW_N = 1;
				BLOCK_COL_N = 1;
				break;
			case 1:
				ROW_N = 8;
				COL_N = 8;
				BLOCK_ROW_N = 2;
				BLOCK_COL_N = 2;
				break;
			case 2:
				ROW_N = 8;
				COL_N = 8;
				BLOCK_ROW_N = 4;
				BLOCK_COL_N = 4;
				break;
		}
		switch(i/3){
			case 0: DENSITY = 0.2; break;
			case 1: DENSITY = 0.5; break;
			case 2: DENSITY = 0.8; break;
		}
		execution_time = 0.0;
		
		for (j=0;j<N_TRIALS;++j){
			Matrix A = random_sparse_matrix(ROW_N, COL_N, BLOCK_ROW_N, BLOCK_COL_N, DENSITY);
			print_matrix(A, "A");
			
			mat_and_time AT_struct = matBlockT(A);
			Matrix AT = AT_struct.M;
			print_matrix(AT, "AT");
			
			execution_time += AT_struct.execution_time * (1.0 / N_TRIALS);
			
			deallocate_matrix(A);
			deallocate_matrix(AT);
		}
		
		report_file << fixed << setprecision(6);
		report_file << ROW_N << "," << COL_N << "," << BLOCK_ROW_N << "," << BLOCK_COL_N << "," << DENSITY << "," << execution_time << endl;
	}
	
	report_file.close();
	return 0;
}

Matrix allocate_matrix(int rows, int cols, int block_rows, int block_cols)
{
	int i;
	Matrix M;
	M.rows = rows;
	M.cols = cols;
	M.block_rows = block_rows;
	M.block_cols = block_cols;
	
	M.row_index = new int[rows+1];
	for (i=0;i<rows+1;++i) M.row_index[i] = 0;
	M.nonzeroes = 0;
	
	return M;
}

void deallocate_matrix(Matrix M)
{
	delete[] M.row_index;
}

Matrix random_sparse_matrix(int rows, int cols, int block_rows, int block_cols, float density)
{
	Matrix M;
	M = allocate_matrix(rows, cols, block_rows, block_cols);
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

mat_and_time matBlockT(Matrix A)
{
	Matrix AT;
	AT = allocate_matrix(A.cols, A.rows, A.block_cols, A.block_rows);
	float execution_time = 0.0;
	int outer_rows, outer_cols;
	int B_max_nonzeroes;
	int i, j, ib, jb, ja;
	
	if (A.rows%A.block_rows == 0 && A.cols%A.block_cols == 0){
		outer_rows = A.rows / A.block_rows;
		outer_cols = A.cols / A.block_cols;
		
		Matrix B, BT;
		B = allocate_matrix(A.block_rows, A.block_cols, A.block_rows, A.block_cols);
		BT = allocate_matrix(A.block_cols, A.block_rows, A.block_cols, A.block_rows);
		
		// We don't know in advance how many elements will B have, su we assume the worst case: Or B is full, or it has all the elements of A.
		B_max_nonzeroes = A.nonzeroes<(B.rows * B.cols) ? A.nonzeroes : (B.rows * B.cols);
		
		B.vals.resize(B_max_nonzeroes);
		B.col_index.resize(B_max_nonzeroes);
		BT.vals.resize(B_max_nonzeroes);
		BT.col_index.resize(B_max_nonzeroes);
		
		AT.vals.resize(A.nonzeroes);
		AT.col_index.resize(A.nonzeroes);
		
		vector<list<float>> AT_vals_support(AT.rows);
		vector<list<int>> AT_col_index_support(AT.rows);
		
		auto start_time = chrono::high_resolution_clock::now();
		
		for (i=0;i<outer_rows;++i){ // for each block
			for (j=0;j<outer_cols;++j){
				// Extract the block
				for (ib=0;ib<B.rows+1;++ib) B.row_index[ib] = 0;
				B.nonzeroes = 0;
				
				for (ib=0;ib<B.rows;++ib){
					for (ja=A.row_index[i*B.rows+ib];ja<A.row_index[i*B.rows+ib+1];++ja){
						if (A.col_index[ja]>=j*B.cols && A.col_index[ja]<(j+1)*B.cols){
							//cout << A.vals[ja] << " "; // Debug
							B.vals[B.nonzeroes] = A.vals[ja];
							B.col_index[B.nonzeroes] = A.col_index[ja] % B.cols;
							B.nonzeroes++;
							B.row_index[ib+1]++;
						}
					}
				}
				for (ib=2;ib<B.rows+1;++ib) B.row_index[ib] += B.row_index[ib-1];
				//print_matrix(B, "B"); // Debug
				
					// Transpose the block
					// This time clear BT.row_index is needed, as allocate_matrix() is executed only one time, but BT is used more times
					for (ib=0;ib<BT.rows+1;++ib) BT.row_index[ib] = 0;
					for (ib=0;ib<B.nonzeroes;++ib) BT.row_index[B.col_index[ib]+1] += 1;
					for (ib=2;ib<BT.rows+1;++ib) BT.row_index[ib] += BT.row_index[ib-1];
					
					for (ib=0;ib<B.rows;++ib){
						for (jb=B.row_index[ib];jb<B.row_index[ib+1];++jb){
							BT.vals[BT.row_index[B.col_index[jb]]] = B.vals[jb];
							BT.col_index[BT.row_index[B.col_index[jb]]] = ib;
							BT.row_index[B.col_index[jb]]++;
						}
					}
					
					for (ib=BT.rows-1;ib>0;--ib) BT.row_index[ib] = BT.row_index[ib-1];
					BT.row_index[0] = 0;
					BT.nonzeroes = BT.row_index[BT.rows];
					//print_matrix(BT, "BT"); // Debug
				
				// Re-insert the block in the appropriate position
				for (ib=0;ib<BT.rows;++ib){
					for (jb=BT.row_index[ib];jb<BT.row_index[ib+1];++jb){
						AT_vals_support[j*BT.rows+ib].push_back(BT.vals[jb]);
						AT_col_index_support[j*BT.rows+ib].push_back(i*BT.cols + BT.col_index[jb]);
					}
				}
			}
		}
		
		for (i=0;i<AT.rows;++i){
			while(!AT_vals_support[i].empty()){
				//cout << AT_col_index_support[i].front() << " "; // Debug
				AT.vals[AT.nonzeroes] = AT_vals_support[i].front();
				AT.col_index[AT.nonzeroes] = AT_col_index_support[i].front();
				AT.nonzeroes++;
				AT_vals_support[i].pop_front();
				AT_col_index_support[i].pop_front();
				AT.row_index[i+1]++;
			}
		}
		for (i=2;i<AT.rows+1;++i) AT.row_index[i] += AT.row_index[i-1];
		
		auto end_time = chrono::high_resolution_clock::now();
		auto difference_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
		execution_time = difference_time.count() * 1e-6;
		
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
	int i, j, k;
	cout << fixed << setprecision(5);
	cout << ">> Matrix " << name << " " << M.rows << " x " << M.cols << ", blocks "  << M.block_rows << " x " << M.block_cols << endl;
	
	for (i=0;i<M.rows;++i){
		if (i%M.block_rows == 0 && i!=0) cout << endl;
		j = M.row_index[i];
		for (k=0;k<M.cols;++k){
			if (k%M.block_cols == 0 && k!=0) cout << "  ";
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
