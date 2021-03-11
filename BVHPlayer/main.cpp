#ifdef WIN32
#include <windows.h>
#  pragma comment(lib, "glew32.lib")
#include "glew.h"
#include "glut.h"
#endif
#include <iostream>
#include <cstdlib>

/*頂点csv読み込み*/
#include"ReadMotionData.hpp"
ReadMotionData *readmotion = NULL;
bool bReadMotion = false;
int readMotionIndex;
float csvMotionVertex[51];

/*Bvh編集*/
#include"Bvhedit.hpp"
Bvhedit bvhedit;

/*行列・クォータニオン*/
#include "Matrix.hpp"
#include "Quaternion.hpp"


/* BVHデータ*/
#include "BVH.hpp"
BVH* bvh = NULL;
// アニメーションの再生時間
float  animationTime = 0.0f;
//フレーム番号
int frameNum = 0;
//一時停止用
bool   onAnimation = true;

/*シェーダーデータ*/
#include "Shader.hpp"
Shader shader;
//シェーダーファイル名
static const char vVertSource[] = "varray.vert";
static const char vFragSource[] = "varray.frag";
//シェーダーオブジェクト
static GLuint bvhProgram;
static GLuint csvProgram;
//シェーダープログラムのID
static GLuint bvhVertexposition = 0;
static GLuint csvVertexposition = 0;
//モデルビュー行列変数
static Matrix bvhViewMatrix;
static GLint bvhModelViewMatrixLocation;
static Matrix csvViewMatrix;
static GLint csvModelViewMatrixLocation;
//プロジェクション行列(平行投影変換行列)
static Matrix bvhProjectionMatrix;
static GLint bvhProjectionMatrixLocation;
static Matrix csvProjectionMatrix;
static GLint csvProjectionMatrixLocation;
//バッファオブジェクト名
static GLuint BvhBuffer = 0;
static GLuint CsvBuffer = 0;

/*glui*/
#include"glui.h"
//UIウィンドウインデックス
int glutWindow;
//UI用ボタンインデックス
int loadBvhButton;
int loadCsvButton;
int addLineButton;
int deleteLineButton;
int stopMotionButton;

//UI用総フレーム数変数
GLUI_EditText* editNumFrame;

//UI用テキスト出力ファイル名
GLUI_String exportPathText;
GLUI_EditText *editExportPath;
//出力ファイル名変数
std::string exportPath = "";

//UI用のインターバル変数
float interval = 0.001f;

//UI用Addline関数の差し込みフレームインデックス
int insetFrameIndex  = 0;
GLUI_EditText* editInsetFrameIndex;
int insetFrameIndexText;
//UI用Addline関数の差し込みフレーム数
int insetFrameNum = 0;
GLUI_EditText* editInsetFrameNum;
int insetFrameNumText;

//UI用DeleteLine関数の削除フレームスタートインデックス
int deleteStartIndex = 0;
GLUI_EditText* editDeleteStartIndex;
int deleteStartText;
//UI用DeleteLine関数の削除フレームエンドインデックス
int deleteEndIndex = 0;
GLUI_EditText* editDeleteEndIndex;
int deleteEndText;


//シェーダーへの頂点登録
GLuint RegisterBuffer(GLuint* Buffer, float* data, int vertexcount,std::string bvhorcsv) {
    int axisInfoy = 0 ; int axisInfoz = 0 ;
    typedef GLdouble Vertex[4];
    if (bvhorcsv.compare("bvh") == 0) {
        axisInfoy = 1;axisInfoz = 2;
    }
    else if (bvhorcsv.compare("csv") == 0) {
        axisInfoy = 2;axisInfoz = 1;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffer[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Vertex) * vertexcount, NULL, GL_STREAM_DRAW);
    Vertex* vertex = (Vertex*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE);
    for (int i = 0; i < vertexcount; i += 3) {
        (*vertex)[0] = data[i];
        (*vertex)[1] = data[i + axisInfoy];
        (*vertex)[2] = data[i + axisInfoz];
        (*vertex)[3] = 1.0;
        vertex++;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return vertexcount;
}

//関連付け&描画&解放(bvh)
void BvhFrameCreate(GLuint position) {
    glGenBuffers(1, &BvhBuffer);
    RegisterBuffer(&BvhBuffer, bvh->vertexArray, bvh->vertexIndexSize * 4,"bvh");
    glEnableVertexAttribArray(position);
    glBindBuffer(GL_ARRAY_BUFFER, BvhBuffer);
    glVertexAttribPointer(0, 4, GL_DOUBLE, GL_FALSE, 0, 0);
    glDrawElements(GL_LINE_LOOP, bvh->vertexIndexSize, GL_UNSIGNED_INT, bvh->vertexIndex);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(position);
    glDeleteBuffers(1, &BvhBuffer);
}

//関連付け&描画&解放(csv)
void CsvFrameCreate(GLuint position) {
    glGenBuffers(1, &CsvBuffer);
    RegisterBuffer(&CsvBuffer, csvMotionVertex, (sizeof(readmotion->motiondataIndex)/ sizeof(readmotion->motiondataIndex[0])) * 4,"csv");
    glEnableVertexAttribArray(position);
    glBindBuffer(GL_ARRAY_BUFFER, CsvBuffer);
    glVertexAttribPointer(0, 4, GL_DOUBLE, GL_FALSE, 0, 0);
    glDrawElements(GL_LINE_LOOP, (sizeof(readmotion->motiondataIndex) / sizeof(readmotion->motiondataIndex[0])), GL_UNSIGNED_INT, readmotion->motiondataIndex);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(position);
    glDeleteBuffers(1, &CsvBuffer);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);

    
    if (bvh) {
        //シェーダープログラム有効
        glUseProgram(bvhProgram);
        glUniformMatrix4fv(bvhModelViewMatrixLocation, 1, GL_FALSE, bvhViewMatrix.get());
        glUniformMatrix4fv(bvhProjectionMatrixLocation, 1, GL_FALSE, bvhProjectionMatrix.get());
        bvh->MatrixCreate(frameNum);
        BvhFrameCreate(bvhVertexposition);
    }

    //csv読み込み
   if (readmotion && bReadMotion) {
       glUseProgram(csvProgram);
       glUniformMatrix4fv(csvModelViewMatrixLocation, 1, GL_FALSE, csvViewMatrix.get());
       glUniformMatrix4fv(csvProjectionMatrixLocation, 1, GL_FALSE, csvProjectionMatrix.get());
        if(readMotionIndex >= readmotion->csvFrameNum)readMotionIndex = 0;
        Sleep(40);
        for (int i = 0; i < readmotion->jointNum * readmotion->axisNum; i++) {
            csvMotionVertex[i] = readmotion->motiondata[readMotionIndex * readmotion->jointNum * readmotion->axisNum + i];
        }
        CsvFrameCreate(csvVertexposition);
        readMotionIndex++;
    }
    glutSwapBuffers();
    glutPostRedisplay();
}

//ToDo:マウスによる視点変更

void resize(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    int cx, cy;
    /* ウィンドウの中心を求める */
    cx = w / 2;
    cy = h / 2;
    /* ウィンドウ全体をビューポートにする */
    glViewport(0, 0, w, h);

    bvhProjectionMatrix.loadIdentity();
    bvhProjectionMatrix.translate(0,0,1);
    bvhProjectionMatrix.orthogonal(60, -60, -60, 60, 0.1f, 500);

    csvProjectionMatrix.loadIdentity();
    csvProjectionMatrix.translate(0, 0, 1);
    csvProjectionMatrix.orthogonal(60, -60, -60, 60, 0.1f, 500);
}

//BVHファイルロード関数
std::string LoadBvh() {
#ifdef  WIN32
    const int  fileNameLen = 256;
    char  fileName[fileNameLen] = "";

    // ファイルダイアログの設定
    OPENFILENAME	open_file;
    memset(&open_file, 0, sizeof(OPENFILENAME));
    open_file.lStructSize = sizeof(OPENFILENAME);
    open_file.hwndOwner = NULL;
    open_file.lpstrFilter = "BVH Motion Data (*.bvh)\0*.bvh\0All (*.*)\0*.*\0";
    open_file.nFilterIndex = 1;
    open_file.lpstrFile = fileName;
    open_file.nMaxFile = fileNameLen;
    open_file.lpstrTitle = "Select a BVH file";
    open_file.lpstrDefExt = "bvh";
    open_file.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    // ファイルダイアログを表示
    BOOL  ret = GetOpenFileName(&open_file);
    // ファイルが指定されたら新しい動作を設定
    if (ret)
    {
        // 動作データを読み込み
        if (bvh) delete  bvh;
        

        bvh = new BVH(fileName);

        // 読み込みに失敗したら削除
        if (!bvh->IsLoadSuccess())
        {
            delete  bvh;
            bvh = NULL;
            std::cout << "Load bvhfile : failed" << std::endl;;
        }
        else {
            std::cout << "Load bvhfile : success" << std::endl;
            //readmotionインスタンス削除
            if (readmotion) {
                delete readmotion;
                readmotion = NULL;
            }
        }

        //	アニメーションをリセット
        animationTime = 0.0f;
        frameNum = 0;

        return fileName;
    }
    else {
        return "\n";
    }

#endif
}

std::string LoadCsv() {
#ifdef  WIN32
    const int  fileNameLen = 256;
    char  fileName[fileNameLen] = "";

    // ファイルダイアログの設定
    OPENFILENAME	open_file;
    memset(&open_file, 0, sizeof(OPENFILENAME));
    open_file.lStructSize = sizeof(OPENFILENAME);
    open_file.hwndOwner = NULL;
    open_file.lpstrFilter = "csv Motion Data (*.csv)\0*.csv\0All (*.*)\0*.*\0";
    open_file.nFilterIndex = 1;
    open_file.lpstrFile = fileName;
    open_file.nMaxFile = fileNameLen;
    open_file.lpstrTitle = "Select a CSV file";
    open_file.lpstrDefExt = "csv";
    open_file.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    // ファイルダイアログを表示
    BOOL  ret = GetOpenFileName(&open_file);
    if (ret) {
        if (readmotion)delete readmotion;

        readmotion = new ReadMotionData();
        if (readmotion->ReadMotionFile(fileName)) {
            bReadMotion = true;
            //bvhインスタンス削除
            if (bvh) {
                delete bvh;
                bvh = NULL;
            }
        }
        return fileName;
    }
#endif

}

void idle(void)
{
    if (onAnimation)
    {
        static  double  last_time = 0;
        //現在時間取得
        double  curr_time = timeGetTime();

        double  delta = (curr_time - last_time) * interval;
        if (delta > 0.03f)
            delta = 0.03f;
        last_time = curr_time;
        animationTime += delta;

        // 現在のフレーム番号を計算
        if (bvh)
        {
            frameNum = animationTime / bvh->GetInterval();
            frameNum = frameNum % bvh->GetNumFrame();
            glRasterPos2d(0,0);
            std::string currentFrame = std::to_string(frameNum);
            for (int i = 0; i < currentFrame.length(); ++i) {
                char ic = currentFrame[i];
                glutBitmapCharacter(GLUT_BITMAP_9_BY_15, ic);
            }
        }
        else
            frameNum = 0;
    }
    glutPostRedisplay();
}
//UI用LoadCsvラッパー関数
void UiloadCsv() {
    std::string s = LoadCsv();
}
//UI用LoadBvhラッパー関数
void UiloadBvh() {
    std::string s = LoadBvh();
}
//UI用bvhedit.AddLineラッパー関数
void UiAddLine() {
    std::string s = LoadBvh();
    if (bvh) {
        if (bvhedit.AddLine(s, exportPath, bvh->num_frame, insetFrameIndex, insetFrameNum, bvh->channels.size(), bvh->motion)) {
            std::cout << "Rewrite the number of frames:success" << std::endl;
        }
        else {
            std::cout << "Rewrite the number of frames:failed" << std::endl;
        }
    }
}
//UI用bvhedit.DeleteLineラッパー関数
void UiDeleteLine() {
    std::vector<int> frameIndex;
    std::string s = LoadBvh();
    for (int i = deleteStartIndex; i < deleteEndIndex; i++) {
        frameIndex.push_back(i);
    }
    if (bvh) {
        if (bvhedit.DeleteLine(s, exportPath, frameIndex, bvh->num_frame)) {
            std::cout << "Rewrite the number of frames:success" << std::endl;
        }
        else {
            std::cout << "Rewrite the number of frames:failed" << std::endl;
        }
    }
    frameIndex.clear();
}
//UI用onAnimationフラグ
void UiStopMotion() {
    onAnimation = !onAnimation;
}
//UI用exportPath変数設定
void UiSetExportPath() {
    exportPath = editExportPath->get_text();
}
//UI用numFrame表示用
void UiSetNumFrame() {
    std::string str = std::to_string(frameNum);
   // editNumFrame->set_text((GLUI_String)str);
    std::cout<<str<< std::endl;
}
//UI用InsetFrameIndex変数設定
void UISetInsetFrameIndex() {
    insetFrameIndex =std::stoi(editInsetFrameIndex->get_text());
}
//UI用InsetFrameNum変数設定
void UISetInsertFramNum() {
    insetFrameNum = std::stoi(editInsetFrameNum->get_text());
}
//UI用deleteStartIndex変数設定
void UISetDeleteStartIndex() {
    deleteStartIndex = std::stoi(editDeleteStartIndex->get_text());
}
//UI用deleteEndIndex変数設定
void UISetDeleteEndIndex() {
    deleteEndIndex = std::stoi(editDeleteEndIndex->get_text());
}

void glui() {
    GLUI* glui = GLUI_Master.create_glui("GLUI Window", 0, 300, 300);
    glui->add_statictext("BVH Player"); 
    GLUI_Panel* csvPanel = new GLUI_Panel(glui, "LoadCsv", true);
    new GLUI_Button(csvPanel, "LoadCsv", loadCsvButton, (GLUI_Update_CB)UiloadCsv);
    GLUI_Panel* bvhPanel = new GLUI_Panel(glui, "LoadBvh", true);
    new GLUI_Button(bvhPanel, "LoadBvh", loadBvhButton,(GLUI_Update_CB)UiloadBvh);
    
    GLUI_Panel* EditBvh = new GLUI_Panel(glui, "EditBvh", true);
    editExportPath = new GLUI_EditText(EditBvh, "ExportFileName", &exportPathText,-1, (GLUI_Update_CB)UiSetExportPath);

    //AddLine関数用UI設定
    GLUI_Panel* AddLine = new GLUI_Panel(EditBvh, "AddLine", true);
    editInsetFrameIndex = new GLUI_EditText(AddLine, "InsetFrameIndex", &insetFrameIndexText, -1, (GLUI_Update_CB)UISetInsetFrameIndex);
    editInsetFrameNum = new GLUI_EditText(AddLine, "InsetFrameNum", &insetFrameNumText, -1, (GLUI_Update_CB)UISetInsertFramNum);
    new GLUI_Button(AddLine, "AddLine", addLineButton, (GLUI_Update_CB)UiAddLine);

    //Delete関数用UI設定
    GLUI_Panel* DeleteLine = new GLUI_Panel(EditBvh, "DeleteLine", true);
    editDeleteStartIndex = new GLUI_EditText(DeleteLine, "DeleteStartIndex", &deleteStartText, -1, (GLUI_Update_CB)UISetDeleteStartIndex);
    editDeleteEndIndex = new GLUI_EditText(DeleteLine, "DeleteEndIndex", &deleteEndText, -1, (GLUI_Update_CB)UISetDeleteEndIndex);
    new GLUI_Button(DeleteLine, "DeleteLine", deleteLineButton, (GLUI_Update_CB)UiDeleteLine);

    glui->add_separator();
    glui->add_statictext("MotionSpeed");
    GLUI_Scrollbar* motionSpeedBar = new GLUI_Scrollbar(glui, "MotionSpeed", GLUI_SCROLL_HORIZONTAL, &interval, -1);
    motionSpeedBar->set_float_limits(0.01, 0.0001);

    glui->add_button("Stop/Start", stopMotionButton, (GLUI_Update_CB)UiStopMotion);

    glui->add_separator();
    new GLUI_Button(glui, "Quit", 0, (GLUI_Update_CB)exit);
    glui->set_main_gfx_window(glutWindow);
    GLUI_Master.set_glutIdleFunc(idle); 
}

//デバッグ用
void keyboard(unsigned char key, int x, int y)
{
    if (key == 's') {
        onAnimation = !onAnimation;
    }
    if (key == 'l')
    {
        std::string s = LoadBvh();
    }
}

void init(void)
{
#if defined(WIN32)
    /* GLEW の初期化 */
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(1);
    }
#endif
    
    //シェーダー読み込み(Shader.hpp)
    if ((bvhProgram = shader.LoadShader(vVertSource, vFragSource, bvhVertexposition, "Position")) == 0) {
        printf("Error:LoadShader");
        exit(1);
    }

    bvhViewMatrix.loadIdentity();
    bvhViewMatrix.lookat(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    bvhModelViewMatrixLocation = glGetUniformLocation(bvhProgram, "modelViewMatrix");
    bvhProjectionMatrixLocation = glGetUniformLocation(bvhProgram, "projectionMatrix");

    
    if ((csvProgram = shader.LoadShader(vVertSource, vFragSource, csvVertexposition, "Position")) == 0) {
        printf("Error:LoadShader");
        exit(1);
    }
    csvViewMatrix.loadIdentity();
    csvViewMatrix.lookat(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    csvModelViewMatrixLocation = glGetUniformLocation(csvProgram, "modelViewMatrix");
    csvProjectionMatrixLocation = glGetUniformLocation(csvProgram, "projectionMatrix");
    
   

    glClearColor(1.0, 1.0, 1.0, 1.0);
}


int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutWindow = glutCreateWindow(argv[0]);
    glui();
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyboard);
    init();
    glutMainLoop();

    return 0;
}
