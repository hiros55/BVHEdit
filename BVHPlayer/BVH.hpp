#pragma once
#include <vector>
#include <map>
#include <string>
#include"Matrix.hpp"

class  BVH
{
public:


	// チャンネルの種類
	enum  ChannelEnum
	{
		X_ROTATION, Y_ROTATION, Z_ROTATION,
		X_POSITION, Y_POSITION, Z_POSITION
	};
	struct  Joint;

	// チャンネル情報
	struct  Channel
	{
		// 対応関節
		Joint* joint;

		// チャンネルの種類
		ChannelEnum          type;

		// チャンネル番号
		int                  index;
	};

	// 関節情報
	struct  Joint
	{
		// 関節名
		std::string               name;
		// 関節番号
		int                  index;

		// 関節階層（親関節）
		Joint* parent;
		// 関節階層（子関節）
		std::vector< Joint* >    children;

		// 接続位置
		double               offset[3];

		// 末端位置情報を持つかどうかのフラグ
		bool                 has_site;
		// 末端位置
		double               site[3];

		// 回転軸
		std::vector< Channel* >  channels;
	};


public:
	// ロードが成功したかどうかのフラグ
	bool                     isloadsuccess;

	/*  ファイルの情報  */
	std::string                   file_name;   // ファイル名

	/*  階層構造の情報  */
	int                      num_channel; // チャンネル数
	std::vector< Channel* >      channels;    // チャンネル情報 [チャンネル番号]
	std::vector< Joint* >        joints;      // 関節情報 [パーツ番号]
	std::map< std::string, Joint* >   joint_index; // 関節名から関節情報へのインデックス

	/*  モーションデータの情報  */
	int                      num_frame;   // フレーム数
	double                   interval;    // フレーム間の時間間隔
	double* motion;      // [フレーム番号][チャンネル番号]

	//関節行列計算用
	Matrix* bvhMatrix;

	/*シェーダー記述用*/
	//頂点配列
	float* vertexArray;
	//頂点インデックス配列
	int* vertexIndex;
	//頂点インデックス数
	int vertexIndexSize;

public:
	// コンストラクタ・デストラクタ
	BVH();
	BVH(const char* bvh_file_name);
	~BVH();

	// 全情報のクリア
	void  Clear();

	// BVHファイルのロード
	void  Load(const char* bvh_file_name);

public:
	/*  データアクセス関数  */

	// ロードが成功したかどうかを取得
	bool  IsLoadSuccess() const { return isloadsuccess; }

	// モーションデータの情報の取得
	int     GetNumFrame() const { return  num_frame; }
	double  GetInterval() const { return  interval; }

	//BVHファイルの行列設定用関数
	void MatrixCreate(int frame_no);
	void MatrixCreate(const Joint* joint, const double* data);
};
