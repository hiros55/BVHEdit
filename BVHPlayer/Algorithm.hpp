#pragma once
#include<string>
#include<vector>

class Algorithm {
public:
	std::string  split(std::string &str, const std::string &separater, bool endindex, int plusindex);
	std::vector<std::string> split(std::string &str,const std::string &separater);
	std::string split(std::string &str, const char &separater);
};