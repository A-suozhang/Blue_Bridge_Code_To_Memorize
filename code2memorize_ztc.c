// #include “intrins.h” //如果需要用__nop__()，需要include这个
// #include "stc15f2k6s2"  可以省事，不用定义AUXR
// define LED P0 
// define BUFFER P3  for key_scan()

// --------------------------------------------------Fracture----------------------------------------------- //
#define uchar unsigned char;
#define uint  unsigned int;


sfr AUXR = 0x8E;    //AUXR地址是AUXR
// AUXR |= 0x80;	//1T模式，IAP15F2K61S2单片机特殊功能寄存器
// 0x80  1T模式

void cls_buzz()	   //关闭蜂鸣器
 {
  P2=(P2&0x1f)|0xa0;
  P0 =0x00;		//全都关
  P2&=0x1f;
 }
						 
void cls_led()		//关闭LED
{
	P2 = ((P2&0x1f)|0x80); 
	P0 = 0xFF;  
	P2 &= 0x1f;
}

void open_relay()
{
	P2 = (P2&0x1f)|0xa0;
	P0 |= 0X10;//&=0xEF     // 吸和继电器 |=0X10（灯亮）
	P2 &= 0X1F;
}
						 
						 
void lit_l1(){		//点亮LED1
		P2=(P2&0x1f)|0x80;
		P0=0XFE;
		P2&=0x1f;
}

// -----------------------------------------------Delay--------------------------------------------------- //

void Delay_1_ms(unsigned int n)//@11.0592MHz
{
   unsigned char i,j;
   for(i = 0;i > n;i--)
   {
      _nop_();
      _nop_();
      _nop_();
      i = 11;
      j = 190;
      do
      {
         while (--j);
      } while (--i);
   }
}


// -----------------------------------------------------Key------------------------------------------- //

//如果include<reg52.h>的话要这么写
sfr P4 = 0xc0;//reg52.h中没有定义P4寄存器故我们自己来定义
//转接板的P42,44代替本来应该接的P36,37



#define LED P0  //宏定义，在程序中可用LED代替P0使用
#define BUFFER P3  //宏定义，在程序中用BUFFER代替P3端口
 
sbit P3_6 = P4^2;//位定义用 P3_6 在程序中替换 P4^2的功能
sbit P3_7 = P4^4;//同上
//用这个代码之前要记得给生成的延迟函数加一个 unsigned int n的传入参数


// 记得这个keyscan要放在中断里面，display也可以放在中断里面
void keyscan()
{
   BUFFER = 0X0F; P3_6 = 0; P3_7 = 0;
   /*首先我们进行列扫描，将P34~P37都置零，其中要注意，开发板上用
   P42、P44替换了P36、P37故我们要单独将这两个IO置零*/
   if(BUFFER != 0X0F)// “!=”表示档P3不等于0x0f时条件为真
   {
      Delay1ms(10); //延时消抖
      if(BUFFER != 0X0F)//再次判断
      {
         switch(BUFFER)
         /*当按键按下行中值发生变化得到一个BUFFER，
         生成一个新的标志位*/
         {
            case 0X07: key_value = 1; break;
            case 0X0B: key_value = 5; break;
            case 0X0D: key_value = 9; break;
            case 0X0E: key_value = 13; break;
         }
 
         BUFFER = 0XF0; P3_6 = 1; P3_7 = 1;
         /*我们在进行行扫描，原理同上下方进行判断第几行
         按键按下，结合列标志位生成新的行列标志位。*/
         if(P3_7 == 0) key_value += 0; 
         if(P3_6 == 0) key_value += 1; 
         if(BUFFER == 0XD0) key_value += 2; 
         if(BUFFER == 0XE0) key_value += 3; 
      }
   }
   else{
	key_value = 0;
   }
}



// Debounce
if (key_flag == 2){
	// Do Something
	Delay_1_ms(10);//延时消抖，去除按键机械抖动带来的干扰因素
	while(key_flag == 2)
}

// --------------------------------------------------Seg Display---------------------------------------------- //

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xff,0xbf};  //LED 0~9，灭，-
uchar dspbuf[]={10,10,10,10,10,10,10,10};	//数码管
uchar dscom=0;	//数码管index


//本质是开一个定时器中断，可以用你妈烧写软件生成

void display()
{
 	P2=(P2&0x1f)|0xe0;
	P0=0xff;				//数码管消隐
	P2&=0x1f;

	P2=(P2&0x1f)|0xc0;
	P0=1<<dscom;
	P2&=0x1f;

	P2=(P2&0x1f)|0xe0;
	P0=tab[dspbuf[dscom]];
	P2&=0x1f;

	if(++dscom==8)dscom=0;
 
}

// ---------------------------------------------INTERUPTION----------------------------------------- //

//可以用软件生成
void Timer0Init(void)//5毫秒  @11.0592MHz
{
   AUXR |= 0x80;//定时器时钟1T模式
   TMOD &= 0xF0;//设置定时器模式
   TL0 = 0x00;//设置定时初值
   TH0 = 0x28;//设置定时初值
   TF0 = 0;//清除TF0标志
   TR0 = 1;//定时器0开始计时
}

//定时器中断服务函数
void isr_timer_0(void)  interrupt 1  //默认中断优先级 1
{     
	display();
	if(++intr == 200)  //1s执行一次
	{
        intr = 0;  //很关键
		//Do Something
    }
}

//  !  使用定时器中断之后，在main函数中除了需要调用 Timer0Init
//     还要写

EA = 1;  //定时器使能
ET0 = 1; //定时器0开启
//ET1 = 1; 如果用了两个定时器则也需要激活定时器1

void Timer0_int() interrupt 3 using 0
{
	//DO Sonething
}


// -------------------------------------------------IIC---------------------------------------------------- //
// IIC可以定义一个Delay5us替换其中的somenop
// 还有一个Delay10ms用来在操作中延时
// 用于驱动PCF（AD） 或EEPROM （AT）  

//PCF8591初始化  输入为Channel数
void ADC_Init(uchar chanel)  //Cahnnel Being like 0x03...
{
   IIC_Start();      //IIC启动信号
   IIC_SendByte(0x90);  //1001 000 0;选中该器件（前7位是地址，最后一位是R/~W，表示写）
   IIC_WaitAck();  //等待应答
   IIC_SendByte(chanel);//通过IIC发送通道
   IIC_WaitAck();  //等待应答
   IIC_Stop();  //IIC停止信号
   Delay10ms();      //等待10ms
}

//读取PCF8591
uchar ADC_Read()
{
   uchar temp;
   IIC_Start();     //IIC启动信号
   IIC_SendByte(0x91); //1001 000 1;选中该器件（前7位是地址，最后一位是R/~W,表示读）
   IIC_WaitAck(); //等待应答
   temp=IIC_RecByte(); //通过IIC读取采集的值
   IIC_Ack(0); //通过IIC发送应答
   IIC_Stop(); //IIC停止信号
   return temp;
}



// ------------------------------------------
// 需要一个delay10ms
//Usage :   Write_AT24C02(0x00, 15)
//          ReadByte_AT24C02(0x00)

void WriteByte_AT24C02(uchar add,uchar date)
{
   IIC_Start();
   IIC_SendByte(0xa0);//发方式字1010 0000
   IIC_WaitAck();
   IIC_SendByte(add);
   IIC_WaitAck();
   IIC_SendByte(date);
   IIC_WaitAck();
   IIC_Stop();
   Delay10ms();
}

uchar ReadByte_AT24C02(uchar add)
{
   uchar date;
 
   IIC_Start();
   IIC_SendByte(0xa0);//发方式字1010 0000
   IIC_WaitAck();
   IIC_SendByte(add);
   IIC_WaitAck();
 
   IIC_Start();
   IIC_SendByte(0xa1);//发方式字1010 0001
   IIC_WaitAck();
   date = IIC_RecByte();
   IIC_Ack(0);
   IIC_Stop();
   return date;
}


// ----------------------------------------------DS18B20----------------------------------------------- //

//DS18b20 has function Delay_OneWire 修改为下面这个
//单总线延时函数
void Delay_OneWire(unsigned int t)
{
  unsigned char i;
	while(t--){
		for(i=0;i<12;i++);
	}
}
//DS18B20温度采集程序：整数
//把采集函数放在主函数里面，每过一段时间执行一次，把display函数放在中断里面，不要都放在中断里面(原因不明)
unsigned char rd_temperature(void)
{
    unsigned char low,high;
  	char temp;
  
  	Init_DS18B20();
  	Write_DS18B20(0xCC);
  	Write_DS18B20(0x44); //启动温度转换
  	Delay_OneWire(200);

  	Init_DS18B20();
  	Write_DS18B20(0xCC);
  	Write_DS18B20(0xBE); //读取寄存器

  	low = Read_DS18B20(); //低字节
  	high = Read_DS18B20(); //高字节
  
  	temp = high<<4;
  	temp |= (low>>4);
  
  	return temp;
}

float rd_temperature_f(void)
{
    unsigned int temp;
	float temperature;
    unsigned char low,high;
  
  	Init_DS18B20();
  	Write_DS18B20(0xCC);
  	Write_DS18B20(0x44); //启动温度转换
  	Delay_OneWire(200);

  	Init_DS18B20();
  	Write_DS18B20(0xCC);
  	Write_DS18B20(0xBE); //读取寄存器

  	low = Read_DS18B20(); //低字节
  	high = Read_DS18B20(); //高字节
/** 精度为0.0625摄氏度 */  
	temp = (high&0x0f);
	temp <<= 8;
	temp |= low;
	temperature = temp*0.0625;
  
  	return temperature;
}

// ---------------------------------------------------DS1302----------------------------------------------- //


uchar s_time[7]={0x28,0x23,0x20,0x05,0x03,0x04,0x17};//设置时间数组,前面三个直接是十进制时间（s，min，h， r, y, xxx, n）
uchar g_time[7]={0};  //保存时间数组

//调用DS1302驱动程序完成封装
void set_time()		//设置时间
{
 	char i=0;
	char adr=0x80;   //写地址
	Write_Ds1302_Byte(0x8e,0x00);    //写保护可写
	for(i=0;i<7;i++)
	Write_Ds1302_Byte(adr+i*2,s_time[i]);
	Write_Ds1302_Byte(0x8e,0x80);   //写保护不可写
 
}


//get time 在 主循环里边每x个周期执行一次，不然会有重影
void get_time()	  //获取时间
{
 	char i=0;
	for(i=0;i<7;i++)
	{
	g_time[i]=Read_Ds1302_Byte(0x81+i*2);
	delay(30);
	}
}

void dstime_sfm()	 //显示时间
{
	//BCD转码
 	dspbuf[6]=(g_time[0]&0x70)>>4;//秒十位
	dspbuf[7]=g_time[0]&0x0f;	  //秒各位
 
	dspbuf[5]=11;	//-
 
	dspbuf[3]=(g_time[1]&0x70)>>4;//分十位
	dspbuf[4]=g_time[1]&0x0f;	  //分各位
 
	 dspbuf[2]=11;	//-
 
	dspbuf[0]=(g_time[2]&0x30)>>4;//时十位
	dspbuf[1]=g_time[2]&0x0f;	  //时各位
 
}

void dstime_nyr()		  //显示年月日
{
	//BCD转码
 	dspbuf[6]=(g_time[3]&0x30)>>4;//日十位
	dspbuf[7]=g_time[3]&0x0f;	  //日各位
 
	dspbuf[5]=11;	//-
 
	dspbuf[3]=(g_time[4]&0x10)>>4;//月十位
	dspbuf[4]=g_time[4]&0x0f;	  //月各位
 
	dspbuf[2]=11;	//-
 
	dspbuf[0]=(g_time[6]&0xf0)>>4;//年十位
	dspbuf[1]=g_time[6]&0x0f;	  //年各位
 	
}



// -------------------------------------------------UART---------------------------------------- //
// 串口貌似是阻塞的，串口发送的时候跳不到别的中断里面(数码管显示会崩)
// 使用串口貌似需要一个用TL1的和波特率有关的中断初始化
// 初始化这个定时器之后在main里不要忘记把ET1置1

void Uart_Timer_Init(void)		//1毫秒@11.0592MHz
{
	SCON = 0x50;                
    AUXR = 0x40;                //1T
    TMOD = 0x00;                //
    TL1 = (65536 - (SYSTEMCLOCK/4/BAUD));   //
    TH1 = (65536 - (SYSTEMCLOCK/4/BAUD))>>8;
	TR1 = 1;		//定时器1开始计时
}

// 然后在Main 函数里面直接调用uart_sendstring就好

void uart_sendstring(unsigned char *str)
{
    unsigned char *p;
    
    p = str;
    while(*p != '\0')
    {
        SBUF = *p;
		while(TI == 0);  //等待发送标志位置位
		TI = 0;
        p++;
    }
}