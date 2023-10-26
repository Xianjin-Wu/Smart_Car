/*********************************************************************************************************************
* TC264 Opensourec Library 即（TC264 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC264 开源库的一部分
*
* TC264 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          cpu0_main
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          ADS v1.8.0
* 适用平台          TC264D
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2022-09-15       pudding            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#pragma section all "cpu0_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中


// 工程导入到软件之后，应该选中工程然后点击refresh刷新一下之后再编译
// 工程默认设置为关闭优化，可以自己右击工程选择properties->C/C++ Build->Setting
// 然后在右侧的窗口中找到C/C++ Compiler->Optimization->Optimization level处设置优化等级
// 一般默认新建立的工程都会默认开2级优化，因此大家也可以设置为2级优化

// 对于TC系列默认是不支持中断嵌套的，希望支持中断嵌套需要在中断内使用 interrupt_global_enable(); 来开启中断嵌套
// 简单点说实际上进入中断后TC系列的硬件自动调用了 interrupt_global_disable(); 来拒绝响应任何的中断，因此需要我们自己手动调用 interrupt_global_enable(); 来开启中断的响应。

// *************************** 例程硬件连接说明 ***************************
// 核心板正常供电即可 无需额外连接
// 如果使用主板测试 主板必须要用电池供电


// *************************** 例程测试说明 ***************************
// 1.核心板烧录完成本例程，完成上电
// 2.可以看到核心板上四个 LED 呈流水灯状闪烁
// 3.将 SWITCH1 / SWITCH2 两个宏定义对应的引脚分别按照 00 01 10 11 的组合接到 1-VCC 0-GND 或者波动对应主板的拨码开关
// 3.不同的组合下，四个 LED 流水灯状闪烁的频率会发生变化
// 4.将 KEY1 / KEY2 / KEY3 / KEY4 两个宏定义对应的引脚接到 1-VCC 0-GND 或者 按对应按键
// 5.任意引脚接 GND 或者 按键按下会使得所有LED一起闪烁 松开后恢复流水灯
// 如果发现现象与说明严重不符 请参照本文件最下方 例程常见问题说明 进行排查

// **************************** 代码区域 ****************************
#define KEY1                    (P11_3)
#define KEY2                    (P11_2)
#define KEY3                    (P20_7)
#define KEY4                    (P20_6)

//按键键值
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
// **************************** 代码区域 ****************************

int core0_main(void)
{
    int Binary_Threshold = 120;
    int Binary_Mode = 0;
    int Search_Mode = 0;
    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();                   // 初始化默认调试串口'
    // 此处编写用户代码 例如外设初始化代码等

    gpio_init(KEY1, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY1 输入 默认高电平 上拉输入
    gpio_init(KEY2, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY2 输入 默认高电平 上拉输入
    gpio_init(KEY3, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY3 输入 默认高电平 上拉输入
    gpio_init(KEY4, GPI, GPIO_HIGH, GPI_PULL_UP);           // 初始化 KEY4 输入 默认高电平 上拉输入

    // 此处编写用户代码 例如外设初始化代码等
    tft180_init();
    mt9v03x_init();
    key_init(10);
    cpu_wait_event_ready();         // 等待所有核心初始化完毕

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
        //绘制前瞻行
        tft180_draw_line(0, 20, 127, 20, RGB565_GREEN);
        tft180_draw_line(0, 60, 127, 60, RGB565_GREEN);
        //绘制运行中线
        tft180_draw_line(64, 0, 64, 81, RGB565_PURPLE);
        tft180_show_string(0, 116, "error:");
        tft180_show_float(70, 116, error, 2, 1);
    }
}
/*************************************************************************
 *  函数名称：MiSearchLine (uint8_t *img)
 *  功能说明：迷宫扫线
 *  参数说明：img ：图像数据
 *  函数返回：无
*************************************************************************/
void MiSearchLine(uint8_t *img, int thred)
{
    int left_num=0;
    int right_num=0;
    RightSearchLine(img, &right_num, thred);
    LeftSearchLine(img, &left_num, thred);
    //取left_num和right_num中较小的一个
    int num = left_num < right_num ? left_num : right_num;
    //用动态内存建立中线坐标数组
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
            //取平均值
            midpts[i][0] /= 2;
            midpts[i][1] /= 2;
            error += 0.1*(midpts[i][0]-64);
        }
        tft180_draw_point(temp_right_x, temp_right_y, RGB565_YELLOW);
        //绘制中线
        tft180_draw_point(midpts[i][0], midpts[i][1], RGB565_RED);
    }
}
/*************************************************************************
 *  函数名称：RightSearchLine (uint8_t *img)
 *  功能说明：右手迷宫扫线
 *  参数说明：img ：图像数据
 *  函数返回：无
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
    //从边界开始进行迷宫扫线
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
        if(front_value < local_thres)//前方有障碍物
        {   
            dir = (dir + 3)%4;
            turn++;
        }
        else if (frontright_value < local_thres){//右前方有障碍物
            x += dir_front[dir][0];
            y += dir_front[dir][1];
            rightpts[*step][0] = x;
            rightpts[*step][1] = y;
            *step++;
            turn = 0;
        }else {//前方以及右前方均无障碍物
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
 *  函数名称：LeftSearchLine (uint8_t *img)
 *  功能说明：左手迷宫扫线
 *  参数说明：img ：图像数据
 *  函数返回：无
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
    //从边界开始进行迷宫扫线
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
        if(front_value < local_thres)//前方有障碍物
        {
            dir = (dir + 1)%4;
            turn++;
        }
        else if (frontleft_value < local_thres){//左前方有障碍物
            x += dir_front[dir][0];
            y += dir_front[dir][1];
            leftpts[*step][0] = x;
            leftpts[*step][1] = y;
            *step++;
            turn = 0;
        }else {//前方以及左前方均无障碍物
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
 *  函数名称：SimpleMiddle (uint8_t *img, int th)
 *  功能说明：中线搜线法
 *  参数说明：img ：图像数据，th：阈值
 *  函数返回：无
*************************************************************************/
void SimpleMiddle (uint8_t *img, int th)
{
    int MiddleLength = 0;
    //丢线判断
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
    //向左向右搜索边界
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
    //绘制左右边线以及中线
    for(int i=MT9V03X_H-1; i>=MT9V03X_H-MiddleLength-1; i--)
    {
        //判断是否丢线
        if(Left_Lost_Flag[i]!=1 && Right_Lost_Flag[i]!=1)
        {
            //需要进行坐标变换，因为图像和显示屏坐标不一致
            int temp_left = (int)((1.0*leftborders[i]/MT9V03X_W)*128);
            int temp_right = (int)((1.0*rightborders[i]/MT9V03X_W)*128);
            int temp_mid = (temp_left + temp_right)/2;
            int temp_i = (int)((1.0*i/MT9V03X_H)*81);
            error += 0.1*(temp_mid-64);
            //判断坐标是否超出范围
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
 *  函数名称：Longestwhite (uint8_t *img, int th)
 *  功能说明：最长白列搜线法
 *  参数说明：img ：图像数据，th：阈值
 *  函数返回：无
*************************************************************************/
void Longestwhite(uint8_t *img, int th)
{
    int White_Column[MT9V03X_W] = {0};
    int start_column = 20, end_column = MT9V03X_W - 20;
    int search_stop_line = 0;
    int right_border = 0, left_border = 0;
    int Right_Lost_Flag[100] = {0};
    int Left_Lost_Flag[100] = {0};
    //从左往右，从下往上，遍历图像，统计每一列的白点数量
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
    int Longest_White_Column_left[2] = {0};//[0]为白列长度，[1]为白列起始点
    int Longest_White_Column_right[2] = {0};//[0]为白列长度，[1]为白列起始点
    //从左往右遍历，找到最长的白列
    for(int i = start_column; i< end_column; i++)
    {
        if(White_Column[i] > Longest_White_Column_left[0])
        {
            Longest_White_Column_left[0] = White_Column[i];
            Longest_White_Column_left[1] = i;
        }
    }
    //从右往左遍历，找到最长的白列
    for(int i = end_column; i> Longest_White_Column_left[1]; i--)
    {
        if(White_Column[i] > Longest_White_Column_right[0])
        {
            Longest_White_Column_right[0] = White_Column[i];
            Longest_White_Column_right[1] = i;
        }
    }
    search_stop_line = Longest_White_Column_left[0];
    //巡线

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
    //绘制左右边线以及中线
    for(int i = MT9V03X_H -1; i>=MT9V03X_H - search_stop_line; i--)
    {
        //判断是否丢线
        if(Right_Lost_Flag[i] != 1 && Left_Lost_Flag[i] != 1)
        {
            //需要进行坐标变换，因为图像和显示屏坐标不一致
            int temp_left = (int)((1.0*leftborders[i]/MT9V03X_W)*128);
            int temp_right = (int)((1.0*rightborders[i]/MT9V03X_W)*128);
            int temp_mid = (temp_left + temp_right)/2;
            error += 0.1*(temp_mid-64);
            int temp_i = (int)((1.0*i/MT9V03X_H)*81);
            //判断坐标是否超出范围
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
 *  函数名称：short GetOTSU (uint8_t *img)
 *  功能说明：大津法求阈值大小
 *  参数说明：img ： 图像数据
 *  函数返回：阈值
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
    float OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma; // 类间方差;
    signed short MinValue, MaxValue;
    signed short Threshold = 0;
    unsigned short HistoGram[256] = {0};     //原先为unsigned char ，但是同一个灰度值的像素点可能会超过255个;
    SigmaB = -1;

    for (j = 0; j < MT9V03X_H; j++)
    {
        for (i = 0; i < MT9V03X_W; i++)
        {
            HistoGram[*(img+ j * MT9V03X_W + i)]++; //统计灰度级中每个像素在整幅图像中的个数
            MinValue = ((*(img+ j * MT9V03X_W + i)) < MinValue) ? (*(img+ j * MT9V03X_W + i)) : MinValue; //统计最小灰度值
            MaxValue = ((*(img+ j * MT9V03X_W + i)) > MaxValue) ? (*(img+ j * MT9V03X_W + i)) : MaxValue; //统计最大灰度值
        }
    }

    for (j = MinValue; j <= MaxValue; j++)
    {
        Amount += HistoGram[j];        //  像素总数
        Pixelshortegral += HistoGram[j] * j;        //灰度值总数
    }

    for (j = MinValue+10; j < MaxValue-10; j++)
    {
        PixelBack = PixelBack + HistoGram[j];     //前景像素点数
        PixelFore = Amount - PixelBack;           //背景像素点数
        OmegaBack = (float) PixelBack / Amount;   //前景像素百分比
        OmegaFore = (float) PixelFore / Amount;   //背景像素百分比
        PixelshortegralBack += HistoGram[j] * j;  //前景灰度值
        PixelshortegralFore = Pixelshortegral - PixelshortegralBack;  //背景灰度值
        MicroBack = (float) PixelshortegralBack / PixelBack;   //前景灰度百分比
        MicroFore = (float) PixelshortegralFore / PixelFore;   //背景灰度百分比
        Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);   //计算类间方差
        if (Sigma > SigmaB)                    //遍历最大的类间方差g //找出最大类间方差以及对应的阈值
        {
            SigmaB = Sigma;
            Threshold = j;
        }
    }
    return Threshold;                        //返回最佳阈值;
}
/*************************************************************************
 *  函数名称：img_otsu_acc(uint8_t *img)
 *  功能说明：加速大津法求阈值大小
 *  参数说明：img
 *  函数返回：阈值
*************************************************************************/
int img_otsu_acc(uint8_t *img)
{
    uint8_t grayhist[64] = {0};          //灰度直方图
    uint16_t px_sum_all = 0;             //像素点总数
    uint32_t gray_sum_all = 0;           //总灰度积分
    uint16_t px_sum = 0;                 //像素点数量
    uint32_t gray_sum = 0;               //灰度积分
    float fCal_var;

    //生成：1. 灰度直方图 2. 像素点总数 3. 总灰度积分
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
    //迭代求得最大类间方差的阈值
    float fTemp_maxvar = 0;
    uint8_t temp_best_th = 0;
    float u0, u1, w0, w1;
    for (uint8_t k = 0; k < 64; k++)
    {
        px_sum += grayhist[k];       //该灰度及以下的像素点数量
        gray_sum += k * grayhist[k]; //该灰度及以下的像素点的灰度和
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
    //低四位有值的话就不输出,低四位无值则输出相应键值
    g_button = (key&0x0f)? BUTTON_NONE: (key>>4);
}
#pragma section all restore
// **************************** 代码区域 ****************************

// *************************** 例程常见问题说明 ***************************
// 遇到问题时请按照以下问题检查列表检查
// 问题1：LED 不闪烁
//      如果使用主板测试，主板必须要用电池供电
//      查看程序是否正常烧录，是否下载报错，确认正常按下复位按键
//      万用表测量对应 LED 引脚电压是否变化，如果不变化证明程序未运行，如果变化证明 LED 灯珠损坏
// 问题2：SWITCH1 / SWITCH2 更改组合流水灯频率无变化
//      如果使用主板测试，主板必须要用电池供电
//      查看程序是否正常烧录，是否下载报错，确认正常按下复位按键
//      万用表测量对应 LED 引脚电压是否变化，如果不变化证明程序未运行，如果变化证明 LED 灯珠损坏
//      万用表检查对应 SWITCH1 / SWITCH2 引脚电压是否正常变化，是否跟接入信号不符，引脚是否接错
// 问题2：KEY1 / KEY2 / KEY3 / KEY4 接GND或者按键按下无变化
//      如果使用主板测试，主板必须要用电池供电
//      查看程序是否正常烧录，是否下载报错，确认正常按下复位按键
//      万用表测量对应 LED 引脚电压是否变化，如果不变化证明程序未运行，如果变化证明 LED 灯珠损坏
//      万用表检查对应 KEY1 / KEY2 / KEY3 / KEY4 引脚电压是否正常变化，是否跟接入信号不符，引脚是否接错

