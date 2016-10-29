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

//// �ð�������ؼ�
//#include <chrono>
//#include <ctime>

using namespace std;

struct word {
	int index_no; // ���ξ� id
	int DF;
	int CF;
	int start;
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
bool compare(const iindex& i1, const iindex& i2) {
	if (i1.index_id != i2.index_id)
		return i1.index_id < i2.index_id;
	else
		return i1.doc_id < i2.doc_id;
}

unordered_map<string, string> sirregular(); // irregularverbs.txt�� �ҷ��ͼ� map�� ����
unordered_set<string> storeStopwd(); // stopword.txt�� �ҷ��ͼ� set�� ����
string stem(string t, int doc_count, ofstream& f); // stemming ������ ó�����ִ� �Լ�
string topic_Stem(string s); // topic stemming
string format_digit(int num_digit, int num); // �ڸ��� ���ߴ� �Լ�
string format_weight(double weight); // ���������� ������ �����ִ� �Լ�
void utility::get_file_paths(LPCWSTR current, vector<string>& paths); // data ��θ� �����ϴ� �Լ�
void return_index(int start);

// �������� ����
int index_id = 1;
int doc_size = 0;
int document_length = 0;
unordered_set<string> stopwordlist = storeStopwd(); // stopword�� unordered set�� ����
unordered_map<string, string> irrlist = sirregular(); // �ұ�Ģ���� unordered map�� ����
map<string, word> wordlist; // ��ü �ܾ� ����

void main()
{
	//// �ð� ������ ���ؼ�
	//std::chrono::time_point<std::chrono::system_clock> tstart, tend;
	//tstart = std::chrono::system_clock::now();

	ifstream file;
	//ofstream outTF("TF.dat", ios::ate); // TF ����
	//ofstream outDoc("doc.dat",ios::ate); // ���� ���� ����
	vector<string> paths;
	article article;
	document document;
	string line,line2;
	int i = 0;
	int Tagsize = 0;
	int begin = 0;
	int end = 0;
	int doc_count = 1;
	bool state = false;
	bool state1 = false;
	bool state2 = false;
	double dnom = 0;

	// ���� path ����
	utility::get_file_paths(TEXT(".\\ir_corpus"), paths);

	/*
	for (i = 0; i < paths.size(); i++) {
		cout << "File Open :" << paths[i];
		file.open(paths[i]);
		if (file.is_open()) {
			while (!file.eof()) {
				getline(file, line);
				// DOCNO Parsing
				if (line.find("</DOCNO>") != string::npos) state = false;
				if (state) article.docno += line + " ";
				if ((begin = line.find("<DOCNO>")) != string::npos) {
					if ((end = line.find("</DOCNO>")) != string::npos) {
						Tagsize = string("<DOCNO>").size();
						article.docno += line.substr(begin + Tagsize, end - Tagsize);
						state = false;
					}
					else state = true;
				}

				// Headline Parsing
				if (line.find("</HEADLINE>") != string::npos) state1 = false;
				if (state1) article.headline += line + " ";
				if ((begin = line.find("<HEADLINE>")) != string::npos) {
					if ((end = line.find("</HEADLINE>")) != string::npos) {
						Tagsize = string("<HEADLINE>").size();
						article.headline += line.substr(begin + Tagsize, end - Tagsize);
						state1 = false;
					}
					else state1 = true;
				}

				// Text Parsing
				if (line.find("</TEXT>") != string::npos) state2 = false;
				if (state2) article.text += line + " ";
				if ((begin = line.find("<TEXT>")) != string::npos) {
					Tagsize = string("<TEXT>").size();
					if ((end = line.find("</TEXT>")) != string::npos) {
						article.text += line.substr(begin + Tagsize, end - Tagsize);
						state2 = false;
					}
					else {
						article.text += line.substr(begin + Tagsize, line.size());
						state2 = true;
					}
				}

				// /doc�� ã���� 1�� parsing ��
				if (line.find("</DOC>") != string::npos) {

					// stemming
					article.headline = stem(article.headline, doc_count, outTF);
					article.text = stem(article.text, doc_count, outTF);

					// <���� ���� ����> -> doc.dat
					document.ID = doc_count++;
					document.name = article.docno;
					document.len = document_length;
					outDoc << document.ID << "	" << document.name << "	" << document.len << endl;
				
					article.docno = "";
					article.headline = "";
					article.text = "";
				}
			}// end while
		}// end if 
		file.close();
		cout << "	ó���Ϸ�" << endl;
	}// for Loop
	
	outTF.close(); // TF ���� CLOSE
	outDoc.close(); // ������������ CLOSE

	doc_size = doc_count; // ���� �� ����
	cout << "�� ���� ���� :" << doc_size << endl;
	cout << "�� �ܾ� ���� :" << wordlist.size() << endl;
	cout << "������ ���� & �ܾ��������� ������..." << endl;


	i = 0; // ������ġ counting
	int j = 1;
	int doc_id = 1;
	int id, tf;
	string sword;
	unordered_map<string, int> TF;
	map<string, word>::const_iterator it;
	iindex itmp;
	vector<iindex> iindexlist; // ������ ���Ͽ� �� ������ �ӽ÷� ����

	char data[100];
	string result = "";

	ofstream outTerm("term.dat", ios::ate); // �ܾ� ���� ����
	for (auto& x : wordlist) {
		x.second.index_no = index_id++; // index id�� �ο���
		// �ܾ��������� -> term.dat ���� <���ξ�ID/���ξ�/DF/CF/������ġ>
		result = to_string(x.second.index_no) + "	" + x.first + "	" + to_string(x.second.DF) + "	" + to_string(x.second.CF) + "	" + to_string(i) + "\n";
		memcpy(data,result.c_str(),sizeof(string)*2);
		outTerm.write(data,result.size());
		i = i + x.second.DF;//������ġ count
	}
	outTerm.close(); // �ܾ��������� CLOSE

	file.open("TF.dat");
	//FILE *rfile;
	//rfile = fopen("TF.dat", "rm");
	//setvbuf(rfile, NULL, _IOLBF, 1024);
	while (file >> id) {
		if (j != id) {
			for (auto y : TF) {
				dnom += pow(log10((y.second) + 1.0)*(log10(doc_size / wordlist.find(y.first)->second.DF)), 2); // �и���
			}
			for (auto x : TF) {
				it = wordlist.find(x.first);
				itmp.doc_id = j;
				itmp.index_id = it->second.index_no;
				itmp.tf = x.second;
				itmp.weight = (1 + log10(x.second))*log10(doc_size / it->second.DF) / sqrt(dnom);
				iindexlist.push_back(itmp); // ������ ���� ������ ���ؼ� �ϴ� vector�� ����
			}
			TF.clear();
			dnom = 0;
			j++;
		}
		file >> sword;
		file >> tf;
		TF.insert(make_pair(sword, tf));
	}
	file.close(); // TF.dat ���� CLOSE

	sort(iindexlist.begin(),iindexlist.end(),compare); // ������ ���� ����

	ofstream outIndex("index.dat", ios::ate); // ������ ����
	for (i = 0; i < iindexlist.size(); i++) {
		// ������ ���ϸ���� index.dat ���� <���ξ�id/����id/TF/weight>
		result = format_digit(6, iindexlist[i].index_id) + format_digit(6, iindexlist[i].doc_id) + format_digit(3, iindexlist[i].tf) + format_weight(iindexlist[i].weight) + "\n";
		memcpy(data, result.c_str(), sizeof(string));
		outIndex.write(data,23);
	}
	iindexlist.clear();
	outIndex.close(); // ���������� CLOSE
	cout << "������ ���� & �ܾ��������� �����Ϸ�!!!" << endl; 
	*/
	
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

	topic topic25;
	vector<topic> topiclist;
	state = false;
	state2 = false;

	// Query ���� 
	file.open("topics25.txt");
	if (file.is_open()) {
		while (getline(file, line)) {
			if (line == "") break;
			if (line.find("<num>") != string::npos) {
				line2 = line.substr(line.size() - 4, 4);
				line2.erase(remove(line2.begin(), line2.end(), ' '), line2.end());
				topic25.num = line2;
			}
			if ((begin=line.find("<title>")) != string::npos) {
				line2 = line.substr(begin + 7, line.size() - 7);
				topic25.title = topic_Stem(line2);
			} 

			if (line.find("<narr>") != string::npos) state = false;
			if (state) topic25.desc += line + " ";
			if (line.find("<desc>") != string::npos) state = true;

			if (line.find("</top>") != string::npos) state2 = false;
			if (state2) topic25.narr += line + " ";
			if (line.find("<narr>") != string::npos) state2 = true; 

			if (line.find("</top>") != string::npos) {
				topic25.desc = topic_Stem(topic25.desc);
				topic25.narr = topic_Stem(topic25.narr);
				topiclist.push_back(topic25);
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

	return_index(10); // ���� ��ġ�� �ѱ�� ������ return
	exit(0);

	////�ð�����
	//tend = std::chrono::system_clock::now();
	//std::chrono::duration<double> elapsed_seconds = tend - tstart;
	//std::time_t end_time = std::chrono::system_clock::to_time_t(tend);
	//std::cout << "����ð� :" << std::ctime(&end_time)
	//	<< "�� �ҿ�ð�: " << elapsed_seconds.count() << "��" << endl;
}


void return_index(int start) {
	int offset = 6 + 6 + 3 + 7;
	string result;
	ifstream f("index.dat");
	
	f.seekg(start*(offset+2), ios::beg);
	getline(f, result);
	cout << result.substr(0,6) << endl; // ���ξ� id
	cout << result.substr(6, 6) << endl; // ���� id
	cout << result.substr(12, 3) << endl; // TF
	cout << result.substr(15, 7) << endl; // ������ġ
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

		// ��������
		if (index == "") result += index;
		else {
			result += " " + index;
			document_length++;	// ���� ���� ����
		}

		// �ӽ� unordered_map TF�� �ӽ÷� �����ϸ鼭 wordlist���� �ܾ ����
		if ((it2 = TF.find(index)) != TF.end() && !(index == "")) {
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
			//w.index_no = index_id++;
			w.DF = 1;
			w.CF = x.second;
			wordlist.insert(make_pair(x.first, w));
		}
	}
	TF.clear();
	return result;
}
string topic_Stem(string s) {
	stringstream ss(s);
	string line,result;
	unordered_map<string, string>::const_iterator it;

	while (getline(ss, line, ' ')) {
		Porter2Stemmer::trim(line); // Trim 
		if (stopwordlist.find(line) != stopwordlist.end()) line = ""; // Remove Stopword
		if ((it = irrlist.find(line)) != irrlist.end()) line = it->second; // �ҿ������� ó��
		Porter2Stemmer::stem(line); // Stemming 

		// ��������
		if (line == "") result += line;
		else {
			result += " " + line;
			document_length++;	// ���� ���� ����
		}
	}
	return result;
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