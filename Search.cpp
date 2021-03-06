#include <iostream>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "porter2_stemmer.h"
#include "utility.h"

// 시간출력위해서
#include <chrono>
#include <ctime>

using namespace std;

struct word {
	int index_no; // 색인어 id
	int DF;
	int CF;
	int start; // 역색인 table에서의 시작위치
	word() {
		index_no = 0;
		DF = 0;
		CF = 0;
		start = 0;
	}
};

struct document {
	int ID;
	string name;
	int len;
};

struct article {
	string docno;
	string headline;
	string text;
};

struct topic {
	string num;
	string desc;
	string title;
	string narr;
	topic() {
		num = " ";
		desc = " ";
		title = " ";
		narr = " ";
	}
};

// 역색인 구조체 색인어id/문서id/TF/weight
struct iindex {
	int index_id;
	int doc_id;
	int tf;
	double weight;
	iindex() {
		index_id = 0;
		doc_id = 0;
		tf = 0;
		weight = 0;
	}
};
bool pair_sort_hit(const pair<int, int>& lhs, const pair<int, int>& rhs) {
	return lhs.second > rhs.second;
}
bool pair_sort(const pair<int, double>& lhs, const pair<int, double>& rhs) {
	return lhs.second > rhs.second;
}
bool pair_sort_lm(const pair<int, double>& lhs, const pair<int, double>& rhs) {
	return lhs.second < rhs.second;
}
bool compare(const iindex& i1, const iindex& i2) {
	if (i1.index_id != i2.index_id)
		return i1.index_id < i2.index_id;
	else
		return i1.doc_id < i2.doc_id;
}

unordered_map<string, string> sirregular(); // irregularverbs.txt를 불러와서 map에 저장
unordered_set<string> storeStopwd(); // stopword.txt를 불러와서 set에 저장
string stem(string t, int doc_count, ofstream& f); // stemming 과정을 처리해주는 함수
void topic_Stem(topic t, map<int, map<string, int>>& list);  // query topic stemming
string format_digit(int num_digit, int num); // 자리수 맞추는 함수
string format_weight(double weight); // 역색인파일 형식을 맞춰주는 함수
void utility::get_file_paths(LPCWSTR current, vector<string>& paths); // data 경로를 저장하는 함수
vector<pair<int, double>> lmcal(map<string, int> query, vector<pair<int, int>> RelDoc_Set, double Sum_Of_CF, map<int, map<string, int>> doc_word_TF); // Language Model Smoothing
void return_RelDoc_name(int Qnum, vector<pair<int, double>> cs, ofstream& of); // relevant document의 이름을 return
vector<pair<int, int>> Reldoc_Set(map<string, int> query, map<int, map<string, int>>& doc_word_TF); // relevant document set을 return
//vector<pair<int, double>> cscal(map<string, int> query);

// 전역변수 선언
int index_id = 1;
int doc_size = 0;
int document_length = 0;
unordered_set<string> stopwordlist = storeStopwd(); // stopword를 unordered set에 저장
unordered_map<string, string> irrlist = sirregular(); // 불규칙동사 unordered map에 저장
map<string, word> wordlist; // 전체 단어 저장

void main()
{
	ifstream file;
	vector<string> paths;
	article article;
	document document;
	string line,line2;
	string result = "";
	string result2 = "";
	int i = 0;
	int Tagsize = 0;
	int begin = 0;
	int end = 0;
	int doc_count = 1;
	bool state = false;
	bool state1 = false;
	bool state2 = false;
	double dnom = 0;
	char data[100];

	//// 시간 측정을 위해서
	//std::chrono::time_point<std::chrono::system_clock> tstart, tend;
	//tstart = std::chrono::system_clock::now();
	//// 파일 path 저장
	//utility::get_file_paths(TEXT(".\\ir_corpus"), paths);

	//ofstream outTF("TF.dat", ios::ate); // TF 파일
	//ofstream outDoc("doc.dat", ios::ate); // 문서 정보 파일
	//for (i = 0; i < paths.size(); i++) {
	//	cout << "File Open :" << paths[i];
	//	file.open(paths[i]);
	//	if (file.is_open()) {
	//		while (!file.eof()) {
	//			getline(file, line);
	//			// DOCNO Parsing
	//			if (line.find("</DOCNO>") != string::npos) state = false;
	//			if (state) article.docno += line + " ";
	//			if ((begin = line.find("<DOCNO>")) != string::npos) {
	//				if ((end = line.find("</DOCNO>")) != string::npos) {
	//					Tagsize = string("<DOCNO>").size();
	//					article.docno += line.substr(begin + Tagsize, end - Tagsize);
	//					state = false;
	//				}
	//				else state = true;
	//			}

	//			// Headline Parsing
	//			if (line.find("</HEADLINE>") != string::npos) state1 = false;
	//			if (state1) article.headline += line + " ";
	//			if ((begin = line.find("<HEADLINE>")) != string::npos) {
	//				if ((end = line.find("</HEADLINE>")) != string::npos) {
	//					Tagsize = string("<HEADLINE>").size();
	//					article.headline += line.substr(begin + Tagsize, end - Tagsize);
	//					state1 = false;
	//				}
	//				else state1 = true;
	//			}

	//			// Text Parsing
	//			if (line.find("</TEXT>") != string::npos) state2 = false;
	//			if (state2) article.text += line + " ";
	//			if ((begin = line.find("<TEXT>")) != string::npos) {
	//				Tagsize = string("<TEXT>").size();
	//				if ((end = line.find("</TEXT>")) != string::npos) {
	//					article.text += line.substr(begin + Tagsize, end - Tagsize);
	//					state2 = false;
	//				}
	//				else {
	//					article.text += line.substr(begin + Tagsize, line.size());
	//					state2 = true;
	//				}
	//			}

	//			// /doc를 찾으면 1개 parsing 끝
	//			if (line.find("</DOC>") != string::npos) {
	//				// stemming
	//				result2 = stem(article.text + " " + article.headline, doc_count, outTF);
	//				// <문서 정보 파일> -> doc.dat
	//				document.ID = doc_count++;
	//				document.name = article.docno;
	//				document.len = document_length;

	//				result = format_digit(6, document.ID) + document.name + format_digit(4, document.len) + "\n";
	//				result.erase(remove(result.begin(), result.end(), ' '), result.end());
	//				
	//				memcpy(data, result.c_str(), sizeof(string));
	//				outDoc.write(data, 27);

	//				result2 = "";
	//				article.docno = "";
	//				article.headline = "";
	//				article.text = "";
	//			}
	//		}// end while
	//	}// end if 
	//	file.close();
	//	cout << "	처리완료" << endl;
	//}// for Loop
	//
	//outTF.close(); // TF 파일 CLOSE
	//outDoc.close(); // 문서정보파일 CLOSE
	//doc_size = doc_count; // 문서 총 개수
	//cout << "총 문서 개수 :" << doc_size << endl;
	//cout << "총 단어 개수 :" << wordlist.size() << endl;
	//cout << "역색인 파일 & 단어정보파일 생성중..." << endl;

	//i = 0; // 시작위치 counting
	//int j = 1;
	//int doc_id = 1;
	//int id, tf;
	//string sword;
	//unordered_map<string, int> TF;
	//map<string, word>::const_iterator it;
	//iindex itmp;
	//vector<iindex> iindexlist; // 역색인 파일에 쓸 내용을 임시로 저장
	//
	//ofstream outTerm("term.dat", ios::ate); // 단어 정보 파일
	//for (auto& x : wordlist) {
	//	x.second.index_no = index_id++; // index id를 부여함
	//	// 단어정보파일 -> term.dat 생성 <색인어ID/색인어/DF/CF/시작위치>
	//	result = to_string(x.second.index_no) + "	" + x.first + "	" + to_string(x.second.DF) + "	" + to_string(x.second.CF) + "	" + to_string(i) + "\n";
	//	memcpy(data,result.c_str(),sizeof(string)*2);
	//	outTerm.write(data,result.size());
	//	i = i + x.second.DF;//시작위치 count
	//}
	//outTerm.close(); // 단어정보파일 CLOSE

	//file.open("TF.dat");
	//while (file >> id) {
	//	if (j != id) {
	//		for (auto y : TF) {
	//			dnom += pow(log((y.second) + 1.0)*(log(doc_size / wordlist.find(y.first)->second.DF)), 2); // 분모계산
	//		}
	//		for (auto x : TF) {
	//			it = wordlist.find(x.first);
	//			itmp.doc_id = j;
	//			itmp.index_id = it->second.index_no;
	//			itmp.tf = x.second;
	//			itmp.weight = (1 + log(x.second))*log(doc_size / it->second.DF) / sqrt(dnom);
	//			iindexlist.push_back(itmp); // 역색인 파일 정렬을 위해서 일단 vector에 저장
	//		}
	//		TF.clear();
	//		dnom = 0;
	//		j++;
	//	}
	//	file >> sword;
	//	file >> tf;
	//	TF.insert(make_pair(sword, tf));
	//}
	//file.close(); // TF.dat 파일 CLOSE
	//for (auto y : TF) {
	//	dnom += pow(log((y.second) + 1.0)*(log(doc_size / wordlist.find(y.first)->second.DF)), 2); // 분모계산
	//}
	//for (auto x : TF) {
	//	it = wordlist.find(x.first); // iterator
	//	itmp.doc_id = j;
	//	itmp.index_id = it->second.index_no;
	//	itmp.tf = x.second;
	//	itmp.weight = (1 + log(x.second))*log(doc_size / it->second.DF) / sqrt(dnom);
	//	iindexlist.push_back(itmp); // 역색인 파일 정렬을 위해서 일단 vector에 저장
	//}
	//TF.clear();
	//dnom = 0;
	//j++;
	//
	//sort(iindexlist.begin(),iindexlist.end(),compare); // 역색인 파일 정렬

	//ofstream outIndex("index.dat", ios::ate); // 역색인 파일
	//for (i = 0; i < iindexlist.size(); i++) {
	//	// 역색인 파일만들기 index.dat 생성 <색인어id/문서id/TF/weight>
	//	result = format_digit(6, iindexlist[i].index_id) + format_digit(6, iindexlist[i].doc_id) + format_digit(3, iindexlist[i].tf) + format_weight(iindexlist[i].weight) + "\n";
	//	memcpy(data, result.c_str(), sizeof(string));
	//	outIndex.write(data,23);
	//}
	//iindexlist.clear();
	//outIndex.close(); // 역색인파일 CLOSE
	//cout << "역색인 파일 & 단어정보파일 생성완료!!!" << endl; 

	string tmp[4];
	word tmpword;
	double Sum_Of_CF = 0;
	file.open("term.dat");
	if (file.is_open()) {
		while (getline(file, line)) {
			if (line == " " && line=="") break;
			stringstream sstmp(line);
			sstmp >> tmp[0] >> tmp[1] >> tmp[2] >> tmp[3] >> tmp[4];
			tmpword.index_no = stoi(tmp[0].c_str());
			tmpword.DF = stoi(tmp[2].c_str());
			tmpword.CF = stoi(tmp[3].c_str());
			tmpword.start = stoi(tmp[4].c_str());
			wordlist.insert(make_pair(tmp[1],tmpword));
			Sum_Of_CF += tmpword.CF;
		}
	}
	file.close(); // term.dat CLOSE
	cout << "Term.dat 처리 끝!!!" << endl;
	// 시간 측정을 위해서
	std::chrono::time_point<std::chrono::system_clock> tstart, tend;
	tstart = std::chrono::system_clock::now();

	topic topic25;
	state = false;
	state2 = false;
	stringstream st;
	map<int, map<string, int>> query; // Query -> stemming -> TF 계산

	// Query 정련 
	file.open("topics25.txt");
	if (file.is_open()) {
		while (getline(file, line)) {
			if (line.find("<num>") != string::npos) {
				line2 = line.substr(line.size() - 4, 4);
				line2.erase(remove(line2.begin(), line2.end(), ' '), line2.end());
				topic25.num = line2;
			}
			if ((begin=line.find("<title>")) != string::npos) {
				line2 = line.substr(begin + 7, line.size() - 7);
				topic25.title = line2;
			} 

			if (line.find("<narr>") != string::npos) state = false;
			if (state) topic25.desc += line + " ";
			if (line.find("<desc>") != string::npos) state = true;

			if (line.find("</top>") != string::npos) state2 = false;
			if (state2) topic25.narr += line + " ";
			if (line.find("<narr>") != string::npos) state2 = true; 

			if (line.find("</top>") != string::npos) {
				topic_Stem(topic25, query); // query에 대한 stemming, trimming, TF계산 진행
				topic25.num = " ";
				topic25.desc = " ";
				topic25.title = " ";
				topic25.narr = " ";
			}
		}
	}
	file.close(); // topic25 CLOSE
	stopwordlist.clear(); // stopwordlist CLEAR
	irrlist.clear(); // 불규칙동사 CLEAR
	cout << "Query 정련 끝!!!" << endl;

	vector<pair<int, double>> ranking;
	vector<string> docname;
	ofstream out;

	//out.open("result.txt", ios::ate);
	//for (auto x : query) {
	//	// 1개 쿼리 단위로 qword,qtf 넘김
	//	ranking = cscal(x.second);
	//	return_RelDoc_name(x.first , ranking, out);
	//	ranking.clear();
	//}
	//out.close();
	//cout << "VSM 결과 출력 완료" << endl;

	vector<pair<int, int>> Rdoc_Set; // relevant document set을 저장. query 단어와 매칭 수가 많은수대로 상위 1000개 
	map<int, map<string, int>> doc_word_TF;

	// Language Model - Dirichlet Smoothing
	cout << "언어모델 시작!!!" << endl;
	out.open("result.txt");
	for (auto x : query) {
		Rdoc_Set = Reldoc_Set(x.second, doc_word_TF); // 각 쿼리별 단어set을 인자로 넘기고 relevant set을 return 받음
		ranking = lmcal(x.second, Rdoc_Set, Sum_Of_CF, doc_word_TF);
		return_RelDoc_name(x.first, ranking, out);
		doc_word_TF.clear();
		Rdoc_Set.clear();
		ranking.clear();
	}
	out.close();
	cout << "언어모델 결과 출력 완료!!!" << endl;

	//시간측정
	tend = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = tend - tstart;
	std::time_t end_time = std::chrono::system_clock::to_time_t(tend);
	std::cout << "종료시간 :" << std::ctime(&end_time)
		<< "총 소요시간: " << elapsed_seconds.count() << "초" << endl;
	exit(0);
}

// relevant document의 이름을 return
void return_RelDoc_name(int Qnum, vector<pair<int, double>> cs, ofstream& of) {
	int offset = 6 + 16 + 4; // 26
	string line = "";
	string line2 = "";
	ifstream f;
	char data[30];
	f.open("doc.dat");
	of << Qnum << endl;
	for (auto x : cs) {
		f.seekg((unsigned long long)((x.first-1)*(offset+2)), ios::beg);
		getline(f, line);
		line2 = line.substr(6, 16) + "\t";
		memcpy(data, line2.c_str(), sizeof(string));
		of.write(data, 17);
	}
	of << endl;
	f.close();
}

// document 길이를 return
int return_doclen(int id) {
	int offset = 6 + 16 + 4;
	string line = "";
	ifstream f;
	f.open("doc.dat");
	f.seekg((unsigned long long)((id - 1)*(offset + 2)), ios::beg);
	getline(f, line);
	f.close();
	return stoi(line.substr(23, 4));
}

// return sumOfCF and relevant document set <doc_id,doc_len> 
vector<pair<int, int>> Reldoc_Set(map<string, int> query, map<int, map<string, int>>& doc_word_TF_result) {
	ifstream f;
	map<int, int> Rsetmap;
	map<string, word>::const_iterator it; // wordlist iterator
	map<int, int>::iterator it2; // Rsetmap iterator
	int tmp = 0; // 문서 개수 counting
	int doc_id = 0;
	int offset = 6 + 6 + 3 + 7; // 역색인 line 길이 22
	unsigned long long index_start = 0;
	string line = "";
	vector<pair<int, int>> Rset;
	vector<pair<int, int>> result;

	map<int, map<string, int>> doc_word_TF; // 각 문서별 출현한 query 단어를 기록.
	map<int, map<string, int>>::iterator doc_word_TF_it;
	map<string, int> tmpp;
	int TF;
	double weight = 0;

	f.open("index.dat");
	for (auto x : query) {
		if ((it = wordlist.find(x.first)) != wordlist.end()) {
			index_start = it->second.start*(offset + 2); // index.dat 접근 start 위치 계산
			f.seekg(index_start, ios::beg);
			for (int i = 0; i < it->second.DF; i++) {
				getline(f, line);
				doc_id = stoi(line.substr(6,6));
				TF = stoi(line.substr(12, 3));
				weight = stod(line.substr(15,6)); // 임시

				if ((doc_word_TF_it = doc_word_TF.find(doc_id)) != doc_word_TF.end()) {
					doc_word_TF_it->second.insert(make_pair(x.first, TF));
				}
				else {
					tmpp.clear();
					tmpp.insert(make_pair(x.first, TF));
					doc_word_TF.insert(make_pair(doc_id, tmpp));
				}

				if ((it2 = Rsetmap.find(doc_id)) != Rsetmap.end()) {
					it2->second += x.second * weight; // doc hitting 수를 계산 query의 TF에 역색인의 weight를 곱해서 가중치 계산
				}
				else {
					Rsetmap.insert(make_pair(doc_id, x.second* weight));
				}

			} // end inner for loop
		} // end if 
	} // end for loop 
	f.close();

	// Rsetmap을 hit수로 정렬한다음에 상위 1000개의 문서만을 Rset에 insert해서 doc_id, doc_len이랑 같이 return 
	// 일단 vector 형식의 Rset에 다시 삽입함. 정렬을 위해.
	for (auto x : Rsetmap) {
		Rset.push_back(make_pair(x.first, x.second));
	}
	Rsetmap.clear();
	sort(Rset.begin(), Rset.end(), pair_sort_hit); // 가중치가 높은대로 계산
	for (auto& y : Rset) {
		y.second = return_doclen(y.first);
		result.push_back(make_pair(y.first, y.second));

		doc_word_TF_it = doc_word_TF.find(y.first);
		doc_word_TF_result.insert(make_pair(doc_word_TF_it->first, doc_word_TF_it->second));
		tmp++;
		if (tmp == 1000) break; // 상위 1000개의 문서만을 선택
	} // doc_len을 구해서 result에 <doc_id,doc_len> 으로 만들어 저장.

	doc_word_TF.clear();
	Rsetmap.clear();
	return result;
}

// Language Model - Dirichlet Smoothing 계산
vector<pair<int, double>> lmcal(map<string, int> query, vector<pair<int, int>> RelDoc_Set, double Sum_Of_CF, map<int, map<string, int>> doc_word_TF) {
	vector<pair<int, double>> result;
	map<string, word>::const_iterator it; // wordlist iterator
	double MU = 2000; // Dirichlet Smoothing 식에서의 뮤값
	double calc;
	int TF = 0;
	double CF = 0;

	map<int, map<string, int>>::const_iterator it2;
	map<string, int>::const_iterator it3;

	// RelDoc_Set 에서 doc_id, doc_len 넘어옴 
	// x.first=doc_id , x.second=doc_len
	for (auto x : RelDoc_Set) {
		calc = 0;
		it2 = doc_word_TF.find(x.first);
		for (auto y : query) {
			if ((it = wordlist.find(y.first)) != wordlist.end()) {
				CF = it->second.CF;
			} else CF = 0;

			if ((it3 = it2->second.find(y.first)) != it2->second.end()) {
				TF = it3->second;
			} else TF = 0;

			if (CF != 0)
				calc += log((TF + MU*CF/Sum_Of_CF) / (x.second + MU));
		}
		result.push_back(make_pair(x.first, calc));
	}
	sort(result.begin(), result.end(), pair_sort);

	//int tmpp=0;
	//vector<pair<int, double>> result2;
	//for (auto& x : result) {
	//	result2.push_back(make_pair(x.first, x.second));
	//	tmpp++;
	//	if (tmpp == 900) break; // recall-precision graph 그리기 위해 100*n개씩 출력하는중.
	//}
	//return result2;
	return result;
}

//// Query & document Cosine Similarity 계산
//vector<pair<int, double>> cscal(map<string, int> query) {
//	int offset = 6 + 6 + 3 + 7;
//	int tmp = 0;
//	ifstream f;
//	map<string, word>::iterator it; // wordlist iterator
//	map<int, pair<double, double>>::iterator it2; // cs iterator
//	vector<pair<int, double>> result; // doc_id, similarirty 들어감
//	string line = "";
//	int doc_id = 0;;
//	double weight = 0;
//
//	map<int, pair<double, double>> cs;
//
//	f.open("index.dat");
//	for (auto y : query) {
//		if ((it = wordlist.find(y.first)) != wordlist.end()) {
//			f.seekg((unsigned long long)(it->second.start*(offset + 2)), ios::beg);
//			for (int i = 0; i < it->second.DF; i++) {
//				getline(f, line);
//				doc_id = stoi(line.substr(6, 6));
//				weight = stod(line.substr(15, 7));
//				if ((it2 = cs.find(doc_id)) != cs.end()) {
//					it2->second.first += (weight*y.second);
//					it2->second.second += pow(weight,2);
//				}
//				else {
//					cs.insert(make_pair(doc_id, make_pair((weight*y.second), pow(weight, 2))));
//				}
//			}
//		}
//	} // cosine similarity 계산에 필요한 분자,분모 구하기 끝
//	f.close();
//
//	for (auto x : cs) {
//		result.push_back(make_pair(x.first, x.second.first / sqrt(x.second.second)));
//	}
//	sort(result.begin(), result.end(), pair_sort); // cosine similarity 높은 순으로 정렬

//
//	result.clear();
//	cs.clear();
//}

// stopword를 불러와서 unordered_set에 저장함
unordered_set<string> storeStopwd() {
	ifstream file("stopword.txt");
	string word;
	unordered_set<string> list;
	if (file.is_open()) {
		while (getline(file, word)) {
			list.insert(word);
		}
	} else cout << "stopword file을 읽지 못했습니다." << endl; 
	file.close();
	return list;
}

// 불규칙 동사를 불러와서 unodered_map에 저장함
unordered_map<string, string> sirregular() {
	ifstream file("irregularverbs.txt");
	int i = 0;
	string word;
	string value, key1, key2;
	string line;
	unordered_map<string,string> list;

	if (file.is_open()) {
		while (getline(file, line)) {
			i = 0;
			stringstream tmp(line);
			while (getline(tmp, word,'	')) {			
				// 첫번째 단어가 value
				// 두세번째 단어가 key
				if (i == 0) {
					value = word;
					i++;
				}
				else if (i == 1) {
					key1 = word;
					i++;
				}
				else if (i == 2)
					key2 = word;
			}
			list.insert(make_pair(key1, value));
			list.insert(make_pair(key2, value));
		}
	}
	file.close();
	return list;
}
string stem(string t, int doc_count, ofstream& f) {
	// headline 또는 text가 들어왔을 때 stemming 처리
	stringstream tmp(t);
	string index,result;
	word w;
	unordered_map<string, int> TF;
	unordered_map<string, string>::const_iterator it;
	unordered_map<string, int>::iterator it2;
	map<string, word>::iterator it3;

	document_length = 0;
	// word 단위로 쪼개져서 들어옴
	while (getline(tmp, index, ' ')) {
		if (index.find("http") != string::npos) index = "";
		if (index.find("www") != string::npos) index = "";
		if (index.find("'") != string::npos) index = "";
		if (index.find("-") != string::npos) index = " ";
		
		// trim 
		Porter2Stemmer::trim(index);

		// Remove Stopword
		if (stopwordlist.find(index) != stopwordlist.end()) index = "";

		// 불완전동사 처리
		if ((it = irrlist.find(index)) != irrlist.end()) index = it->second;

		// Stemming 
		Porter2Stemmer::stem(index);

		// Remove Stopword
		if (stopwordlist.find(index) != stopwordlist.end()) index = "";

		if (index != "" && index.at(0) == index.at(1)) index = "";

		// 공백제거
		if (index == "") result += index;
		else {
			result += " " + index;
			document_length++;	// 문서 길이 측정
		}

		// 임시 unordered_map TF에 임시로 저장하면서 wordlist에는 단어만 저장
		if ((it2 = TF.find(index)) != TF.end()) {
				it2->second += 1; // TF 계산
		} else {
			if (!(index == "")) {
				TF.insert(make_pair(index, 1));
			}
		}
	} // end while(getline)

	// unordered_map TF 에는 이번 문서에서 저장한 단어들과 TF계산한 것이 들어가있음
	for (auto& x : TF) {
		// 문서id, 단어, TF 를 TF.dat에 저장
		f << doc_count << "	" << x.first << "	" << x.second << endl;		
		// 전역 list에 insert
		if ((it3 = wordlist.find(x.first)) != wordlist.end()) {
			it3->second.DF += 1;
			it3->second.CF += x.second;
		} else {
			w.DF = 1;
			w.CF = x.second;
			wordlist.insert(make_pair(x.first, w));
			// index id는 나중에 줌.
		}
	}
	TF.clear();
	return result;
}
void topic_Stem(topic t, map<int, map<string,int>>& list) {
	stringstream ss;
	string line,result;
	unordered_map<string, string>::const_iterator it;
	map<string, int>::iterator it2;
	map<string, int> tmp;

	for (int i = 0; i < 3; i++) {
		ss.clear();
		if (i == 0) ss.str(t.narr);
		else if (i == 1) ss.str(t.desc);
		else if (i == 2) ss.str(t.title);

		while (getline(ss, line, ' ')) {
			Porter2Stemmer::trim(line); // Trim 
			if (stopwordlist.find(line) != stopwordlist.end()) line = ""; // Remove Stopword
			if ((it = irrlist.find(line)) != irrlist.end()) line = it->second; // 불완전동사 처리
			Porter2Stemmer::stem(line); // Stemming 
			if (stopwordlist.find(line) != stopwordlist.end()) line = ""; // Remove Stopword

			if ((it2 = tmp.find(line)) != tmp.end()) {
				if (i == 0) it2->second += 1; // TF계산
				else if (i == 1) it2->second += 5; // desc이면 5씩증가
				else if (i == 2) it2->second += 10; // title이면 tf증가를 10씩
			}
			else {
				if (!(line == "") && !(line == " ")) {
					if (i == 0) tmp.insert(make_pair(line, 1));
					else if (i == 1) tmp.insert(make_pair(line, 5));
					else if (i == 2) tmp.insert(make_pair(line, 10));
				}
			}
		}
	}
	list.insert(make_pair(stoi(t.num), tmp));
}

// 역색인파일 작성시 필요한 색인어ID,문서ID,TF를 원하는 자릿수로 표현
string format_digit(int digit, int num) { 
	string result = "";
	for (int i = 0; i < digit; ++i) {
		result += (to_string(num % 10));
		num /= 10;
	}
	reverse(result.begin(), result.end());
	return result;
}

// weight 자리수 맞춰주는 함수
string format_weight(double weight) {
	string results = "";
	ostringstream os;
	os << weight;

	string str = os.str();

	if (weight == (int)weight) {
		str.append(".0");
	}

	if (str.size() > 7) {
		str = str.substr(0, 7);
	}
	else {
		while (str.size() < 7) {
			str.append("0");
		}
	}
	return str;
}
// 디렉토리를 열어서 파일 경로를 가져오는 함수
void utility::get_file_paths(LPCWSTR current, vector<string>& paths) {
	WIN32_FIND_DATA FindFileData;
	TCHAR tmp[MAX_PATH];
	StringCchCopy(tmp, MAX_PATH, current);
	StringCchCat(tmp, MAX_PATH, TEXT("\\*"));

	HANDLE hFind = FindFirstFile(tmp, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}
	do {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (wcscmp(FindFileData.cFileName, TEXT(".")) != 0 && wcscmp(FindFileData.cFileName, TEXT("..")) != 0) {
				TCHAR next[MAX_PATH];
				StringCchCopy(next, MAX_PATH, current);
				StringCchCat(next, MAX_PATH, TEXT("\\"));
				StringCchCat(next, MAX_PATH, FindFileData.cFileName);
				get_file_paths(next, paths);
			}
		}
		else {
			TCHAR path[MAX_PATH];
			StringCchCopy(path, MAX_PATH, current);
			StringCchCat(path, MAX_PATH, TEXT("\\"));
			StringCchCat(path, MAX_PATH, FindFileData.cFileName);
			wstring ws = path;
			string tmp = "";
			tmp.assign(ws.begin(), ws.end());
			paths.push_back(tmp);
		}
	} while (FindNextFile(hFind, &FindFileData));

	FindClose(hFind);
	return;
}