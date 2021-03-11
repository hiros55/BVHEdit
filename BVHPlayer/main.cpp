#ifdef WIN32
#include <windows.h>
#  pragma comment(lib, "glew32.lib")
#include "glew.h"
#include "glut.h"
#endif
#include <iostream>
#include <cstdlib>

/*���_csv�ǂݍ���*/
#include"ReadMotionData.hpp"
ReadMotionData *readmotion = NULL;
bool bReadMotion = false;
int readMotionIndex;
float csvMotionVertex[51];

/*Bvh�ҏW*/
#include"Bvhedit.hpp"
Bvhedit bvhedit;

/*�s��E�N�H�[�^�j�I��*/
#include "Matrix.hpp"
#include "Quaternion.hpp"


/* BVH�f�[�^*/
#include "BVH.hpp"
BVH* bvh = NULL;
// �A�j���[�V�����̍Đ�����
float  animationTime = 0.0f;
//�t���[���ԍ�
int frameNum = 0;
//�ꎞ��~�p
bool   onAnimation = true;

/*�V�F�[�_�[�f�[�^*/
#include "Shader.hpp"
Shader shader;
//�V�F�[�_�[�t�@�C����
static const char vVertSource[] = "varray.vert";
static const char vFragSource[] = "varray.frag";
//�V�F�[�_�[�I�u�W�F�N�g
static GLuint bvhProgram;
static GLuint csvProgram;
//�V�F�[�_�[�v���O������ID
static GLuint bvhVertexposition = 0;
static GLuint csvVertexposition = 0;
//���f���r���[�s��ϐ�
static Matrix bvhViewMatrix;
static GLint bvhModelViewMatrixLocation;
static Matrix csvViewMatrix;
static GLint csvModelViewMatrixLocation;
//�v���W�F�N�V�����s��(���s���e�ϊ��s��)
static Matrix bvhProjectionMatrix;
static GLint bvhProjectionMatrixLocation;
static Matrix csvProjectionMatrix;
static GLint csvProjectionMatrixLocation;
//�o�b�t�@�I�u�W�F�N�g��
static GLuint BvhBuffer = 0;
static GLuint CsvBuffer = 0;

/*glui*/
#include"glui.h"
//UI�E�B���h�E�C���f�b�N�X
int glutWindow;
//UI�p�{�^���C���f�b�N�X
int loadBvhButton;
int loadCsvButton;
int addLineButton;
int deleteLineButton;
int stopMotionButton;

//UI�p���t���[�����ϐ�
GLUI_EditText* editNumFrame;

//UI�p�e�L�X�g�o�̓t�@�C����
GLUI_String exportPathText;
GLUI_EditText *editExportPath;
//�o�̓t�@�C�����ϐ�
std::string exportPath = "";

//UI�p�̃C���^�[�o���ϐ�
float interval = 0.001f;

//UI�pAddline�֐��̍������݃t���[���C���f�b�N�X
int insetFrameIndex  = 0;
GLUI_EditText* editInsetFrameIndex;
int insetFrameIndexText;
//UI�pAddline�֐��̍������݃t���[����
int insetFrameNum = 0;
GLUI_EditText* editInsetFrameNum;
int insetFrameNumText;

//UI�pDeleteLine�֐��̍폜�t���[���X�^�[�g�C���f�b�N�X
int deleteStartIndex = 0;
GLUI_EditText* editDeleteStartIndex;
int deleteStartText;
//UI�pDeleteLine�֐��̍폜�t���[���G���h�C���f�b�N�X
int deleteEndIndex = 0;
GLUI_EditText* editDeleteEndIndex;
int deleteEndText;


//�V�F�[�_�[�ւ̒��_�o�^
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

//�֘A�t��&�`��&���(bvh)
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

//�֘A�t��&�`��&���(csv)
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
        //�V�F�[�_�[�v���O�����L��
        glUseProgram(bvhProgram);
        glUniformMatrix4fv(bvhModelViewMatrixLocation, 1, GL_FALSE, bvhViewMatrix.get());
        glUniformMatrix4fv(bvhProjectionMatrixLocation, 1, GL_FALSE, bvhProjectionMatrix.get());
        bvh->MatrixCreate(frameNum);
        BvhFrameCreate(bvhVertexposition);
    }

    //csv�ǂݍ���
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

//ToDo:�}�E�X�ɂ�鎋�_�ύX

void resize(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    int cx, cy;
    /* �E�B���h�E�̒��S�����߂� */
    cx = w / 2;
    cy = h / 2;
    /* �E�B���h�E�S�̂��r���[�|�[�g�ɂ��� */
    glViewport(0, 0, w, h);

    bvhProjectionMatrix.loadIdentity();
    bvhProjectionMatrix.translate(0,0,1);
    bvhProjectionMatrix.orthogonal(60, -60, -60, 60, 0.1f, 500);

    csvProjectionMatrix.loadIdentity();
    csvProjectionMatrix.translate(0, 0, 1);
    csvProjectionMatrix.orthogonal(60, -60, -60, 60, 0.1f, 500);
}

//BVH�t�@�C�����[�h�֐�
std::string LoadBvh() {
#ifdef  WIN32
    const int  fileNameLen = 256;
    char  fileName[fileNameLen] = "";

    // �t�@�C���_�C�A���O�̐ݒ�
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
    // �t�@�C���_�C�A���O��\��
    BOOL  ret = GetOpenFileName(&open_file);
    // �t�@�C�����w�肳�ꂽ��V���������ݒ�
    if (ret)
    {
        // ����f�[�^��ǂݍ���
        if (bvh) delete  bvh;
        

        bvh = new BVH(fileName);

        // �ǂݍ��݂Ɏ��s������폜
        if (!bvh->IsLoadSuccess())
        {
            delete  bvh;
            bvh = NULL;
            std::cout << "Load bvhfile : failed" << std::endl;;
        }
        else {
            std::cout << "Load bvhfile : success" << std::endl;
            //readmotion�C���X�^���X�폜
            if (readmotion) {
                delete readmotion;
                readmotion = NULL;
            }
        }

        //	�A�j���[�V���������Z�b�g
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

    // �t�@�C���_�C�A���O�̐ݒ�
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
    // �t�@�C���_�C�A���O��\��
    BOOL  ret = GetOpenFileName(&open_file);
    if (ret) {
        if (readmotion)delete readmotion;

        readmotion = new ReadMotionData();
        if (readmotion->ReadMotionFile(fileName)) {
            bReadMotion = true;
            //bvh�C���X�^���X�폜
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
        //���ݎ��Ԏ擾
        double  curr_time = timeGetTime();

        double  delta = (curr_time - last_time) * interval;
        if (delta > 0.03f)
            delta = 0.03f;
        last_time = curr_time;
        animationTime += delta;

        // ���݂̃t���[���ԍ����v�Z
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
//UI�pLoadCsv���b�p�[�֐�
void UiloadCsv() {
    std::string s = LoadCsv();
}
//UI�pLoadBvh���b�p�[�֐�
void UiloadBvh() {
    std::string s = LoadBvh();
}
//UI�pbvhedit.AddLine���b�p�[�֐�
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
//UI�pbvhedit.DeleteLine���b�p�[�֐�
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
//UI�ponAnimation�t���O
void UiStopMotion() {
    onAnimation = !onAnimation;
}
//UI�pexportPath�ϐ��ݒ�
void UiSetExportPath() {
    exportPath = editExportPath->get_text();
}
//UI�pnumFrame�\���p
void UiSetNumFrame() {
    std::string str = std::to_string(frameNum);
   // editNumFrame->set_text((GLUI_String)str);
    std::cout<<str<< std::endl;
}
//UI�pInsetFrameIndex�ϐ��ݒ�
void UISetInsetFrameIndex() {
    insetFrameIndex =std::stoi(editInsetFrameIndex->get_text());
}
//UI�pInsetFrameNum�ϐ��ݒ�
void UISetInsertFramNum() {
    insetFrameNum = std::stoi(editInsetFrameNum->get_text());
}
//UI�pdeleteStartIndex�ϐ��ݒ�
void UISetDeleteStartIndex() {
    deleteStartIndex = std::stoi(editDeleteStartIndex->get_text());
}
//UI�pdeleteEndIndex�ϐ��ݒ�
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

    //AddLine�֐��pUI�ݒ�
    GLUI_Panel* AddLine = new GLUI_Panel(EditBvh, "AddLine", true);
    editInsetFrameIndex = new GLUI_EditText(AddLine, "InsetFrameIndex", &insetFrameIndexText, -1, (GLUI_Update_CB)UISetInsetFrameIndex);
    editInsetFrameNum = new GLUI_EditText(AddLine, "InsetFrameNum", &insetFrameNumText, -1, (GLUI_Update_CB)UISetInsertFramNum);
    new GLUI_Button(AddLine, "AddLine", addLineButton, (GLUI_Update_CB)UiAddLine);

    //Delete�֐��pUI�ݒ�
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

//�f�o�b�O�p
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
    /* GLEW �̏����� */
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(1);
    }
#endif
    
    //�V�F�[�_�[�ǂݍ���(Shader.hpp)
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
