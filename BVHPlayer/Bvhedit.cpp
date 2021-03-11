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

	//総フレーム数調節
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

	//関節情報読み込み
	while (std::getline(csv, line)) {
		//フレーム数書き換え
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

	//モーションデータ読み込み
	while (std::getline(csv, line)) {
		//モーションデータ編集部分(削除)
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

	//csv書き出し
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

	//総フレーム数調節
	int editFramenum = 0;
	editFramenum = insertFrameNum + numFrames;
	std::string editFramenumstr = std::to_string(editFramenum);

	//関節情報読み込み
	std::string line;
	std::string csvFileBuffer;

	while (std::getline(csv, line)) {
		//フレーム数書き換え
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
	//モーションデータ読み込み
	while (std::getline(csv, line)) {
		//編集部分(追加)
		if (lineIndex == firstFrameNum) {
			csvMotionBuffer += line;
			csvMotionBuffer += '\n';
			for (int j = 0; j < channelNum; j++)
			{
				//選択されたフレームの角度情報
				double currentFrameNum = motionData[lineIndex * channelNum + j];
				//選択されたフレームの角度情報と1フレーム分大きな角度情報の差分をとる
				double sub = motionData[(lineIndex + 1) * channelNum + j] - motionData[lineIndex * channelNum + j];
				//挿入フレーム分で角度情報の差分を割って、角度情報を保存
				double tmp = sub / (insertFrameNum + 1);
				for (int i = 0; i < insertFrameNum; i++) {
					currentFrameNum += tmp;
					insertBuffer[i + (j * insertFrameNum)] = currentFrameNum;
				}
			}
			//設定した角度情報を挿入する
			for (int i = 0; i < insertFrameNum; i++) {
				for (int j = i; j < channelNum * insertFrameNum; j += insertFrameNum) {
					std::stringstream stream;
					stream << std::fixed << std::setprecision(4) << insertBuffer[j];
					csvMotionBuffer += stream.str();
					csvMotionBuffer += ' ';
				}
				//最後のスペースのみ削除
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
	//csv書き出し
	std::ofstream ofsCsv("./" + exportPath + ".bvh");
	ofsCsv << csvFileBuffer;
	ofsCsv << csvMotionBuffer;

	csv.close();
	ofsCsv.close();
	return true;

}