#pragma once
#include<vector>
constexpr int MaxMotionData = 8600;
constexpr int MotionIndex = 33;
class ReadMotionData {
private:
	//���W�擾���s�C���f�b�N�X�z��
	std::vector<std::vector<int>> getFailedLineIndex;
	//���W�擾���s�⊮�֐�
	bool MotionEdit();

	int FirstMotionSearch(int currentFrame, int failedFrameIndex);
	int EndMotionSearch(int currentFrame, int failedFrameIndex);
public:
	int csvFrameNum;
	const int jointNum = 17;
	const int axisNum = 3;
	//�`��p���_�f�[�^
	float motiondata[MaxMotionData];
	//�`��p���_�C���f�b�N�X
	int motiondataIndex[MotionIndex] = {0,1,2,3,2,1,0,4,5,6,5,4,0,7,8,11,12,13,12,11,8,14,15,16,15,14,8,9,10,9,8,7,0};
	//�f�[�^�ǂݍ��݊֐�
	bool ReadMotionFile(std::string csvPath);
	
};