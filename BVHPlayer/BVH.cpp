#include <fstream>
#include <string.h>
#include<iostream>
#include "glew.h"
#include "BVH.hpp"
#include "Matrix.hpp"
#include"Quaternion.hpp"
#include"Algorithm.hpp"
#include <chrono>
static Algorithm algorithm;

BVH::BVH()
{
	motion = NULL;
	vertexArray = NULL;
	vertexIndex = NULL;
	bvhMatrix = NULL;
	Clear();
}

// �R���g���N�^
BVH::BVH(const char* bvh_file_name)
{
	motion = NULL;
	vertexArray = NULL;
	vertexIndex = NULL;
	bvhMatrix = NULL;
	Clear();
	Load(bvh_file_name);
	vertexArray = new float[joints.size() * 3];
	bvhMatrix = new Matrix[joints.size()];
}



// �f�X�g���N�^
BVH::~BVH()
{
	Clear();
}


// �S���̃N���A
void  BVH::Clear()
{
	for (int i = 0; i < channels.size(); i++)
		delete  channels[i];
	for (int i = 0; i < joints.size(); i++)
		delete  joints[i];


	if (motion != NULL)
		delete[] motion;

	if (vertexArray != NULL)
		delete[] vertexArray;

	if (bvhMatrix != NULL)
		delete[] bvhMatrix;

	if (vertexIndex != NULL)
		delete[] vertexIndex;

	isloadsuccess = false;

	file_name = "";

	num_channel = 0;
	channels.clear();
	joints.clear();
	joint_index.clear();

	vertexIndexSize = 0;

	num_frame = 0;
	interval = 0.0;
	motion = NULL;
}

void  BVH::Load(const char* bvh_file_name)
{

	std::ifstream  file;
	std::string line;
	std::string token;

	std::vector< Joint* >   joint_stack;
	Joint* joint = NULL;
	Joint* new_joint = NULL;
	bool      is_site = false;
	double    x, y, z;
	int       i, j;

	//���_�V�F�[�_�[�̒��_�C���f�b�N�X�ϒ��z��
	std::vector<int> vVertexIndex;

	// ������
	Clear();

	// �t�@�C���̏��i�t�@�C�����E���얼�j�̐ݒ�
	file_name = bvh_file_name;

	// �t�@�C���̃I�[�v��
	file.open(bvh_file_name, std::ios::in);
	if (file.is_open() == 0) { 
		std::cout << "Counld not load file" << std::endl;
		return;
	}

	// �K�w���̓ǂݍ���
	while (!file.eof())
	{
		// �t�@�C���̍Ō�܂ł��Ă��܂�����ُ�I��
		if (file.eof()) {
			std::cout << "Error:File Eof" << std::endl;
			file.close();
		}

		// �P�s�ǂݍ��݁A�擪�̒P����擾
		std::getline(file, line);

		//'\t'�܂œǂݍ���
		char cseparater = '\t';
		token = algorithm.split(line, cseparater);
		//' '�܂œǂݍ���
		std::string sseparater = " ";
		token = algorithm.split(token, sseparater, true, 0);

		// ��s�̏ꍇ�͎��̍s��
		if (token.empty())  continue;

		// �֐߃u���b�N�̊J�n
		if (token.compare("{") == 0)
		{
			// ���݂̊֐߂��X�^�b�N�ɐς�
			joint_stack.push_back(joint);
			joint = new_joint;
			if (!joint->has_site)
				vVertexIndex.push_back(joint->index);
			continue;
		}
		// �֐߃u���b�N�̏I��
		if (token.compare("}") == 0)
		{
			if (!joint->has_site)
				vVertexIndex.push_back(joint->index);

			// ���݂̊֐߂��X�^�b�N������o��
			joint = joint_stack.back();
			joint_stack.pop_back();
			is_site = false;

			continue;
		}

		// �֐ߏ��̊J�n
		if ((token.compare("ROOT") == 0) ||
			(token.compare("JOINT") == 0))
		{
			// �֐߃f�[�^�̍쐬
			new_joint = new Joint();
			new_joint->index = joints.size();
			new_joint->parent = joint;
			new_joint->has_site = false;
			new_joint->offset[0] = 0.0;  new_joint->offset[1] = 0.0;  new_joint->offset[2] = 0.0;
			new_joint->site[0] = 0.0;  new_joint->site[1] = 0.0;  new_joint->site[2] = 0.0;
			joints.push_back(new_joint);
			if (joint)
				joint->children.push_back(new_joint);

			//�C���f�b�N�X�o�^�����_index�z�񐶐�
			std::string separater = " ";
			line = line.substr(line.find(separater) + 1);
			line = algorithm.split(line, separater, true, 0);
			//�֐ߖ��ǂݍ���
			new_joint->name = line;

			// �C���f�b�N�X�֒ǉ�
			joint_index[new_joint->name] = new_joint;
			continue;
		}

		// ���[���̊J�n
		if ((token.compare("End") == 0))
		{
			vVertexIndex.pop_back();
			new_joint = joint;
			is_site = true;
			continue;
		}

		// �֐߂̃I�t�Z�b�g or ���[�ʒu�̏��
		if (token.compare("OFFSET") == 0)
		{
			// ���W�l��ǂݍ���
			std::string separater = " ";
			std::vector<std::string> StringList;
			StringList = algorithm.split(line, separater);
			x = std::stod(StringList[1]);
			y = std::stod(StringList[2]);
			z = std::stod(StringList[3]);

			if (is_site)
			{
				joint->has_site = true;
				joint->site[0] = x;
				joint->site[1] = y;
				joint->site[2] = z;
			}
			else
			{
				joint->offset[0] = x;
				joint->offset[1] = y;
				joint->offset[2] = z;
			}
			continue;
		}

		// �֐߂̃`�����l�����
		if (token.compare("CHANNELS") == 0)
		{
			// �`�����l������ǂݍ���
			//token = strtok(NULL, separater);
			std::string separater = " ";
			//line�������CHANNELS��������폜�������̂ɍX�V
			line = line.substr(line.find(separater));

			//token�����߂邽�߂�tokenline��line�̕������������
			std::string tokenline = line;
			token = algorithm.split(tokenline, separater, true, 1);
			joint->channels.resize(std::stoi(token));

			std::vector<std::string> stringList;
			stringList = algorithm.split(line, separater);

			// �`�����l������ǂݍ��݁i���̃W���C���g�Ƀ`�����l����񂪓���j
			for (i = 0; i < joint->channels.size(); i++)
			{
				// �`�����l���̍쐬
				Channel* channel = new Channel();
				channel->joint = joint;
				channel->index = channels.size();
				channels.push_back(channel);
				joint->channels[i] = channel;

				token = stringList[i + 2];

				// �`�����l���̎�ނ̔���
				if (token.compare("Xrotation") == 0)
					channel->type = X_ROTATION;
				else if (token.compare("Yrotation") == 0)
					channel->type = Y_ROTATION;
				else if (token.compare("Zrotation") == 0)
					channel->type = Z_ROTATION;
				else if (token.compare("Xposition") == 0)
					channel->type = X_POSITION;
				else if (token.compare("Yposition") == 0)
					channel->type = Y_POSITION;
				else if (token.compare("Zposition") == 0)
					channel->type = Z_POSITION;
			}
		}

		// Motion�f�[�^�̃Z�N�V�����ֈڂ�
		if (token.compare("MOTION") == 0) {
			break;
		}
	}

	// ���[�V�������̓ǂݍ���

	//�uFrames: ���t���[�����v�𕪊�����
	{
		std::getline(file, line);
		std::string separater = ":";
		std::vector<std::string> StringList;
		StringList = algorithm.split(line, separater);

		if (StringList[0].compare("Frames") != 0) {
			std::cout << "Error:Frames" << std::endl;
			file.close();
		}
		if (StringList[1].empty()) {
			std::cout << "Error:Frames" << std::endl;
			file.close();
		}
		num_frame = stod(StringList[1]);
	}

	//�uFrame Time:1�t���[�����ԁv�𕪊�����
	{
		std::getline(file, line);
		std::string separater = ":";
		std::vector<std::string> StringList;

		//stringList[0](Frame Time)
		StringList = algorithm.split(line, separater);
		
		if (StringList[0].compare("Frame Time") != 0) {
			std::cout << "Error:Frame Time" << std::endl;
			file.close();
		}
		//stringList[1](1�t���[������)
		if (StringList[1].empty()) {
			std::cout << "Error:Frame Time" << std::endl;
			file.close();
		}
		//1�t���[�����Ԃ�ǂݍ���
		interval = stod(StringList[1]);

	}

	//�`�����l���̑���
	num_channel = channels.size();

	//�t���[����*�`�����l����
	motion = new double[num_frame * num_channel];

	// ���[�V�����f�[�^�̓ǂݍ���
	std::string separater = std::string(" ");

	for (i = 0; i < num_frame; i++)//i�F�t���[����,j�F�`�����l����
	{
		std::getline(file, line);
		std::vector<std::string> StringList;

		StringList = algorithm.split(line, separater);

		//���[�V�����f�[�^������ɓǂݍ��܂�Ȃ��ꍇ
		if (StringList.size() != num_channel) {
			std::cout << "Error:Motion Data"<< std::endl;
			std::cout << "Frame NO : "<< i << std::endl;
			file.close();
			break;
		}
		for (j = 0; j < num_channel; j++)
		{
			motion[i * num_channel + j] = stod(StringList[j]);
		}
	}

	//���_�C���f�b�N�X�z�񐶐�
	vertexIndex = new int[vVertexIndex.size()];
	//���_�C���f�b�N�X�z��֑��(vector��int)
	for (int i = 0; i < vVertexIndex.size(); i++) {
		vertexIndex[i] = vVertexIndex[i];
	}
	//���_���擾
	vertexIndexSize = vVertexIndex.size();

	// �t�@�C���̃N���[�Y
	file.close();

	// ���[�h�̐���
	isloadsuccess = true;
	return;

}
//MatrixCreate�̃��b�p�[�֐�
void BVH::MatrixCreate(int frame_no) {
	MatrixCreate(joints[0], motion + frame_no * num_channel);
}

//�e�֐߂��Ƃ̍s��𐶐������_�v�Z
void BVH::MatrixCreate(const Joint* joint, const double* data) {

	//�s��v�Z�p�ϐ�
	Matrix mat;
	mat.loadIdentity();
	for (int i = 0; i < joint->channels.size(); i++)
	{
		//�e�����Ƃ̉�]�s��𐶐����A��]���|����
		Channel* channel = joint->channels[i];
		if (channel->type == X_ROTATION) {
			mat.rotate(1.0f, 0.0f, 0.0f, data[channel->index]);
		}
		else if (channel->type == Y_ROTATION) {
			mat.rotate(0.0f, 1.0f, 0.0f, data[channel->index]);
		}
		else if (channel->type == Z_ROTATION) {
			mat.rotate(0.0f, 0.0f, 1.0f, data[channel->index]);
		}
	}

	//��]�s���(��]�s�����)���s�ړ���������
	bvhMatrix[joint->index] = mat;
	bvhMatrix[joint->index].array[12] = joint->offset[0];
	bvhMatrix[joint->index].array[13] = joint->offset[1];
	bvhMatrix[joint->index].array[14] = joint->offset[2];
	bvhMatrix[joint->index].array[15] = 1;


	if (joint->parent != nullptr) {
		//�e�̊֐߂̍s����|���邱�ƂŐe�̏d�݂����������ۂ̍s������߂�
		bvhMatrix[joint->index].multiply(bvhMatrix[joint->parent->index], bvhMatrix[joint->index]);
	}

	//�S�̂̕��s�ړ��s��
	Matrix translate;
	translate.loadTranslate(data[0], data[1], data[2]);
	//��]�s�񂩂�擾�������s�ړ�����
	float tmp[4] = { bvhMatrix[joint->index].array[12] ,bvhMatrix[joint->index].array[13] ,bvhMatrix[joint->index].array[14] ,1 };
	//�S�̂̕��s�ړ��s�����]�s�񂩂�擾�������s�ړ������֊|����
	translate.projection(tmp, tmp);
	//���������s�񂩂畽�s�ړ������𔲂��o���A�V�F�[�_�[�̒��_�ւƓn��
	vertexArray[(joint->index * 3) + 0] = tmp[0];
	vertexArray[(joint->index * 3) + 1] = tmp[1];
	vertexArray[(joint->index * 3) + 2] = tmp[2];


	//�ċA�֐��Őe����q�v�f�ւƌv�Z���s���Ă���
	for (int i = 0; i < joint->children.size(); i++)
	{
		MatrixCreate(joint->children[i], data);
	}
}
