#include<string>
#include<vector>
#include"Algorithm.hpp"
#include<iostream>

//�I�������C���f�b�N�X���珉�߂Ɏw�肵���v�f��������B
//�������C���f�b�N�X��endindex�ɂ��邩firstindex�ɂ��邩��I��������
std::string Algorithm::split(std::string &str, const std::string &separater, bool endindex, int plusindex) {

	if (separater.empty() || plusindex > str.length()) {
		return str;
	}
	//�I�������C���f�b�N�X��������𕪊�����
	str = str.substr(plusindex);

	int i = str.find(separater);
	if (i == std::string::npos) {
		return str;
	}

	//�ǂ̂悤�ɕ������邩
	if (endindex) {
		str = str.substr(0, i);
	}
	else {
		str = str.substr(i, str.length() - i);
	}
	return str;
}


//��Ɍ��܂���separator������ɂ��str�����񕡐������֐�(vector�ւ̊i�[)
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

//�A������char�^��sepatater�����Q��؂���
std::string Algorithm::split(std::string &str, const char &separater) {
	while (separater == str[0]) {
		str = str.substr(1);
	}
	return str;
}