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

	//ファイルopen確認
	if (csv.is_open() == 0) {
		std::cout << "Could not open file" <<std::endl;
		csv.close();
		return false;
	}


	std::vector<std::string> buffer;
	std::vector<int> getFailedIndex;
	std::string line;
	int lineIndex = 0;
	//座標取得失敗vectorインデックス
	int getFailedIndexIndex = 0;
	//モーションデータの行インデックス
	int motionDataLineIndex = 0;

	//データ読み込み部分
	while (std::getline(csv, line)) {
		if (line.compare(" ") == 0) {
			continue;
		}else {
			//カンマでcsvデータを分割
			buffer = algorithm.split(line, ",");

			//分割がbool値の場合
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
			}//分割がfloat値の場合
			else {
				for (int i = 0; i < 3; i++) {
					motiondata[(motionDataLineIndex) * 3 + i] = stod(buffer[i]);
					//サイズ調節
					motiondata[(motionDataLineIndex) * 3 + i] /= csvScale;
				}
				motionDataLineIndex++;
			}
			buffer.clear();
			lineIndex++;
		}
	}
	//取得フレーム数
	csvFrameNum =(lineIndex / csvLineNum);

	//取得失敗座標補間関数
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
	//現在のインデックスから遡って、座標が取得出来ている最大のインデックスを探索
	for (int j = 0; j < getFailedLineIndex[cFrame - 1].size();j++) {

		//一つ前のフレームでも座標を取得できていなかった場合が再帰
		if (getFailedLineIndex[cFrame][fFrame] == getFailedLineIndex[cFrame - 1][j]) {
			//vector要素更新
			cFrame -= 1;fFrame = j; 
			//for分リセット
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
	//現在のインデックスから下って、座標が取得出来ている最小のインデックスを探索
	for (int j = 0; j < getFailedLineIndex[cFrame + 1].size();j++) {

		//一つ後のフレームでも座標を取得できていなかった場合
		if (getFailedLineIndex[cFrame][fFrame] == getFailedLineIndex[cFrame + 1][j]) {
			//vector要素更新
			cFrame += 1;fFrame = j;
			//for分リセット
			j = -1;
		}
		else {
			continue;
		}
	}
	return cFrame + 1;
}
