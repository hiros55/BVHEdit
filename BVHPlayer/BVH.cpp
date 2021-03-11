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

// コントラクタ
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



// デストラクタ
BVH::~BVH()
{
	Clear();
}


// 全情報のクリア
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

	//頂点シェーダーの頂点インデックス可変長配列
	std::vector<int> vVertexIndex;

	// 初期化
	Clear();

	// ファイルの情報（ファイル名・動作名）の設定
	file_name = bvh_file_name;

	// ファイルのオープン
	file.open(bvh_file_name, std::ios::in);
	if (file.is_open() == 0) { 
		std::cout << "Counld not load file" << std::endl;
		return;
	}

	// 階層情報の読み込み
	while (!file.eof())
	{
		// ファイルの最後まできてしまったら異常終了
		if (file.eof()) {
			std::cout << "Error:File Eof" << std::endl;
			file.close();
		}

		// １行読み込み、先頭の単語を取得
		std::getline(file, line);

		//'\t'まで読み込み
		char cseparater = '\t';
		token = algorithm.split(line, cseparater);
		//' 'まで読み込み
		std::string sseparater = " ";
		token = algorithm.split(token, sseparater, true, 0);

		// 空行の場合は次の行へ
		if (token.empty())  continue;

		// 関節ブロックの開始
		if (token.compare("{") == 0)
		{
			// 現在の関節をスタックに積む
			joint_stack.push_back(joint);
			joint = new_joint;
			if (!joint->has_site)
				vVertexIndex.push_back(joint->index);
			continue;
		}
		// 関節ブロックの終了
		if (token.compare("}") == 0)
		{
			if (!joint->has_site)
				vVertexIndex.push_back(joint->index);

			// 現在の関節をスタックから取り出す
			joint = joint_stack.back();
			joint_stack.pop_back();
			is_site = false;

			continue;
		}

		// 関節情報の開始
		if ((token.compare("ROOT") == 0) ||
			(token.compare("JOINT") == 0))
		{
			// 関節データの作成
			new_joint = new Joint();
			new_joint->index = joints.size();
			new_joint->parent = joint;
			new_joint->has_site = false;
			new_joint->offset[0] = 0.0;  new_joint->offset[1] = 0.0;  new_joint->offset[2] = 0.0;
			new_joint->site[0] = 0.0;  new_joint->site[1] = 0.0;  new_joint->site[2] = 0.0;
			joints.push_back(new_joint);
			if (joint)
				joint->children.push_back(new_joint);

			//インデックス登録→頂点index配列生成
			std::string separater = " ";
			line = line.substr(line.find(separater) + 1);
			line = algorithm.split(line, separater, true, 0);
			//関節名読み込み
			new_joint->name = line;

			// インデックスへ追加
			joint_index[new_joint->name] = new_joint;
			continue;
		}

		// 末端情報の開始
		if ((token.compare("End") == 0))
		{
			vVertexIndex.pop_back();
			new_joint = joint;
			is_site = true;
			continue;
		}

		// 関節のオフセット or 末端位置の情報
		if (token.compare("OFFSET") == 0)
		{
			// 座標値を読み込み
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

		// 関節のチャンネル情報
		if (token.compare("CHANNELS") == 0)
		{
			// チャンネル数を読み込み
			//token = strtok(NULL, separater);
			std::string separater = " ";
			//line文字列をCHANNELS文字列を削除したものに更新
			line = line.substr(line.find(separater));

			//tokenを求めるためにtokenlineにlineの分割文字列を代入
			std::string tokenline = line;
			token = algorithm.split(tokenline, separater, true, 1);
			joint->channels.resize(std::stoi(token));

			std::vector<std::string> stringList;
			stringList = algorithm.split(line, separater);

			// チャンネル情報を読み込み（一つ一つのジョイントにチャンネル情報が入る）
			for (i = 0; i < joint->channels.size(); i++)
			{
				// チャンネルの作成
				Channel* channel = new Channel();
				channel->joint = joint;
				channel->index = channels.size();
				channels.push_back(channel);
				joint->channels[i] = channel;

				token = stringList[i + 2];

				// チャンネルの種類の判定
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

		// Motionデータのセクションへ移る
		if (token.compare("MOTION") == 0) {
			break;
		}
	}

	// モーション情報の読み込み

	//「Frames: 総フレーム数」を分割する
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

	//「Frame Time:1フレーム時間」を分割する
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
		//stringList[1](1フレーム時間)
		if (StringList[1].empty()) {
			std::cout << "Error:Frame Time" << std::endl;
			file.close();
		}
		//1フレーム時間を読み込む
		interval = stod(StringList[1]);

	}

	//チャンネルの総数
	num_channel = channels.size();

	//フレーム数*チャンネル数
	motion = new double[num_frame * num_channel];

	// モーションデータの読み込み
	std::string separater = std::string(" ");

	for (i = 0; i < num_frame; i++)//i：フレーム数,j：チャンネル数
	{
		std::getline(file, line);
		std::vector<std::string> StringList;

		StringList = algorithm.split(line, separater);

		//モーションデータが正常に読み込まれない場合
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

	//頂点インデックス配列生成
	vertexIndex = new int[vVertexIndex.size()];
	//頂点インデックス配列へ代入(vector→int)
	for (int i = 0; i < vVertexIndex.size(); i++) {
		vertexIndex[i] = vVertexIndex[i];
	}
	//頂点数取得
	vertexIndexSize = vVertexIndex.size();

	// ファイルのクローズ
	file.close();

	// ロードの成功
	isloadsuccess = true;
	return;

}
//MatrixCreateのラッパー関数
void BVH::MatrixCreate(int frame_no) {
	MatrixCreate(joints[0], motion + frame_no * num_channel);
}

//各関節ごとの行列を生成し頂点計算
void BVH::MatrixCreate(const Joint* joint, const double* data) {

	//行列計算用変数
	Matrix mat;
	mat.loadIdentity();
	for (int i = 0; i < joint->channels.size(); i++)
	{
		//各軸ごとの回転行列を生成し、回転を掛ける
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

	//回転行列に(回転行列内の)平行移動成分を代入
	bvhMatrix[joint->index] = mat;
	bvhMatrix[joint->index].array[12] = joint->offset[0];
	bvhMatrix[joint->index].array[13] = joint->offset[1];
	bvhMatrix[joint->index].array[14] = joint->offset[2];
	bvhMatrix[joint->index].array[15] = 1;


	if (joint->parent != nullptr) {
		//親の関節の行列を掛けることで親の重みも加えた実際の行列を求める
		bvhMatrix[joint->index].multiply(bvhMatrix[joint->parent->index], bvhMatrix[joint->index]);
	}

	//全体の平行移動行列
	Matrix translate;
	translate.loadTranslate(data[0], data[1], data[2]);
	//回転行列から取得した平行移動成分
	float tmp[4] = { bvhMatrix[joint->index].array[12] ,bvhMatrix[joint->index].array[13] ,bvhMatrix[joint->index].array[14] ,1 };
	//全体の平行移動行列を回転行列から取得した平行移動成分へ掛ける
	translate.projection(tmp, tmp);
	//生成した行列から平行移動成分を抜き出し、シェーダーの頂点へと渡す
	vertexArray[(joint->index * 3) + 0] = tmp[0];
	vertexArray[(joint->index * 3) + 1] = tmp[1];
	vertexArray[(joint->index * 3) + 2] = tmp[2];


	//再帰関数で親から子要素へと計算を行っていく
	for (int i = 0; i < joint->children.size(); i++)
	{
		MatrixCreate(joint->children[i], data);
	}
}
