#pragma once
#include <Windows.h>
#include <fstream>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <vector>
#include <string>
#pragma comment(lib, "User32.lib")

using namespace std;
namespace utility {
	void get_file_paths(LPCWSTR current, vector<string>& paths);
}