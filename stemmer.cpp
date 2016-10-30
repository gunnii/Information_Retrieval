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

// �ð�������ؼ�
#include <chrono>
#include <ctime>

using namespace std;

struct word {
	int index_no; // ���ξ� id
	int DF;
	int CF;
	int start; // ������ table������ ������ġ
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

// ������ ����ü ���ξ�id/����id/TF/weight
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
bool pair_sort(const pair<int, double>& lhs, const pair<int, double>& rhs) {
	return lhs.second > rhs.second;
}
bool compare(const iindex& i1, const iindex& i2) {
	if (i1.index_id != i2.index_id)
		return i1.index_id < i2.index_id;
	else
		return i1.doc_id < i2.doc_id;
}

unordered_map<string, string> sirregular(); // irregularverbs.txt�� �ҷ��ͼ� map�� ����
unordered_set<string> storeStopwd(); // stopword.txt�� �ҷ��ͼ� set�� ����
string stem(string t, int doc_count, ofstream& f); // stemming ������ ó�����ִ� �Լ�
void topic_Stem(topic t, map<int, map<string, int>>& list);  // topic stemming
string format_digit(int num_digit, int num); // �ڸ��� ���ߴ� �Լ�
string format_weight(double weight); // ���������� ������ �����ִ� �Լ�
void utility::get_file_paths(LPCWSTR current, vector<string>& paths); // data ��θ� �����ϴ� �Լ�
vector<pair<int, double>> cscal(map<string, int> query);
vector<pair<int, double>> lmcal(map<string, int> query);
void return_RelDoc(int Qnum, vector<pair<int, double>> cs, ofstream& of); // relevant document�� �̸��� return


// �������� ����
int index_id = 1;
int doc_size = 0;
int document_length = 0;
unordered_set<string> stopwordlist = storeStopwd(); // stopword�� unordered set�� ����
unordered_map<string, string> irrlist = sirregular(); // �ұ�Ģ���� unordered map�� ����
map<string, word> wordlist; // ��ü �ܾ� ����

void main()
{
	ifstream file;
	vector<string> paths;
	article article;
	document document;
	string line,line2;
	string result = "";
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

	// ���� path ����
	utility::get_file_paths(TEXT(".\\ir_corpus"), paths);

	//ofstream outTF("TF.dat", ios::ate); // TF ����
	//ofstream outDoc("doc.dat", ios::ate); // ���� ���� ����
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

	//			// /doc�� ã���� 1�� parsing ��
	//			if (line.find("</DOC>") != string::npos) {

	//				// stemming
	//				article.headline = stem(article.headline, doc_count, outTF);
	//				article.text = stem(article.text, doc_count, outTF);

	//				// <���� ���� ����> -> doc.dat
	//				document.ID = doc_count++;
	//				document.name = article.docno;
	//				document.len = document_length;

	//				result = format_digit(6, document.ID) + document.name + format_digit(4, document.len)+"\n";
	//				result.erase(remove(result.begin(), result.end(), ' '), result.end());
	//				memcpy(data, result.c_str(), sizeof(string));
	//				outDoc.write(data, 27);

	//				article.docno = "";
	//				article.headline = "";
	//				article.text = "";
	//			}
	//		}// end while
	//	}// end if 
	//	file.close();
	//	cout << "	ó���Ϸ�" << endl;
	//}// for Loop
	//
	//outTF.close(); // TF ���� CLOSE
	//outDoc.close(); // ������������ CLOSE
	//doc_size = doc_count; // ���� �� ����
	//cout << "�� ���� ���� :" << doc_size << endl;
	//cout << "�� �ܾ� ���� :" << wordlist.size() << endl;
	//cout << "������ ���� & �ܾ��������� ������..." << endl;

	//i = 0; // ������ġ counting
	//int j = 1;
	//int doc_id = 1;
	//int id, tf;
	//string sword;
	//unordered_map<string, int> TF;
	//map<string, word>::const_iterator it;
	//iindex itmp;
	//vector<iindex> iindexlist; // ������ ���Ͽ� �� ������ �ӽ÷� ����
	//
	//ofstream outTerm("term.dat", ios::ate); // �ܾ� ���� ����
	//for (auto& x : wordlist) {
	//	x.second.index_no = index_id++; // index id�� �ο���
	//	// �ܾ��������� -> term.dat ���� <���ξ�ID/���ξ�/DF/CF/������ġ>
	//	result = to_string(x.second.index_no) + "	" + x.first + "	" + to_string(x.second.DF) + "	" + to_string(x.second.CF) + "	" + to_string(i) + "\n";
	//	memcpy(data,result.c_str(),sizeof(string)*2);
	//	outTerm.write(data,result.size());
	//	i = i + x.second.DF;//������ġ count
	//}
	//outTerm.close(); // �ܾ��������� CLOSE

	//file.open("TF.dat");
	//while (file >> id) {
	//	if (j != id) {
	//		for (auto y : TF) {
	//			dnom += pow(log((y.second) + 1.0)*(log(doc_size / wordlist.find(y.first)->second.DF)), 2); // �и���
	//		}
	//		for (auto x : TF) {
	//			it = wordlist.find(x.first);
	//			itmp.doc_id = j;
	//			itmp.index_id = it->second.index_no;
	//			itmp.tf = x.second;
	//			itmp.weight = (1 + log(x.second))*log(doc_size / it->second.DF) / sqrt(dnom);
	//			iindexlist.push_back(itmp); // ������ ���� ������ ���ؼ� �ϴ� vector�� ����
	//		}
	//		TF.clear();
	//		dnom = 0;
	//		j++;
	//	}
	//	file >> sword;
	//	file >> tf;
	//	TF.insert(make_pair(sword, tf));
	//}
	//file.close(); // TF.dat ���� CLOSE
	//for (auto y : TF) {
	//	dnom += pow(log((y.second) + 1.0)*(log(doc_size / wordlist.find(y.first)->second.DF)), 2); // �и���
	//}
	//for (auto x : TF) {
	//	it = wordlist.find(x.first);
	//	itmp.doc_id = j;
	//	itmp.index_id = it->second.index_no;
	//	itmp.tf = x.second;
	//	itmp.weight = (1 + log(x.second))*log(doc_size / it->second.DF) / sqrt(dnom);
	//	iindexlist.push_back(itmp); // ������ ���� ������ ���ؼ� �ϴ� vector�� ����
	//}
	//TF.clear();
	//dnom = 0;
	//j++;
	//
	//sort(iindexlist.begin(),iindexlist.end(),compare); // ������ ���� ����

	//ofstream outIndex("index.dat", ios::ate); // ������ ����
	//for (i = 0; i < iindexlist.size(); i++) {
	//	// ������ ���ϸ���� index.dat ���� <���ξ�id/����id/TF/weight>
	//	result = format_digit(6, iindexlist[i].index_id) + format_digit(6, iindexlist[i].doc_id) + format_digit(3, iindexlist[i].tf) + format_weight(iindexlist[i].weight) + "\n";
	//	memcpy(data, result.c_str(), sizeof(string));
	//	outIndex.write(data,23);
	//}
	//iindexlist.clear();
	//outIndex.close(); // ���������� CLOSE
	//cout << "������ ���� & �ܾ��������� �����Ϸ�!!!" << endl; 
	

	string tmp[4];
	word tmpword;
	// �ܾ����� �ҷ����� - �Ŀ��� ���ֵ���
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
		}
	}
	file.close(); // term.dat CLOSE

	// �ð� ������ ���ؼ�
	std::chrono::time_point<std::chrono::system_clock> tstart, tend;
	tstart = std::chrono::system_clock::now();

	topic topic25;
	state = false;
	state2 = false;
	stringstream st;
	map<int, map<string, int>> query; // Query -> stemming -> TF ���

	// Query ���� 
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
				topic_Stem(topic25, query); // query�� ���� stemming, trimming, TF��� ����
				topic25.num = " ";
				topic25.desc = " ";
				topic25.title = " ";
				topic25.narr = " ";
			}
		}
	}
	file.close(); // topic25 CLOSE
	stopwordlist.clear(); // stopwordlist CLEAR
	irrlist.clear(); // �ұ�Ģ���� CLEAR

	vector<pair<int, double>> ranking;
	vector<string> docname;
	ofstream out;

	//out.open("Result_VSM.txt", ios::ate);
	//for (auto x : query) {
	//	// 1�� ���� ������ qword,qtf �ѱ�
	//	ranking = cscal(x.second);
	//	return_RelDoc(x.first , ranking, out);
	//	ranking.clear();
	//}
	//out.close();

	// Dirichlet Smoothing
	out.open("Result_LM_Dirichlet.txt", ios::ate);
	for (auto x : query) {
		// LM - ������ �Լ� ȣ��
		ranking = lmcal(x.second);
		return_RelDoc(x.first, ranking, out);
		ranking.clear();
	}
	out.close();

	//�ð�����
	tend = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = tend - tstart;
	std::time_t end_time = std::chrono::system_clock::to_time_t(tend);
	std::cout << "����ð� :" << std::ctime(&end_time)
		<< "�� �ҿ�ð�: " << elapsed_seconds.count() << "��" << endl;
	exit(0);
}

// relevant document�� �̸��� return
void return_RelDoc(int Qnum, vector<pair<int, double>> cs, ofstream& of) {
	int offset = 6 + 16 + 4;
	string line = "";
	string line2 = "";
	ifstream f;
	f.open("doc.dat");
	int i = 0;
	for (auto x : cs) {
		f.seekg((x.first-1)*(offset+2), ios::beg);
		getline(f, line);
		line2 = line.substr(6, 16);
		if (line2.at(0) == 'Y' || line2.at(0) == 'P') {
			line2 = line.substr(5, 16);
			of << Qnum << "	" << line2 << endl;
		}
		else
			of << Qnum << "	" << line2 << endl;
		i++;
		if (i == 10) break; // ���� 10����
	}
	f.close();
}

int return_doclen(int id) {
	int offset = 6 + 16 + 4;
	string line = "";
	ifstream f;
	f.open("doc.dat");
	f.seekg((unsigned long long)((id - 1)*(offset + 2)), ios::beg);
	getline(f, line);
	f.close();
	if (line.size() != 28) {
		return stoi(line.substr(22, 4));
	}
	else 
		return stoi(line.substr(23, 4)); // ���� ���� return
}

// 1���� query ������ �������� ����
vector<pair<int,double>> lmcal(map<string,int> query) {
	int offset = 6 + 6 + 3 + 7;
	vector<pair<int, double>> result;
	map<string, word>::iterator it; // wordlist iterator
	map<int, double>::iterator it2; // lm iterator
	ifstream f;
	string line = "";
	int doc_id=0;
	int mu = 1000;
	int D = 0;
	double tmp = 0;
	map<int, double> lm;

	f.open("index.dat");
	for (auto x : query) {
		cout << "���� query�ܾ�: " + x.first << endl;
		if ((it = wordlist.find(x.first)) != wordlist.end()) {
			f.seekg((unsigned long long)(it->second.start*(offset + 2)), ios::beg);
			for (int i = 0; i < it->second.DF; i++) {
				if (it->second.CF > 5000) break;
				getline(f, line);
				doc_id = stoi(line.substr(6, 6)); // relevant doc_id
				D = return_doclen(doc_id); // �ش� ������ ���� |D|
				tmp = mu*it->second.CF;
				if ((it2=lm.find(doc_id)) != lm.end()) {
					it2->second += log((double)((x.second + (tmp/pow(10,9))) / (D + mu)));
				}
				else
					lm.insert(make_pair(doc_id, log((double)((x.second + (tmp / pow(10, 9))) / (D + mu)))));
			}
		}
	}
	f.close();

	for (auto y : lm) {
		result.push_back(make_pair(y.first, y.second));
	}
	lm.clear();
	sort(result.begin(), result.end(), pair_sort);
	return result;
}

// Query & document Cosine Similarity ���
vector<pair<int, double>> cscal(map<string, int> query) {
	int offset = 6 + 6 + 3 + 7;
	ifstream f;
	map<string, word>::iterator it; // wordlist iterator
	map<int, pair<double, double>>::iterator it2; // cs iterator
	vector<pair<int, double>> result; // doc_id, similarirty ��
	string line = "";
	int doc_id = 0;;
	double weight = 0;

	map<int, pair<double, double>> cs;
	
	f.open("index.dat");
	for (auto y : query) {
		if ((it = wordlist.find(y.first)) != wordlist.end()) {
			f.seekg((unsigned long long)(it->second.start*(offset + 2)), ios::beg);
			for (int i = 0; i < it->second.DF; i++) {
				getline(f, line);
				doc_id = stoi(line.substr(6, 6));
				weight = stod(line.substr(15, 7));
				if ((it2 = cs.find(doc_id)) != cs.end()) {
					it2->second.first += (weight*y.second);
					it2->second.second += pow(weight,2);
				}
				else {
					cs.insert(make_pair(doc_id, make_pair((weight*y.second), pow(weight, 2))));
				}
			}
		}
	} // cosine similarity ��꿡 �ʿ��� ����,�и� ���ϱ� ��
	f.close();

	for (auto x : cs) {
		result.push_back(make_pair(x.first, x.second.first / sqrt(x.second.second)));
	}
	sort(result.begin(), result.end(), pair_sort); // cosine similarity ���� ������ ����
	cs.clear();

	return result;
}

// stopword�� �ҷ��ͼ� unordered_set�� ������
unordered_set<string> storeStopwd() {
	ifstream file("stopword.txt");
	string word;
	unordered_set<string> list;
	if (file.is_open()) {
		while (getline(file, word)) {
			list.insert(word);
		}
	} else cout << "stopword file�� ���� ���߽��ϴ�." << endl; 
	file.close();
	return list;
}

// �ұ�Ģ ���縦 �ҷ��ͼ� unodered_map�� ������
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
				// ù��° �ܾ value
				// �μ���° �ܾ key
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
	// headline �Ǵ� text�� ������ �� stemming ó��
	stringstream tmp(t);
	string index,result;
	word w;
	unordered_map<string, int> TF;
	unordered_map<string, string>::const_iterator it;
	unordered_map<string, int>::iterator it2;
	map<string, word>::iterator it3;

	document_length = 0;
	// word ������ �ɰ����� ����
	while (getline(tmp, index, ' ')) {
		if (index.find("http") != string::npos) index = "";
		if (index.find("www") != string::npos) index = "";
		if (index.find("'") != string::npos) index = "";
		if (index.find("-") != string::npos) index = " ";
		
		// trim 
		Porter2Stemmer::trim(index);

		// Remove Stopword
		if (stopwordlist.find(index) != stopwordlist.end()) index = "";

		// �ҿ������� ó��
		if ((it = irrlist.find(index)) != irrlist.end()) index = it->second;

		// Stemming 
		Porter2Stemmer::stem(index);

		// Remove Stopword
		if (stopwordlist.find(index) != stopwordlist.end()) index = "";

		if (index != "" && index.at(0) == index.at(1)) index = "";

		// ��������
		if (index == "") result += index;
		else {
			result += " " + index;
			document_length++;	// ���� ���� ����
		}

		// �ӽ� unordered_map TF�� �ӽ÷� �����ϸ鼭 wordlist���� �ܾ ����
		if ((it2 = TF.find(index)) != TF.end()) {
				it2->second += 1; // TF ���
		} else {
			if (!(index == "")) {
				TF.insert(make_pair(index, 1));
			}
		}
	} // end while(getline)

	// unordered_map TF ���� �̹� �������� ������ �ܾ��� TF����� ���� ������
	for (auto x : TF) {
		// ����id, �ܾ�, TF �� TF.dat�� ����
		f << doc_count << "	" << x.first << "	" << x.second << endl;		
		// ���� list�� insert
		if ((it3 = wordlist.find(x.first)) != wordlist.end()) {
			it3->second.DF += 1;
			it3->second.CF += x.second;
		} else {
			w.DF = 1;
			w.CF = x.second;
			wordlist.insert(make_pair(x.first, w));
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
			if ((it = irrlist.find(line)) != irrlist.end()) line = it->second; // �ҿ������� ó��
			Porter2Stemmer::stem(line); // Stemming 
			if (stopwordlist.find(line) != stopwordlist.end()) line = ""; // Remove Stopword

			if ((it2 = tmp.find(line)) != tmp.end()) {
				if (i == 2) it2->second += 10; // title�̸� tf������ 10��
				it2->second += 1; // TF���
			}
			else {
				if(!(line=="") && !(line==" "))
					tmp.insert(make_pair(line, 1));
			}
		}
	}
	list.insert(make_pair(stoi(t.num), tmp));
}

// ���������� �ۼ��� �ʿ��� ���ξ�ID,����ID,TF�� ���ϴ� �ڸ����� ǥ��
string format_digit(int digit, int num) { 
	string result = "";
	for (int i = 0; i < digit; ++i) {
		result = result + (to_string(num % 10));
		num /= 10;
	}
	reverse(result.begin(), result.end());
	return result;
}

// weight �ڸ��� �����ִ� �Լ�
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
// ���丮�� ��� ���� ��θ� �������� �Լ�
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