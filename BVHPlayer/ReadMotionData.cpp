#include <fstream>
#include <string>
#include <iostream>
#include<vector>
#include"ReadMotionData.hpp"
#include"Algorithm.hpp"
#include"glut.h"
#include <windows.h>
Algorithm algorithm;

const int csvLineNum = 18;
const int csvScale = 50;

bool ReadMotionData::ReadMotionFile(std::string csvPath) {
	std::ifstream csv;
	csv.open(csvPath);

	//�t�@�C��open�m�F
	if (csv.is_open() == 0) {
		std::cout << "Could not open file" <<std::endl;
		csv.close();
		return false;
	}


	std::vector<std::string> buffer;
	std::vector<int> getFailedIndex;
	std::string line;
	int lineIndex = 0;
	//���W�擾���svector�C���f�b�N�X
	int getFailedIndexIndex = 0;
	//���[�V�����f�[�^�̍s�C���f�b�N�X
	int motionDataLineIndex = 0;

	//�f�[�^�ǂݍ��ݕ���
	while (std::getline(csv, line)) {
		if (line.compare(" ") == 0) {
			continue;
		}else {
			//�J���}��csv�f�[�^�𕪊�
			buffer = algorithm.split(line, ",");

			//������bool�l�̏ꍇ
			if (lineIndex % csvLineNum == 0 ) {
				for (int i = 0; i < buffer.size();i++) {
					if (buffer[i].compare("True") == 0) {
						continue;
					}
					else {
						getFailedIndex.push_back(i);
					}
				}
				getFailedLineIndex.push_back(getFailedIndex);
				getFailedIndex.clear();
				getFailedIndexIndex++;
			}//������float�l�̏ꍇ
			else {
				for (int i = 0; i < 3; i++) {
					motiondata[(motionDataLineIndex) * 3 + i] = stod(buffer[i]);
					//�T�C�Y����
					motiondata[(motionDataLineIndex) * 3 + i] /= csvScale;
				}
				motionDataLineIndex++;
			}
			buffer.clear();
			lineIndex++;
		}
	}
	//�擾�t���[����
	csvFrameNum =(lineIndex / csvLineNum);

	//�擾���s���W��Ԋ֐�
	MotionEdit();

	csv.close();
	return true;
}

bool ReadMotionData::MotionEdit() {
	for (int i = 0;i < getFailedLineIndex.size();i++) {
		if (getFailedLineIndex[i].size() <= 0) {
			continue;
		}
		else {
			for (int k = 0; k < getFailedLineIndex[i].size();k++) {
				for (int n = 0; n < axisNum ;n++) {
					motiondata[(i * jointNum * axisNum) + (axisNum * getFailedLineIndex[i][k]) + n] =
						((motiondata[(EndMotionSearch(i, k) * jointNum * axisNum) + (axisNum * getFailedLineIndex[i][k]) + n] - motiondata[(FirstMotionSearch(i, k) * jointNum * axisNum) + (axisNum * getFailedLineIndex[i][k]) + n] )/2) + motiondata[(FirstMotionSearch(i, k) * jointNum * axisNum) + (axisNum * getFailedLineIndex[i][k]) + n];
					std::cout << motiondata[(i * jointNum * axisNum) + (axisNum * getFailedLineIndex[i][k]) + n] <<std::endl;
					
				}
			}
		}
	}
	return true;
}

int ReadMotionData::FirstMotionSearch(int currentFrame,int failedFrameIndex) {
	int cFrame = currentFrame, fFrame = failedFrameIndex;
	//���݂̃C���f�b�N�X����k���āA���W���擾�o���Ă���ő�̃C���f�b�N�X��T��
	for (int j = 0; j < getFailedLineIndex[cFrame - 1].size();j++) {

		//��O�̃t���[���ł����W���擾�ł��Ă��Ȃ������ꍇ���ċA
		if (getFailedLineIndex[cFrame][fFrame] == getFailedLineIndex[cFrame - 1][j]) {
			//vector�v�f�X�V
			cFrame -= 1;fFrame = j; 
			//for�����Z�b�g
			j = -1;
		}
		else {
			continue;
		}
	}
	return cFrame - 1;
}

int ReadMotionData::EndMotionSearch(int currentFrame, int failedFrameIndex) {
	int cFrame = currentFrame, fFrame = failedFrameIndex;
	//���݂̃C���f�b�N�X���牺���āA���W���擾�o���Ă���ŏ��̃C���f�b�N�X��T��
	for (int j = 0; j < getFailedLineIndex[cFrame + 1].size();j++) {

		//���̃t���[���ł����W���擾�ł��Ă��Ȃ������ꍇ
		if (getFailedLineIndex[cFrame][fFrame] == getFailedLineIndex[cFrame + 1][j]) {
			//vector�v�f�X�V
			cFrame += 1;fFrame = j;
			//for�����Z�b�g
			j = -1;
		}
		else {
			continue;
		}
	}
	return cFrame + 1;
}
