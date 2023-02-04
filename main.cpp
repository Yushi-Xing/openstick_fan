#include <iostream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <pthread.h>
#include <chrono>
#include <unistd.h>
#include <functional>
//
#include "CTimer.h"

//使用绿色led控制引脚，更换引脚需更改36、139，140，141行

using namespace std;

#define CMD_RESULT_MAX_LENGTH 1024
#define Interval 1000 //回调时间间隔 ms

void getnetbytes(long int savebuffer[]);
std::string excute_cmd(const char* cmd);
void *netspeed_temper(void* args);


int main(int argc,char** argv)
{
    
   


    pthread_t tid[1];
    int ret = pthread_create(&tid[0], NULL, netspeed_temper, NULL);
        if (ret != 0)
    {
        cout << "pthread_create error: error_code=" << ret << endl;
    }    
    char ledtrigger[]="echo timer > /sys/class/leds/green:internet/trigger";
    std::string res=excute_cmd(ledtrigger);


    getchar();//主函数阻塞
    return 0;
}




std::string excute_cmd(const char* cmd) 
{
    FILE* fp = popen(cmd, "r");
    if (nullptr == fp) {
        return "";
    }
    char result[CMD_RESULT_MAX_LENGTH] = {0};
 
    for (int i = 0; i < CMD_RESULT_MAX_LENGTH - 1 && fgets(result + i, CMD_RESULT_MAX_LENGTH - i, fp) != nullptr; i = strlen(result)) {
    }
    pclose(fp);
    fp = nullptr;
    return result;
}


void getnetbytes(long int savebuffer[])
{
    
    string res=excute_cmd("cat /proc/net/dev|grep \"wlan0\"|awk '{print $2}'");
    savebuffer[0]=atoi(res.c_str());

    string res2=excute_cmd("cat /proc/net/dev|grep \"wlan0\"|awk '{print $10}'");
    savebuffer[1]=atoi(res2.c_str());

}
void backCallFunc();
//every seconds run once 
CTimer timer;
void *netspeed_temper(void* args)
{
    timer.start(Interval, std::bind(backCallFunc));
    getchar();//线程阻塞
    return 0;
}


//回调函数
void backCallFunc()
{
	long int net_total[2]={0};
    unsigned char temps[4]={0};
    static long int last_net_total[2]={0};
    static unsigned char last_temps[4]={0};

    char cmd_temp[]="cat /sys/class/thermal/thermal_zone0/temp";
    int sum=0;
    for(int i=0;i<4;i++)
    {
        cmd_temp[35]=*std::to_string(i).c_str();
        string temp=excute_cmd(cmd_temp);
        temps[i]=atoi(temp.c_str())/1000;
        sum+=temps[i];
        //std::cout<<temp<<std::endl;
    }
    float average_temp=sum/4.0;
    std::cout<<average_temp<<std::endl;


    getnetbytes(net_total);//获取为wlan0的上下行流量

    short int netrxspeed=((net_total[0]-last_net_total[0])/1024)/(Interval/1000);
    short int nettxspeed=((net_total[1]-last_net_total[1])/1024)/(Interval/1000);


    std::cout<<netrxspeed<<"KB/S | "<<nettxspeed<< "KB/S"<<std::endl;

    for(int i=0;i<2;i++)
    {
        last_net_total[i]=net_total[i];
        //last_temps[i]=temps[i];
    }

    //绿色led引脚占空比调节
    char cmd_temp1[128],cmd_temp2[128];

    
    //50度-25度之间划分十份
    int PWM_level=(int)(average_temp-25)/2.5;
    if (PWM_level<0)
        PWM_level=0;
    else if (PWM_level>10)
        PWM_level=10;
    static int last_PWM_level;
    
    // sprintf(cmd_temp1,"echo %d > /sys/devices/platform/leds/leds/green:internet/delay_off",(10-PWM_level)*500);
    // sprintf(cmd_temp2,"echo %d > /sys/devices/platform/leds/leds/green:internet/delay_on",(PWM_level)*500);
    // cout<<cmd_temp1<<"\n"<<cmd_temp2<<endl;

    if (PWM_level!=last_PWM_level)
    {
    if(PWM_level>3)
    {
    sprintf(cmd_temp1,"echo %d > /sys/devices/platform/leds/leds/green:internet/delay_off",5-PWM_level/2);
    sprintf(cmd_temp2,"echo %d > /sys/devices/platform/leds/leds/green:internet/delay_on",PWM_level*5);
    excute_cmd("echo 50000 > /sys/devices/platform/leds/leds/green:internet/delay_on");
    cout<<cmd_temp1<<"\n"<<cmd_temp2<<endl;
    //看风扇体制
    usleep(1000000);//1s助力风扇启动
    excute_cmd(cmd_temp1);
    excute_cmd(cmd_temp2);
    }
    else
    {
    sprintf(cmd_temp1,"echo %d > /sys/devices/platform/leds/leds/green:internet/delay_off",2);
    sprintf(cmd_temp2,"echo %d > /sys/devices/platform/leds/leds/green:internet/delay_on",6);
    excute_cmd("echo 50000 > /sys/devices/platform/leds/leds/green:internet/delay_on");
    usleep(1000000);//1s助力风扇启动
    cout<<cmd_temp1<<"\n"<<cmd_temp2<<endl;
    excute_cmd(cmd_temp1);
    excute_cmd(cmd_temp2);

    }

    }
    

    last_PWM_level=PWM_level;
    


   
    
}