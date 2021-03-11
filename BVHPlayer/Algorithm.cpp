#include<string>
#include<vector>
#include"Algorithm.hpp"
#include<iostream>

//選択したインデックスから初めに指定した要素を見つける。
//見つけたインデックスをendindexにするかfirstindexにするかを選択し分割
std::string Algorithm::split(std::string &str, const std::string &separater, bool endindex, int plusindex) {

	if (separater.empty() || plusindex > str.length()) {
		return str;
	}
	//選択したインデックス分文字列を分割する
	str = str.substr(plusindex);

	int i = str.find(separater);
	if (i == std::string::npos) {
		return str;
	}

	//どのように分割するか
	if (endindex) {
		str = str.substr(0, i);
	}
	else {
		str = str.substr(i, str.length() - i);
	}
	return str;
}


//常に決まったseparator文字列によるstr文字列複数分割関数(vectorへの格納)
std::vector<std::string> Algorithm::split(std::string &str, const std::string &separater) {

	std::vector<std::string> stringList;

	if (separater.length() <= 0) {
		stringList.push_back(str);
		return stringList;
	}
	else {
		std::string buf;
		int FindIndex = 0;
		int offset = 0;
		while (1) {
			FindIndex = str.find(separater, offset);
			if (FindIndex == std::string::npos) {
				stringList.push_back(str.substr(offset));
				break;
			}
			stringList.push_back(str.substr(offset, FindIndex - offset));
			offset = FindIndex + separater.length();
		}

		return stringList;
	}
}

//連続するchar型のsepatater文字群を切り取る
std::string Algorithm::split(std::string &str, const char &separater) {
	while (separater == str[0]) {
		str = str.substr(1);
	}
	return str;
}