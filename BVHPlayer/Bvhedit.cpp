#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include"Bvhedit.hpp"
#include"Algorithm.hpp"
Algorithm algorithma;

/*
		std::vector<std::string> strList;
		strList = algorithma.split(line, " ");
		strList[3] = "0.0000";strList[4] = "0.0000";strList[5] = "0.0000";
		std::cout << lineIndex << std::endl;
		std::cout <<std::endl;
		for (int i = 0; i < strList.size();i++) {
			csvMotionBuffer += strList[i];
			csvMotionBuffer += " ";
		}
		csvMotionBuffer.pop_back();
		csvMotionBuffer += '\n';
		lineIndex++;*/

bool Bvhedit::DeleteLine(std::string csvPath, std::string exportPath, std::vector<int> frameIndex, unsigned int framenum) {
	std::ifstream csv;
	csv.open(csvPath);
	if (csv.is_open() == 0) {
		std::cout << "Could not open file" << std::endl;
		return false;
	}

	//���t���[��������
	int editFramenum = 0;
	if (frameIndex.size() > 0) {
		editFramenum = framenum - frameIndex.size();
	}
	else {
		editFramenum = framenum;
	}

	std::string editFramenumstr = std::to_string(editFramenum);


	std::string line;
	std::string csvFileBuffer;

	//�֐ߏ��ǂݍ���
	while (std::getline(csv, line)) {
		//�t���[������������
		if (line.find("Frames") != std::string::npos) {
			csvFileBuffer += "Frames: ";
			csvFileBuffer += editFramenumstr;
			csvFileBuffer += "\n";
		}
		else {
			csvFileBuffer += line;
			csvFileBuffer += "\n";
		}

		if (line.find("Frame Time") != std::string::npos) {
			break;
		}
	}

	std::string csvMotionBuffer;
	int  vectorIndex = 0;
	int  lineIndex = 0;

	//���[�V�����f�[�^�ǂݍ���
	while (std::getline(csv, line)) {
		//���[�V�����f�[�^�ҏW����(�폜)
		if (!frameIndex.empty() && frameIndex[vectorIndex] == lineIndex) {
			int& tmp = frameIndex.back();
			if (frameIndex[vectorIndex] != tmp) {
				vectorIndex++;
			}
		}
		else {
			csvMotionBuffer += line;
			csvMotionBuffer += '\n';
		}
		lineIndex++;
	}

	//csv�����o��
	std::ofstream ofsCsv("./" + exportPath + ".bvh");
	ofsCsv << csvFileBuffer;
	ofsCsv << csvMotionBuffer;

	csv.close();
	ofsCsv.close();
	return true;
}

bool Bvhedit::AddLine(std::string csvPath, std::string exportPath, unsigned int numFrames, int firstFrameNum, unsigned int insertFrameNum, unsigned int channelNum, double* motionData) {

	if (insertFrameNum == 0) {
		std::cout << "insertFrameNum is 0" << std::endl;
		return false;
	}

	std::ifstream csv;
	csv.open(csvPath);
	if (csv.is_open() == 0) {
		std::cout << "Could not open file" << std::endl;
		return false;
	}

	//���t���[��������
	int editFramenum = 0;
	editFramenum = insertFrameNum + numFrames;
	std::string editFramenumstr = std::to_string(editFramenum);

	//�֐ߏ��ǂݍ���
	std::string line;
	std::string csvFileBuffer;

	while (std::getline(csv, line)) {
		//�t���[������������
		if (line.find("Frames") != std::string::npos) {
			csvFileBuffer += "Frames: ";
			csvFileBuffer += editFramenumstr;
			csvFileBuffer += "\n";
		}
		else {
			csvFileBuffer += line;
			csvFileBuffer += "\n";
		}

		if (line.find("Frame Time") != std::string::npos) {
			break;
		}
	}


	std::string csvMotionBuffer;
	int  lineIndex = 0;
	double* insertBuffer = new double[insertFrameNum * channelNum];
	//���[�V�����f�[�^�ǂݍ���
	while (std::getline(csv, line)) {
		//�ҏW����(�ǉ�)
		if (lineIndex == firstFrameNum) {
			csvMotionBuffer += line;
			csvMotionBuffer += '\n';
			for (int j = 0; j < channelNum; j++)
			{
				//�I�����ꂽ�t���[���̊p�x���
				double currentFrameNum = motionData[lineIndex * channelNum + j];
				//�I�����ꂽ�t���[���̊p�x����1�t���[�����傫�Ȋp�x���̍������Ƃ�
				double sub = motionData[(lineIndex + 1) * channelNum + j] - motionData[lineIndex * channelNum + j];
				//�}���t���[�����Ŋp�x���̍����������āA�p�x����ۑ�
				double tmp = sub / (insertFrameNum + 1);
				for (int i = 0; i < insertFrameNum; i++) {
					currentFrameNum += tmp;
					insertBuffer[i + (j * insertFrameNum)] = currentFrameNum;
				}
			}
			//�ݒ肵���p�x����}������
			for (int i = 0; i < insertFrameNum; i++) {
				for (int j = i; j < channelNum * insertFrameNum; j += insertFrameNum) {
					std::stringstream stream;
					stream << std::fixed << std::setprecision(4) << insertBuffer[j];
					csvMotionBuffer += stream.str();
					csvMotionBuffer += ' ';
				}
				//�Ō�̃X�y�[�X�̂ݍ폜
				csvMotionBuffer.pop_back();

				csvMotionBuffer += '\n';
			}
		}
		else {
			csvMotionBuffer += line;
			csvMotionBuffer += '\n';
		}
		lineIndex++;
	}
	//csv�����o��
	std::ofstream ofsCsv("./" + exportPath + ".bvh");
	ofsCsv << csvFileBuffer;
	ofsCsv << csvMotionBuffer;

	csv.close();
	ofsCsv.close();
	return true;

}