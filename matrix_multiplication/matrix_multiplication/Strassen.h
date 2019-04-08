#pragma once
#include <iostream>

// https://en.wikipedia.org/wiki/Strassen_algorithm
// Small letter 'n' may be difference from capital letter 'N' for matrix calculation prupose
// Some operation may not calculate using whole elements

const int N = 4;
using std::cout;

template<typename T>
void MatrixDisp(const int n, const T A[][N])
{
	cout << "[";
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			cout << A[i][j];
			if (j != n - 1) { cout << ", "; }
		}

		if (i != n - 1) { cout << ";\n "; }

	}
	cout << "]\n\n";
}

template<typename T>
void MatrixMult_Standard(const int n, const T A[][N], const T B[][N], T C[][N])
{
	T dot_product;

#pragma omp parallel for
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			dot_product = 0;

			for (int k = 0; k < n; k++)
			{
				dot_product += A[i][k] * B[k][j];
			}

			C[i][j] = dot_product;
		}
	}
}

template<typename T>
void MatrixMult_OpenMP(const int n, const T A[][N], const T B[][N], T C[][N])
{
	T dot_product;

#pragma omp parallel for
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			dot_product = 0;

			for (int k = 0; k < n; k++)
			{
				dot_product += A[i][k] * B[k][j];
			}

			C[i][j] = dot_product;
		}
	}
}

template<typename T>
void MatrixSum(const int n, const T A[][N], const T B[][N], T C[][N])
{

#pragma omp parallel for
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			C[i][j] = A[i][j] + B[i][j];
		}
	}
}

template<typename T>
void MatrixSubs(const int n, const T A[][N], const T B[][N], T C[][N])
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			C[i][j] = A[i][j] - B[i][j];
		}
	}
}

template<typename T>
void MatrixMult_Strassen(const int n, const T A[][N], const T B[][N], T C[][N])
{
	int i, j;
	int mid = n / 2;

	if (n == 2) 
	{  
		MatrixMult_OpenMP<T>(n, A, B, C);
		return;
	}

	T A_11[N][N], A_12[N][N], A_21[N][N], A_22[N][N],
		B_11[N][N], B_12[N][N], B_21[N][N], B_22[N][N],
		C_11[N][N], C_12[N][N], C_21[N][N], C_22[N][N],
		M_1[N][N], M_2[N][N], M_3[N][N], M_4[N][N],
		M_5[N][N], M_6[N][N], M_7[N][N], temp_A[N][N], temp_B[N][N];

	//Matrix Division
	for (i = 0; i < mid; i++)
	{
		for (j =0; j < mid ; j++)
		{
			A_11[i][j] = A[i][j];
			A_12[i][j] = A[i][j + mid];
			A_21[i][j] = A[i + mid][j];
			A_22[i][j] = A[i + mid][j + mid];

			B_11[i][j] = B[i][j];
			B_12[i][j] = B[i][j + mid];
			B_21[i][j] = B[i + mid][j];
			B_22[i][j] = B[i + mid][j + mid];
		}
	}

	// M1 = (A11 + A22) * (B11 + B22)
	MatrixSum(mid, A_11, A_22, temp_A);
	MatrixSum(mid, B_11, B_22, temp_B);
	MatrixMult_Strassen(mid, A_11, A_22, temp_A);

	// M2 = (A21 + A22) * B11
	MatrixSum(mid, A_21, A_22, temp_A);
	MatrixMult_Strassen(mid, temp_A, B_11, M_2);

	// M3 = A11 * (B12-B22)
	MatrixSubs(mid, B_12, B_22, temp_B);
	MatrixMult_Strassen(mid, A_11, temp_B, M_3);

	// M4 = A22 * (B21 - B11)
	MatrixSubs(mid, B_21, B_11, temp_B);
	MatrixMult_Strassen(mid, A_22, temp_B, M_4);

	// M5 = (A11 + A12) * B22
	MatrixSum(mid, A_11, A_12, temp_A);
	MatrixMult_Strassen(mid, temp_A, B_22, M_5);

	// M6 = (A21 - A11) * (B11 + B12)
	MatrixSubs(mid, A_21, A_11, temp_A);
	MatrixSum(mid, B_11, B_12, temp_B);
	MatrixMult_Strassen(mid, temp_A, temp_B, M_6);

	// M7 = (A12 - A22) * (B21 + B22)
	MatrixSubs(mid, A_12, A_22, temp_A);
	MatrixSum(mid, B_21, B_22, temp_B);
	MatrixMult_Strassen(mid, temp_A, temp_B, M_7);

	// C11 = M1 + M4 - M5 + M7
	MatrixSum(mid, M_1, M_4, temp_A);
	MatrixSubs(mid, temp_A, M_5, temp_B);
	MatrixSum(mid, temp_B, M_7, C_11);

	// C21 = M2 + M4
	MatrixSum(mid, M_3, M_5, C_12);

	// C21 = M2 + M4
	MatrixSum(mid, M_2, M_4, C_21);

	// C22 = M1 - M2 + M3 + M6
	MatrixSum(mid, M_1, M_3, temp_A);
	MatrixSubs(mid, temp_A, M_2, temp_B);
	MatrixSum(mid, temp_B, M_6, C_22);

	for (i = 0; i < mid; i++)
	{
		for (j = 0; j < mid; j++)
		{
			C[i][j]             = C_11[i][j];
			C[i][j + mid]       = C_12[i][j];
			C[i + mid][j]       = C_21[i][j];
			C[i + mid][j + mid] = C_22[i][j];
		}
	}
}