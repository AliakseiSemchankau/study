#ifndef SVD
#define SVD

#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <thread>

typedef std::vector<std::vector<long double> > matrix;

#define EPS 1e-9

long double sign(long double number)
{
	if (number > 0)
		return 1;
	if (number < 0)
		return -1;
	if (abs(number) <EPS)
		return 0;
}

void print_matrix(matrix m) 
{
	for (auto i = m.begin(); i != m.end(); ++i) 
	{
		for (auto j = i->begin(); j != i->end(); ++j)
			std::cout << *j << " ";
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

std::vector <long double> mult_row(const matrix& A, const std::vector<long double>& row)
{
	std::vector <long double> answer(A[0].size(), 0);
	for (size_t i = 0; i < answer.size(); ++i)
		for (size_t j = 0; j < row.size(); ++j)
			answer[i] += A[j][i] * row[j];
	return answer;
}

matrix dot(matrix a, matrix b)
{
	size_t num_of_threads = 4;
	std::vector<std::thread> workers;
	size_t n = a.size();
	size_t k = a[0].size();
	if (k != b.size())
		throw std::exception();
	size_t m = b[0].size();
	matrix result(n, std::vector<long double>(m, 0));

	size_t size_of_task = n / num_of_threads;

	auto worker = [&](size_t num_of_thread)
	{
		if (num_of_thread < num_of_threads - 1)
		{
			for (size_t k = num_of_thread * size_of_task; k < (num_of_thread + 1) * size_of_task; ++k)
				result[k] = mult_row(b, a[k]);
		}
		else
		{
			for (size_t k = num_of_thread * size_of_task; k < n; ++k)
				result[k] = mult_row(b, a[k]);
		}
	};

	for (size_t i = 0; i < num_of_threads; ++i)
		workers.emplace_back(worker, i);
	for (auto &i : workers)
		i.join();
	return result;
}

matrix minus(matrix a, matrix b) 
{
	size_t num_of_threads = 1;
	std::vector<std::thread> workers;
	matrix result(a.size(), std::vector<long double>(a[0].size(), 0));

	if (a.size() != b.size() || a[0].size() != b[0].size()) 
		throw std::exception();
	
	size_t size_of_task = a.size() / num_of_threads;

	auto worker = [&](size_t num_of_thread)
	{
		if (num_of_thread < num_of_threads - 1)
		{
			for (size_t k = num_of_thread * size_of_task; k < (num_of_thread + 1) * size_of_task; ++k)
				for (size_t j = 0; j < a[0].size(); ++j)
					result[k][j] = a[k][j] - b[k][j];
		}
		else
		{
			for (size_t k = num_of_thread * size_of_task; k < a.size(); ++k)
				for (size_t j = 0; j < a[0].size(); ++j)
					result[k][j] = a[k][j] - b[k][j];
		}
	};

	for (size_t i = 0; i < num_of_threads; ++i)
		workers.emplace_back(worker, i);

	for (auto &i : workers)
		i.join();

	return result;
}

matrix scalar(matrix a, long double alpha) 
{
	matrix result(a.size(), std::vector<long double>(a[0].size(), 0));
	for (size_t i = 0; i < a.size(); ++i)
		for (size_t j = 0; j < a[0].size(); ++j) 
			result[i][j] = alpha * a[i][j];
	return result;
}

matrix transpose(matrix A)
{
	matrix tr(A[0].size(), std::vector<long double>(A.size(), 0));
	for (size_t i = 0; i < A.size(); ++i)
		for (size_t j = 0; j < A[0].size(); ++j)
			tr[j][i] = A[i][j];
	return tr;
}

matrix get_aug(matrix A) 
{
	size_t n = A.size();
	size_t m = A[0].size();

	matrix aug(m + n, std::vector<long double>(m + n, 0));

	for (size_t i = 0; i < n; ++i) 
		for (size_t j = 0; j < m; ++j)
			aug[i][j + n] = A[i][j];

	matrix transp = transpose(A);

	for (size_t i = 0; i < m; ++i)  
	{
		for (size_t j = 0; j < n; ++j) 
			aug[i + n][j] = transp[i][j];
	}

	return aug;
}

matrix eye(size_t size)
{
	matrix E = matrix(size, std::vector<long double>(size, 0.0));
	for (size_t i = 0; i < size; ++i)
		E[i][i] = 1.0;
	return E;
}

long double norm_of_vector(const std::vector<long double>& v)
{
	long double res = 0;
	for (auto &i : v)
		res += i*i;
	return sqrt(res);
}

std::pair<matrix, matrix> householder_qr(const matrix& a)
{
	matrix buffer = a;
	matrix Q = eye(a.size());
	matrix R(a.size(), std::vector<long double>(a[0].size(), 0));
	
	for (size_t step = 0; step < a.size() - 1; ++step)
	{
		std::vector<long double> v(a.size(), 0);
		long double temp = 0;
		std::vector<std::pair<size_t, size_t>> swapper;
		for (size_t i = step; i < a.size(); ++i)
			if (buffer[i][step] != 0)
			{
				swapper.push_back(std::make_pair(i, step));
				swap(buffer[step], buffer[i]);
				break;
			}
		for (size_t i = step; i < a.size(); ++i)
			temp += buffer[i][step] * buffer[i][step];
		v[step] = sign(buffer[step][step]) * sqrt(temp) + buffer[step][step];
		for (size_t i = step + 1; i < buffer.size(); ++i)
			v[i] = buffer[i][step];
		
		matrix back_local;
		back_local.push_back(v);
		matrix forward_local = transpose(back_local);
		matrix vv_t = dot(forward_local, back_local);
		matrix v_t_v = dot(back_local, forward_local);
		
		if (v_t_v[0][0] == 0)
			continue;

		matrix res = scalar(vv_t, 2.0 / v_t_v[0][0]);

		auto I = eye(a.size());
	
		auto H = minus(I, res);
		
		buffer = dot(H, buffer);
		
		for (int j = swapper.size() - 1; j >= 0; --j)
			std::swap(Q[swapper[j].first], Q[swapper[j].second]);
		Q = dot(H,Q);
	}

	R = buffer;
	return std::make_pair(transpose(Q), R);
}

std::pair<std::vector<long double>, matrix> qr_algorithm(const matrix& a) 
{
	std::pair<matrix, matrix> factor(householder_qr(a));
	//print_matrix(factor.first);
	//print_matrix(factor.second);
	
	matrix t = a;
	matrix eigenvectors = factor.first;
	for (size_t i = 0; i < 5; ++i) 
	{
		t = dot(factor.second, factor.first);
		factor = householder_qr(t);
		eigenvectors = dot(eigenvectors, factor.first);
	}
	std::vector<long double> eigenvalues;
	for (size_t i = 0; i < t.size(); ++i)
		eigenvalues.push_back(t[i][i]);
	for (size_t i = 0; i < t.size(); ++i)
		scalar(eigenvectors, norm_of_vector(eigenvectors[i]));
	transpose(eigenvectors);
	return std::make_pair(eigenvalues, eigenvectors);
}

matrix svd_approximation(matrix B)
{
	matrix left = dot(B, transpose(B));
	auto left_res = qr_algorithm(left);
	matrix right = dot(transpose(B), B);
	auto right_res = qr_algorithm(right);
	matrix U = left_res.second;
	matrix V = right_res.second;
	size_t m = B.size();
	size_t n = B[0].size();
	matrix sigma(m, std::vector<long double>(n, 0));
	size_t k = 0;
	for (size_t i = 0; i < B.size(); ++i)
	{
		if (abs(left_res.first[i]) > EPS)
			sigma[i][i] = sqrt(left_res.first[i]);
		++k;
		if (k == 10)
			break;
	}
	return dot(dot(U, sigma), transpose(V));
	//print_matrix(U);
	//print_matrix(sigma);
	//print_matrix(V);
}

/*std::vector<double> lanczos(matrix B)
{
	matrix w_prev(1, std::vector<double>(B.size(), 0));
	matrix w_cur(1, std::vector<double>(B.size(), 0));
	w_cur[0][0] = 1;
	std::vector<double> beta(B.size(), 0);
	std::vector<double> alpha(B.size(), 0);
	for (size_t i = 0; i < B.size() - 1; ++i)
	{
		auto mult_res_prev = dot(B, transpose(w_cur));
		auto mult_res_cur = scalar(w_prev, beta[i]);
		auto temp = dot(w_cur, minus(mult_res_prev, transpose(mult_res_cur)));
		alpha[i] = temp[0][0];
		auto mult_temp = dot(B, transpose(w_cur));
		mult_temp = minus(mult_temp, transpose(scalar(w_cur, alpha[i])));
		mult_temp = minus(mult_temp, transpose(scalar(w_prev, beta[i])));
		mult_temp = transpose(mult_temp);
		beta[i + 1] = norm_of_vector(mult_temp[0]);
		w_prev = w_cur;
		w_cur = scalar(mult_temp, 1.0 / beta[i + 1]);
	}
	matrix Tj(B.size() , std::vector<double>(B.size(), 0));
	Tj[0][0] = alpha[0];
	Tj[0][1] = beta[1];
	for (size_t i = 1; i < B.size() - 1; ++i)
	{
		Tj[i][i - 1] = beta[i];
		Tj[i][i] = alpha[i];
		Tj[i][i + 1] = beta[i + 1];
	}
	Tj[B.size() - 1][B.size() - 2] = beta[B.size() - 1];
	Tj[B.size() - 1][B.size() - 1] = alpha[B.size() - 1];
	auto res = qr_algorithm(Tj);
	return res.first;
}
*/

#endif
