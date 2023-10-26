/*********************************************************************************************************************
* TC264 Opensourec Library ����TC264 ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
* Copyright (c) 2022 SEEKFREE ��ɿƼ�
*
* ���ļ��� TC264 ��Դ���һ����
*
* TC264 ��Դ�� ��������
* �����Ը��������������ᷢ���� GPL��GNU General Public License���� GNUͨ�ù������֤��������
* �� GPL �ĵ�3�棨�� GPL3.0������ѡ��ģ��κκ����İ汾�����·�����/���޸���
*
* ����Դ��ķ�����ϣ�����ܷ������ã�����δ�������κεı�֤
* ����û�������������Ի��ʺ��ض���;�ı�֤
* ����ϸ����μ� GPL
*
* ��Ӧ�����յ�����Դ���ͬʱ�յ�һ�� GPL �ĸ���
* ���û�У������<https://www.gnu.org/licenses/>
*
* ����ע����
* ����Դ��ʹ�� GPL3.0 ��Դ���֤Э�� �����������Ϊ���İ汾
* �������Ӣ�İ��� libraries/doc �ļ����µ� GPL3_permission_statement.txt �ļ���
* ���֤������ libraries �ļ����� �����ļ����µ� LICENSE �ļ�
* ��ӭ��λʹ�ò����������� ���޸�����ʱ���뱣����ɿƼ��İ�Ȩ����������������
*
* �ļ�����          cpu0_main
* ��˾����          �ɶ���ɿƼ����޹�˾
* �汾��Ϣ          �鿴 libraries/doc �ļ����� version �ļ� �汾˵��
* ��������          ADS v1.8.0
* ����ƽ̨          TC264D
* ��������          https://seekfree.taobao.com/
*
* �޸ļ�¼
* ����              ����                ��ע
* 2022-09-15       pudding            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#pragma section all "cpu0_dsram"
// ���������#pragma section all restore���֮���ȫ�ֱ���������CPU0��RAM��


// ���̵��뵽���֮��Ӧ��ѡ�й���Ȼ����refreshˢ��һ��֮���ٱ���
// ����Ĭ������Ϊ�ر��Ż��������Լ��һ�����ѡ��properties->C/C++ Build->Setting
// Ȼ�����Ҳ�Ĵ������ҵ�C/C++ Compiler->Optimization->Optimization level�������Ż��ȼ�
// һ��Ĭ���½����Ĺ��̶���Ĭ�Ͽ�2���Ż�����˴��Ҳ��������Ϊ2���Ż�

// ����TCϵ��Ĭ���ǲ�֧���ж�Ƕ�׵ģ�ϣ��֧���ж�Ƕ����Ҫ���ж���ʹ�� interrupt_global_enable(); �������ж�Ƕ��
// �򵥵�˵ʵ���Ͻ����жϺ�TCϵ�е�Ӳ���Զ������� interrupt_global_disable(); ���ܾ���Ӧ�κε��жϣ������Ҫ�����Լ��ֶ����� interrupt_global_enable(); �������жϵ���Ӧ��

// *************************** ����Ӳ������˵�� ***************************
// ���İ��������缴�� �����������
// ���ʹ��������� �������Ҫ�õ�ع���


// *************************** ���̲���˵�� ***************************
// 1.���İ���¼��ɱ����̣�����ϵ�
// 2.���Կ������İ����ĸ� LED ����ˮ��״��˸
// 3.�� SWITCH1 / SWITCH2 �����궨���Ӧ�����ŷֱ��� 00 01 10 11 ����Ͻӵ� 1-VCC 0-GND ���߲�����Ӧ����Ĳ��뿪��
// 3.��ͬ������£��ĸ� LED ��ˮ��״��˸��Ƶ�ʻᷢ���仯
// 4.�� KEY1 / KEY2 / KEY3 / KEY4 �����궨���Ӧ�����Žӵ� 1-VCC 0-GND ���� ����Ӧ����
// 5.�������Ž� GND ���� �������»�ʹ������LEDһ����˸ �ɿ���ָ���ˮ��
// �������������˵�����ز��� ����ձ��ļ����·� ���̳�������˵�� �����Ų�

// **************************** �������� ****************************
#define KEY1                    (P11_3)
#define KEY2                    (P11_2)
#define KEY3                    (P20_7)
#define KEY4                    (P20_6)

//������ֵ
#define BUTTON_NONE     0
#define BUTTON_RIGHT    1
#define BUTTON_UP       2
#define BUTTON_DOWN     3
#define BUTTON_C        4
#define BUTTON_LEFT     5

void button_scan(void);
int img_otsu_acc(uint8_t *img);
short GetOTSU (uint8_t *img);
void Longestwhite(uint8_t *img, int th);
void SimpleMiddle (uint8_t *img, int th);
void RightSearchLine(uint8_t *img, int *step, int thred);
void LeftSearchLine(uint8_t *img, int *step, int thred );
void MiSearchLine(uint8_t *img, int thred);

float error = 0.0;
int leftborders[100] = {0};
int rightborders[100] = {0};
int leftpts[100][2] = {0};
int rightpts[100][2] = {0};
uint16 g_button;
int dir_front[4][2] = {
    {0, 1},
    {1, 0},
    {0, -1},
    {-1, 0}
};
int dir_frontleft[4][2] = {
    {-1, -1},
    {1, -1},
    {1, 1},
    {-1, 1}
};
int dir_frontright[4][2] = {
    {1, -1},
    {1, 1},
    {-1, 1},
    {-1, -1}
};
// **************************** �������� ****************************

int core0_main(void)
{
    int Binary_Threshold = 120;
    int Binary_Mode = 0;
    int Search_Mode = 0;
    clock_init();                   // ��ȡʱ��Ƶ��<��ر���>
    debug_init();                   // ��ʼ��Ĭ�ϵ��Դ���'
    // �˴���д�û����� ���������ʼ�������

    gpio_init(KEY1, GPI, GPIO_HIGH, GPI_PULL_UP);           // ��ʼ�� KEY1 ���� Ĭ�ϸߵ�ƽ ��������
    gpio_init(KEY2, GPI, GPIO_HIGH, GPI_PULL_UP);           // ��ʼ�� KEY2 ���� Ĭ�ϸߵ�ƽ ��������
    gpio_init(KEY3, GPI, GPIO_HIGH, GPI_PULL_UP);           // ��ʼ�� KEY3 ���� Ĭ�ϸߵ�ƽ ��������
    gpio_init(KEY4, GPI, GPIO_HIGH, GPI_PULL_UP);           // ��ʼ�� KEY4 ���� Ĭ�ϸߵ�ƽ ��������

    // �˴���д�û����� ���������ʼ�������
    tft180_init();
    mt9v03x_init();
    key_init(10);
    cpu_wait_event_ready();         // �ȴ����к��ĳ�ʼ�����

    while (TRUE)
    {
        button_scan();
        switch(g_button){
            case BUTTON_RIGHT:
            {
                Binary_Mode += 1;
                Binary_Mode = Binary_Mode % 3;
                break;
            }
            case BUTTON_DOWN:
            {
                break;
            }
            case BUTTON_LEFT:
            {
                Search_Mode += 1;
                Search_Mode = Search_Mode % 1;
                break;
            }
            case BUTTON_UP:
            {
                Binary_Threshold += 5;
                Binary_Threshold = Binary_Threshold % 255;
                break;
            }
            default:
                break;
        }
        switch (2)
        {
            case 0:
            {
                tft180_show_string(0, 84, "GlobalB");
                tft180_show_int(90, 84, Binary_Threshold, 3);
                break;
            }
            case 1:
            {
                tft180_show_string(0, 84, "OTSUNor");
                Binary_Threshold = GetOTSU(mt9v03x_image[0]);
                tft180_show_int(90, 84, Binary_Threshold, 3);
                break;
            }
            case 2:
            {
                tft180_show_string(0, 84, "OTSUacc");
                Binary_Threshold = img_otsu_acc(mt9v03x_image[0]);
                tft180_show_int(90, 84, Binary_Threshold, 3);
                break;
            }
        }

        tft180_show_gray_image(0, 0, mt9v03x_image, MT9V03X_W, MT9V03X_H, 128 , 81, Binary_Threshold );
        switch (2)
        {
            case 0:
            {
                tft180_show_string(0, 100, "Longestwhite");
                Longestwhite(mt9v03x_image[0], Binary_Threshold);
                break;
            }
            case 1:
            {
                tft180_show_string(0, 100, "SimpleMiddle");
                SimpleMiddle(mt9v03x_image[0], Binary_Threshold);
                break;
            }
            case 2:
            {
                tft180_show_string(0, 100, "MiSearchLine");
                MiSearchLine(mt9v03x_image[0], Binary_Threshold);
                break;
            }
        }
        //����ǰհ��
        tft180_draw_line(0, 20, 127, 20, RGB565_GREEN);
        tft180_draw_line(0, 60, 127, 60, RGB565_GREEN);
        //������������
        tft180_draw_line(64, 0, 64, 81, RGB565_PURPLE);
        tft180_show_string(0, 116, "error:");
        tft180_show_float(70, 116, error, 2, 1);
    }
}
/*************************************************************************
 *  �������ƣ�MiSearchLine (uint8_t *img)
 *  ����˵�����Թ�ɨ��
 *  ����˵����img ��ͼ������
 *  �������أ���
*************************************************************************/
void MiSearchLine(uint8_t *img, int thred)
{
    int left_num=0;
    int right_num=0;
    RightSearchLine(img, &right_num, thred);
    LeftSearchLine(img, &left_num, thred);
    //ȡleft_num��right_num�н�С��һ��
    int num = left_num < right_num ? left_num : right_num;
    //�ö�̬�ڴ潨��������������
    int midpts[num][2];
    error = 0;
    for(int i=0; i<left_num; i++)
    {
        int temp_left_x = (int)((1.0*leftpts[i][0]/MT9V03X_W)*128);
        int temp_left_y = (int)((1.0*leftpts[i][1]/MT9V03X_H)*81);
        if(i<=num)
        {
            midpts[i][0] += temp_left_x;
            midpts[i][1] += temp_left_y;
        }
        tft180_draw_point(temp_left_x, temp_left_y, RGB565_BLUE);
    }
    for(int i=0; i<right_num; i++)
    {
        int temp_right_x = (int)((1.0*rightpts[i][0]/MT9V03X_W)*128);
        int temp_right_y = (int)((1.0*rightpts[i][1]/MT9V03X_H)*81);
        if(i<=num)
        {
            midpts[i][0] += temp_right_x;
            midpts[i][1] += temp_right_y;
            //ȡƽ��ֵ
            midpts[i][0] /= 2;
            midpts[i][1] /= 2;
            error += 0.1*(midpts[i][0]-64);
        }
        tft180_draw_point(temp_right_x, temp_right_y, RGB565_YELLOW);
        //��������
        tft180_draw_point(midpts[i][0], midpts[i][1], RGB565_RED);
    }
}
/*************************************************************************
 *  �������ƣ�RightSearchLine (uint8_t *img)
 *  ����˵���������Թ�ɨ��
 *  ����˵����img ��ͼ������
 *  �������أ���
*************************************************************************/
void RightSearchLine(uint8_t *img, int* step, int thred)
{
    int x;
    int y = MT9V03X_H-3;
    int Block_size = 5;
    int half = Block_size/2;
    int dir = 0;
    int turn = 0;

    for(int i = MT9V03X_W/2; i<MT9V03X_W-2; i++)
    {
        if(img[(MT9V03X_H-2) * MT9V03X_W + i] > thred && img[(MT9V03X_H-2) * MT9V03X_W + i+1]<thred)
        {
            x = i;
            break;
        }
    }
    //�ӱ߽翪ʼ�����Թ�ɨ��
    while(*step < 100 && turn < 4 && x > 0 && x <= MT9V03X_W-half && y > 0 && y < MT9V03X_H-1)
    {
        int local_thres = 0;
        for(int dy = -half; dy <= half; dy++){
            for(int dx = -half; dx <= half; dx++){
                local_thres += img[(y+dy) * MT9V03X_W + x + dx];
            }
        }
        local_thres /= Block_size*Block_size;
        local_thres -= 2;

        int current_value = img[y * MT9V03X_W + x];
        int front_value = img[(y + dir_front[dir][1]) * MT9V03X_W + x + dir_front[dir][0]];
        int frontright_value = img[(y + dir_frontright[dir][1]) * MT9V03X_W + x + dir_frontright[dir][0]];
        if(front_value < local_thres)//ǰ�����ϰ���
        {   
            dir = (dir + 3)%4;
            turn++;
        }
        else if (frontright_value < local_thres){//��ǰ�����ϰ���
            x += dir_front[dir][0];
            y += dir_front[dir][1];
            rightpts[*step][0] = x;
            rightpts[*step][1] = y;
            *step++;
            turn = 0;
        }else {//ǰ���Լ���ǰ�������ϰ���
            x += dir_frontright[dir][0];
            y += dir_frontright[dir][1];
            dir = (dir + 1)%4;
            rightpts[*step][0] = x;
            rightpts[*step][1] = y;
            *step++;
            turn = 0;
        }
    }
}
/*************************************************************************
 *  �������ƣ�LeftSearchLine (uint8_t *img)
 *  ����˵���������Թ�ɨ��
 *  ����˵����img ��ͼ������
 *  �������أ���
*************************************************************************/
void LeftSearchLine(uint8_t *img, int *step, int thred)
{
    int x =0;
    int y = MT9V03X_H-3;
    int Block_size = 5;
    int half = Block_size/2;
    int dir = 0;
    int turn = 0;

    for(int i = MT9V03X_W/2; i>1; i--)
    {
        if(img[(MT9V03X_H-2) * MT9V03X_W + i] > thred && img[(MT9V03X_H-2) * MT9V03X_W + i+1]<thred)
        {
            x = i;
            break;
        }
    }
    //�ӱ߽翪ʼ�����Թ�ɨ��
    while(*step < 100 && turn < 4 && x > 0 && x <= MT9V03X_W-half && y > 0 && y < MT9V03X_H-1)
    {
        int local_thres = 0;
        for(int dy = -half; dy <= half; dy++){
            for(int dx = -half; dx <= half; dx++){
                local_thres += img[(y+dy) * MT9V03X_W + x + dx];
            }
        }
        local_thres /= Block_size*Block_size;
        local_thres -= 2;

        int current_value = img[y * MT9V03X_W + x];
        int front_value = img[(y + dir_front[dir][1]) * MT9V03X_W + x + dir_front[dir][0]];
        int frontleft_value = img[(y + dir_frontleft[dir][1]) * MT9V03X_W + x + dir_frontleft[dir][0]];
        if(front_value < local_thres)//ǰ�����ϰ���
        {
            dir = (dir + 1)%4;
            turn++;
        }
        else if (frontleft_value < local_thres){//��ǰ�����ϰ���
            x += dir_front[dir][0];
            y += dir_front[dir][1];
            leftpts[*step][0] = x;
            leftpts[*step][1] = y;
            *step++;
            turn = 0;
        }else {//ǰ���Լ���ǰ�������ϰ���
            x += dir_frontleft[dir][0];
            y += dir_frontleft[dir][1];
            dir = (dir + 1)%4;
            leftpts[*step][0] = x;
            leftpts[*step][1] = y;
            *step++;
            turn = 0;
        }
    }
}
/*************************************************************************
 *  �������ƣ�SimpleMiddle (uint8_t *img, int th)
 *  ����˵�����������߷�
 *  ����˵����img ��ͼ�����ݣ�th����ֵ
 *  �������أ���
*************************************************************************/
void SimpleMiddle (uint8_t *img, int th)
{
    int MiddleLength = 0;
    //�����ж�
    int Left_Lost_Flag[100] = {0};
    int Right_Lost_Flag[100] = {0};
    for(int i=MT9V03X_H-1; i>0; i--)
    {
        if(img[i*MT9V03X_W + MT9V03X_W/2] > th)
        {
            MiddleLength++;
        }
        else
        {
            break;
        }
    }
    //�������������߽�
    int left_border = 0, right_border = 0;
    for(int i=MT9V03X_H-1; i>=MT9V03X_H-MiddleLength-1; i--)
    {
        for(int j = MT9V03X_W/2; j>=2; j--)
        {
            if(img[i*MT9V03X_W + j] > th && img[i*MT9V03X_W + j - 1] < th&& img[i*MT9V03X_W + j - 2] < th)
            {
                left_border = j;
                break;
            }else if(j<=2){
                left_border = 2;
                Left_Lost_Flag[i] = 1;
                break;
            }
        }
        for(int j = MT9V03X_W/2; j<=MT9V03X_W - 2; j++)
        {
            if(img[i*MT9V03X_W + j] > th && img[i*MT9V03X_W + j + 1] < th&& img[i*MT9V03X_W + j + 2] < th)
            {
                right_border = j;
                break;
            }else if(j>=MT9V03X_W - 2){
                right_border = MT9V03X_W - 2;
                Right_Lost_Flag[i] = 1;
                break;
            }
        }
        leftborders[i] = left_border;
        rightborders[i] = right_border;
    }
    //�������ұ����Լ�����
    for(int i=MT9V03X_H-1; i>=MT9V03X_H-MiddleLength-1; i--)
    {
        //�ж��Ƿ���
        if(Left_Lost_Flag[i]!=1 && Right_Lost_Flag[i]!=1)
        {
            //��Ҫ��������任����Ϊͼ�����ʾ�����겻һ��
            int temp_left = (int)((1.0*leftborders[i]/MT9V03X_W)*128);
            int temp_right = (int)((1.0*rightborders[i]/MT9V03X_W)*128);
            int temp_mid = (temp_left + temp_right)/2;
            int temp_i = (int)((1.0*i/MT9V03X_H)*81);
            error += 0.1*(temp_mid-64);
            //�ж������Ƿ񳬳���Χ
            if(temp_right <= 1)
            {
                temp_right = 1;
            }
            if(temp_right >= 127)
            {
                temp_right = 127;
            }
            if(temp_left <= 1)
            {
                temp_left = 1;
            }
            if(temp_left >= 127)
            {
                temp_left = 127;
            }
            if(temp_i <= 1)
            {
                temp_i = 1;
            }
            if(i < MT9V03X_H - 20)
            {
                tft180_draw_point(temp_left, temp_i, RGB565_BLUE);
                tft180_draw_point(temp_right, temp_i, RGB565_YELLOW);
            }
            tft180_draw_point(temp_mid, temp_i, RGB565_RED);
        }
    }
}
/*************************************************************************
 *  �������ƣ�Longestwhite (uint8_t *img, int th)
 *  ����˵������������߷�
 *  ����˵����img ��ͼ�����ݣ�th����ֵ
 *  �������أ���
*************************************************************************/
void Longestwhite(uint8_t *img, int th)
{
    int White_Column[MT9V03X_W] = {0};
    int start_column = 20, end_column = MT9V03X_W - 20;
    int search_stop_line = 0;
    int right_border = 0, left_border = 0;
    int Right_Lost_Flag[100] = {0};
    int Left_Lost_Flag[100] = {0};
    //�������ң��������ϣ�����ͼ��ͳ��ÿһ�еİ׵�����
    for(int i = start_column; i< end_column; i++)
    {
        for(int j = MT9V03X_H - 1; j > 0; j--)
        {
            if(img[j * MT9V03X_W + i] > th)
            {
                White_Column[i]++;
            }
            else
            {
                break;
            }
        }
    }
    int Longest_White_Column_left[2] = {0};//[0]Ϊ���г��ȣ�[1]Ϊ������ʼ��
    int Longest_White_Column_right[2] = {0};//[0]Ϊ���г��ȣ�[1]Ϊ������ʼ��
    //�������ұ������ҵ���İ���
    for(int i = start_column; i< end_column; i++)
    {
        if(White_Column[i] > Longest_White_Column_left[0])
        {
            Longest_White_Column_left[0] = White_Column[i];
            Longest_White_Column_left[1] = i;
        }
    }
    //��������������ҵ���İ���
    for(int i = end_column; i> Longest_White_Column_left[1]; i--)
    {
        if(White_Column[i] > Longest_White_Column_right[0])
        {
            Longest_White_Column_right[0] = White_Column[i];
            Longest_White_Column_right[1] = i;
        }
    }
    search_stop_line = Longest_White_Column_left[0];
    //Ѳ��

    for(int i = MT9V03X_H - 1; i>=MT9V03X_H - search_stop_line; i--)
    {
        for(int j = Longest_White_Column_right[1]; j <= MT9V03X_W - 1 - 2; j++)
        {
            if(img[i * MT9V03X_W + j] > th && img[i * MT9V03X_W + j + 1] < th && img[i * MT9V03X_W + j + 2] < th)
            {
                right_border = j;
                Right_Lost_Flag[i] = 0;
                break;
            }else if(j>=MT9V03X_W - 1 - 2){
                right_border = MT9V03X_W - 1;
                Right_Lost_Flag[i] = 1;
                break;
            }
        }
        for(int j = Longest_White_Column_left[1]; j >= 2; j--)
        {
            if(img[i * MT9V03X_W + j] > th && img[i * MT9V03X_W + j - 1] < th && img[i * MT9V03X_W + j - 2] < th)
            {
                left_border = j;
                Left_Lost_Flag[i] = 0;
                break;
            }else if(j<=2){
                left_border = 0;
                Left_Lost_Flag[i] = 1;
                break;
            }
        }
        leftborders[i] = left_border;
        rightborders[i] = right_border;
    }
    error = 0;
    //�������ұ����Լ�����
    for(int i = MT9V03X_H -1; i>=MT9V03X_H - search_stop_line; i--)
    {
        //�ж��Ƿ���
        if(Right_Lost_Flag[i] != 1 && Left_Lost_Flag[i] != 1)
        {
            //��Ҫ��������任����Ϊͼ�����ʾ�����겻һ��
            int temp_left = (int)((1.0*leftborders[i]/MT9V03X_W)*128);
            int temp_right = (int)((1.0*rightborders[i]/MT9V03X_W)*128);
            int temp_mid = (temp_left + temp_right)/2;
            error += 0.1*(temp_mid-64);
            int temp_i = (int)((1.0*i/MT9V03X_H)*81);
            //�ж������Ƿ񳬳���Χ
            if(temp_right <= 1)
            {
                temp_right = 1;
            }
            if(temp_right >= 127)
            {
                temp_right = 127;
            }
            if(temp_left <= 1)
            {
                temp_left = 1;
            }
            if(temp_left >= 127)
            {
                temp_left = 127;
            }
            if(temp_i <= 1)
            {
                temp_i = 1;
            }
            if(i < MT9V03X_H - 20)
            {
                tft180_draw_point(temp_left, temp_i, RGB565_BLUE);
                tft180_draw_point(temp_right, temp_i, RGB565_YELLOW);
            }
            tft180_draw_point(temp_mid, temp_i, RGB565_RED);
        }
    }
}
/*************************************************************************
 *  �������ƣ�short GetOTSU (uint8_t *img)
 *  ����˵�����������ֵ��С
 *  ����˵����img �� ͼ������
 *  �������أ���ֵ
*************************************************************************/
short GetOTSU (uint8_t *img)
{
    signed short i, j;
    unsigned long Amount = 0;
    unsigned long PixelBack = 0;
    unsigned long PixelshortegralBack = 0;
    unsigned long Pixelshortegral = 0;
    signed long PixelshortegralFore = 0;
    signed long PixelFore = 0;
    float OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma; // ��䷽��;
    signed short MinValue, MaxValue;
    signed short Threshold = 0;
    unsigned short HistoGram[256] = {0};     //ԭ��Ϊunsigned char ������ͬһ���Ҷ�ֵ�����ص���ܻᳬ��255��;
    SigmaB = -1;

    for (j = 0; j < MT9V03X_H; j++)
    {
        for (i = 0; i < MT9V03X_W; i++)
        {
            HistoGram[*(img+ j * MT9V03X_W + i)]++; //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
            MinValue = ((*(img+ j * MT9V03X_W + i)) < MinValue) ? (*(img+ j * MT9V03X_W + i)) : MinValue; //ͳ����С�Ҷ�ֵ
            MaxValue = ((*(img+ j * MT9V03X_W + i)) > MaxValue) ? (*(img+ j * MT9V03X_W + i)) : MaxValue; //ͳ�����Ҷ�ֵ
        }
    }

    for (j = MinValue; j <= MaxValue; j++)
    {
        Amount += HistoGram[j];        //  ��������
        Pixelshortegral += HistoGram[j] * j;        //�Ҷ�ֵ����
    }

    for (j = MinValue+10; j < MaxValue-10; j++)
    {
        PixelBack = PixelBack + HistoGram[j];     //ǰ�����ص���
        PixelFore = Amount - PixelBack;           //�������ص���
        OmegaBack = (float) PixelBack / Amount;   //ǰ�����ذٷֱ�
        OmegaFore = (float) PixelFore / Amount;   //�������ذٷֱ�
        PixelshortegralBack += HistoGram[j] * j;  //ǰ���Ҷ�ֵ
        PixelshortegralFore = Pixelshortegral - PixelshortegralBack;  //�����Ҷ�ֵ
        MicroBack = (float) PixelshortegralBack / PixelBack;   //ǰ���ҶȰٷֱ�
        MicroFore = (float) PixelshortegralFore / PixelFore;   //�����ҶȰٷֱ�
        Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);   //������䷽��
        if (Sigma > SigmaB)                    //����������䷽��g //�ҳ������䷽���Լ���Ӧ����ֵ
        {
            SigmaB = Sigma;
            Threshold = j;
        }
    }
    return Threshold;                        //���������ֵ;
}
/*************************************************************************
 *  �������ƣ�img_otsu_acc(uint8_t *img)
 *  ����˵�������ٴ������ֵ��С
 *  ����˵����img
 *  �������أ���ֵ
*************************************************************************/
int img_otsu_acc(uint8_t *img)
{
    uint8_t grayhist[64] = {0};          //�Ҷ�ֱ��ͼ
    uint16_t px_sum_all = 0;             //���ص�����
    uint32_t gray_sum_all = 0;           //�ܻҶȻ���
    uint16_t px_sum = 0;                 //���ص�����
    uint32_t gray_sum = 0;               //�ҶȻ���
    float fCal_var;

    //���ɣ�1. �Ҷ�ֱ��ͼ 2. ���ص����� 3. �ܻҶȻ���
    for (int i = 0; i < MT9V03X_W; i += 10)
    {
        int temp_this_pixel;
        for (int j = 0; j < MT9V03X_H; j += 10)
        {
            temp_this_pixel = img[i * MT9V03X_H + j] /4;
            grayhist[temp_this_pixel]++;
            gray_sum_all += temp_this_pixel;
            px_sum_all++;
        }
    }
    //������������䷽�����ֵ
    float fTemp_maxvar = 0;
    uint8_t temp_best_th = 0;
    float u0, u1, w0, w1;
    for (uint8_t k = 0; k < 64; k++)
    {
        px_sum += grayhist[k];       //�ûҶȼ����µ����ص�����
        gray_sum += k * grayhist[k]; //�ûҶȼ����µ����ص�ĻҶȺ�
        w0 = 1.0 * px_sum / px_sum_all;
        w1 = 1.0 - w0;
        u0 = 1.0 * gray_sum / px_sum;
        u1 = 1.0 * (gray_sum_all - gray_sum) / (px_sum_all - px_sum);
        fCal_var = w0 * w1 * (u0 - u1) * (u0 - u1);
        if (fCal_var > fTemp_maxvar)
        {
            fTemp_maxvar = fCal_var;
            temp_best_th = k;
        }
    }
    return temp_best_th  * 4;
}
void button_scan(void)
{
    static uint8_t key = 0;
    key <<= 4;

    if(!gpio_get_level(KEY1))
    {
        key += BUTTON_UP;
    }
    else if(!gpio_get_level(KEY2))
    {
        key += BUTTON_DOWN;
    }
    else if(!gpio_get_level(KEY3))
    {
        key += BUTTON_LEFT;
    }
   else if(!gpio_get_level(KEY4))
   {
       key += BUTTON_RIGHT;
   }
    //����λ��ֵ�Ļ��Ͳ����,����λ��ֵ�������Ӧ��ֵ
    g_button = (key&0x0f)? BUTTON_NONE: (key>>4);
}
#pragma section all restore
// **************************** �������� ****************************

// *************************** ���̳�������˵�� ***************************
// ��������ʱ�밴�������������б���
// ����1��LED ����˸
//      ���ʹ��������ԣ��������Ҫ�õ�ع���
//      �鿴�����Ƿ�������¼���Ƿ����ر���ȷ���������¸�λ����
//      ���ñ������Ӧ LED ���ŵ�ѹ�Ƿ�仯��������仯֤������δ���У�����仯֤�� LED ������
// ����2��SWITCH1 / SWITCH2 ���������ˮ��Ƶ���ޱ仯
//      ���ʹ��������ԣ��������Ҫ�õ�ع���
//      �鿴�����Ƿ�������¼���Ƿ����ر���ȷ���������¸�λ����
//      ���ñ������Ӧ LED ���ŵ�ѹ�Ƿ�仯��������仯֤������δ���У�����仯֤�� LED ������
//      ���ñ����Ӧ SWITCH1 / SWITCH2 ���ŵ�ѹ�Ƿ������仯���Ƿ�������źŲ����������Ƿ�Ӵ�
// ����2��KEY1 / KEY2 / KEY3 / KEY4 ��GND���߰��������ޱ仯
//      ���ʹ��������ԣ��������Ҫ�õ�ع���
//      �鿴�����Ƿ�������¼���Ƿ����ر���ȷ���������¸�λ����
//      ���ñ������Ӧ LED ���ŵ�ѹ�Ƿ�仯��������仯֤������δ���У�����仯֤�� LED ������
//      ���ñ����Ӧ KEY1 / KEY2 / KEY3 / KEY4 ���ŵ�ѹ�Ƿ������仯���Ƿ�������źŲ����������Ƿ�Ӵ�

