#pragma once
#include<string>
#include<vector>

class Bvhedit {
public:
	bool DeleteLine(std::string csvPath, std::string exportPath, std::vector<int> frameIndex, unsigned int framenum);
	bool AddLine(std::string csvPath, std::string exportPath, unsigned int numFrames, int firstFrameNum, unsigned int insertFrameNum, unsigned int channelNum, double* motionData);
};