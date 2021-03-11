#pragma once
#include<vector>
constexpr int MaxMotionData = 8600;
constexpr int MotionIndex = 33;
class ReadMotionData {
private:
	//座標取得失敗インデックス配列
	std::vector<std::vector<int>> getFailedLineIndex;
	//座標取得失敗補完関数
	bool MotionEdit();

	int FirstMotionSearch(int currentFrame, int failedFrameIndex);
	int EndMotionSearch(int currentFrame, int failedFrameIndex);
public:
	int csvFrameNum;
	const int jointNum = 17;
	const int axisNum = 3;
	//描画用頂点データ
	float motiondata[MaxMotionData];
	//描画用頂点インデックス
	int motiondataIndex[MotionIndex] = {0,1,2,3,2,1,0,4,5,6,5,4,0,7,8,11,12,13,12,11,8,14,15,16,15,14,8,9,10,9,8,7,0};
	//データ読み込み関数
	bool ReadMotionFile(std::string csvPath);
	
};