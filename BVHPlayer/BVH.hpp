#pragma once
#include <vector>
#include <map>
#include <string>
#include"Matrix.hpp"

class  BVH
{
public:


	// �`�����l���̎��
	enum  ChannelEnum
	{
		X_ROTATION, Y_ROTATION, Z_ROTATION,
		X_POSITION, Y_POSITION, Z_POSITION
	};
	struct  Joint;

	// �`�����l�����
	struct  Channel
	{
		// �Ή��֐�
		Joint* joint;

		// �`�����l���̎��
		ChannelEnum          type;

		// �`�����l���ԍ�
		int                  index;
	};

	// �֐ߏ��
	struct  Joint
	{
		// �֐ߖ�
		std::string               name;
		// �֐ߔԍ�
		int                  index;

		// �֐ߊK�w�i�e�֐߁j
		Joint* parent;
		// �֐ߊK�w�i�q�֐߁j
		std::vector< Joint* >    children;

		// �ڑ��ʒu
		double               offset[3];

		// ���[�ʒu���������ǂ����̃t���O
		bool                 has_site;
		// ���[�ʒu
		double               site[3];

		// ��]��
		std::vector< Channel* >  channels;
	};


public:
	// ���[�h�������������ǂ����̃t���O
	bool                     isloadsuccess;

	/*  �t�@�C���̏��  */
	std::string                   file_name;   // �t�@�C����

	/*  �K�w�\���̏��  */
	int                      num_channel; // �`�����l����
	std::vector< Channel* >      channels;    // �`�����l����� [�`�����l���ԍ�]
	std::vector< Joint* >        joints;      // �֐ߏ�� [�p�[�c�ԍ�]
	std::map< std::string, Joint* >   joint_index; // �֐ߖ�����֐ߏ��ւ̃C���f�b�N�X

	/*  ���[�V�����f�[�^�̏��  */
	int                      num_frame;   // �t���[����
	double                   interval;    // �t���[���Ԃ̎��ԊԊu
	double* motion;      // [�t���[���ԍ�][�`�����l���ԍ�]

	//�֐ߍs��v�Z�p
	Matrix* bvhMatrix;

	/*�V�F�[�_�[�L�q�p*/
	//���_�z��
	float* vertexArray;
	//���_�C���f�b�N�X�z��
	int* vertexIndex;
	//���_�C���f�b�N�X��
	int vertexIndexSize;

public:
	// �R���X�g���N�^�E�f�X�g���N�^
	BVH();
	BVH(const char* bvh_file_name);
	~BVH();

	// �S���̃N���A
	void  Clear();

	// BVH�t�@�C���̃��[�h
	void  Load(const char* bvh_file_name);

public:
	/*  �f�[�^�A�N�Z�X�֐�  */

	// ���[�h�������������ǂ������擾
	bool  IsLoadSuccess() const { return isloadsuccess; }

	// ���[�V�����f�[�^�̏��̎擾
	int     GetNumFrame() const { return  num_frame; }
	double  GetInterval() const { return  interval; }

	//BVH�t�@�C���̍s��ݒ�p�֐�
	void MatrixCreate(int frame_no);
	void MatrixCreate(const Joint* joint, const double* data);
};
