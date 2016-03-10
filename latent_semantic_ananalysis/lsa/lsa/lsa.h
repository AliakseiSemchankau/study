#ifndef LSA
#define LSA

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <set>
#include "english_stem.h"
#include "svd.h"

class lsa
{
	std::vector<std::vector<long double>> term_document_matrix;
	std::map<std::wstring, size_t> terms;
	size_t num_of_documents;
	std::set<std::wstring> stop_words;

	void parse_and_stem(const char* name_of_start_document)
	{
		std::fstream start_document;
		start_document.open(name_of_start_document);
		start_document >> num_of_documents;
		std::fstream document_of_stop_words;
		document_of_stop_words.open("stop_words.txt");
		while (!document_of_stop_words.eof())
		{
			std::string current_word;
			document_of_stop_words >> current_word;
			std::wstring cur_wstr(current_word.begin(), current_word.end());
			stop_words.insert(cur_wstr);
		}
		document_of_stop_words.close();

		std::fstream cur_document;
		for (size_t i = 0; i < num_of_documents; ++i)
		{
			std::string name_of_cur_document;
			start_document >> name_of_cur_document;
			cur_document.open(name_of_cur_document);

			stemming::english_stem<> StemEnglish;

			char cur_symbol;
			std::string cur_word;
			while (!cur_document.eof())
			{
				cur_document.get(cur_symbol);
				if ((cur_symbol >= 'a' && cur_symbol <= 'z') || (cur_symbol >= 'A' && cur_symbol <= 'Z'))
				{
					if (cur_symbol >= 'A' && cur_symbol <= 'Z')
						cur_symbol -= ('A' - 'a');
					cur_word.push_back(cur_symbol);
				}
				else
				{
					if (cur_word.length() > 2)
					{
						std::wstring cur_wstr(cur_word.begin(), cur_word.end());
						StemEnglish(cur_wstr);

						if (stop_words.find(cur_wstr) == stop_words.end())
						{
							if (terms.find(cur_wstr) == terms.end())
							{
								std::vector<long double> term_frequency(num_of_documents);
								term_document_matrix.push_back(term_frequency);
								size_t cur_row = term_document_matrix.size() - 1;
								terms[cur_wstr] = cur_row;
								++term_document_matrix[cur_row][i];
							}
							else
							{
								++term_document_matrix[terms[cur_wstr]][i];
							}
						}
					}
					cur_word.erase();
				}
			}
			cur_document.close();
		}
		start_document.close();
	}

	void tf_idf()
	{
		std::vector<long double> num_of_words_in_documents;
		num_of_words_in_documents.assign(num_of_documents, 0);
		for (size_t j = 0; j < num_of_documents; ++j)
			for (size_t i = 0; i < term_document_matrix.size(); ++i)
				num_of_words_in_documents[j] += term_document_matrix[i][j];
		std::vector<long double> num_of_documents_with_word;
		num_of_documents_with_word.assign(term_document_matrix.size(), 0);
		for (size_t i = 0; i < term_document_matrix.size(); ++i)
			for (size_t j = 0; j < num_of_documents; ++j)
				if (term_document_matrix[i][j] != 0)
					++num_of_documents_with_word[i];
		for (size_t i = 0; i < term_document_matrix.size(); ++i)
			for (size_t j = 0; j < num_of_documents; ++j)
				term_document_matrix[i][j] = (term_document_matrix[i][j] / num_of_words_in_documents[j]) * log(static_cast<long double>(num_of_documents) / num_of_documents_with_word[i]);
	}

	long double cos_sim(std::vector<long double>& first, std::vector<long double>& second)
	{
		long double temp = 0;
		for (size_t i = 0; i < first.size(); ++i)
			temp += first[i] * second[i];
		auto first_norm = norm_of_vector(first);
		auto second_norm = norm_of_vector(second);
		if (abs(first_norm) > EPS && abs(second_norm) > EPS)
			return (temp / norm_of_vector(first)) / norm_of_vector(second);
		else
			return 0;
	}

	std::vector<long double> get_column(size_t k)
	{
		std::vector<long double> column(term_document_matrix.size());
		for (size_t i = 0; i < term_document_matrix.size(); ++i)
			column[i] = term_document_matrix[i][k];
		return column;
	}

public:
	lsa(const char* name_of_start_document)
	{
		parse_and_stem(name_of_start_document);
		tf_idf();
	}

	std::vector<size_t> query(size_t q_document, size_t k)
	{
		term_document_matrix = svd_approximation(term_document_matrix);
		std::vector<size_t> res;
		std::multimap<long double, size_t> similarity;
		auto q_column = get_column(q_document);
		for (size_t i = 0; i < num_of_documents; ++i)
			if (i != q_document)
				similarity.insert(std::make_pair(cos_sim(get_column(i), q_column), i));
		int count = 0;
		for (auto it = similarity.rbegin(); it != similarity.rend(); ++it)
		{
			++count;
			res.push_back(it->second);
			if (count == k)
				break;
		}
		return res;
	}
};

#endif