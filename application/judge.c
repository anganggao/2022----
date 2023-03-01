#include "judge.h"
#include "string.h"
#include "chassis_task.h"
#include "FreeRTOS.h"


/*****************系统数据定义**********************/
ext_game_status_t          RobotState;		
ext_robot_hurt_t           HurtData;			
ext_shoot_data_t           ShootData;		
ext_power_heat_data_t      PowerHeatData;	
ext_rfid_status_t          RfidDetect;		
ext_game_result_t		   GameResultData;	
ext_buff_t			       GetBuffData;		
ext_game_robot_pos_t	   GameRobotPosData;	


xFrameHeader              FrameHeader;		//发送帧头信息
xShowData                 ShowData;
/****************************************************/

bool Judge_Data_TF = FALSE;//裁判数据是否可用,辅助函数调用

/**************裁判系统数据辅助****************/
uint16_t ShootNum;//统计发弹量,0x0003触发一次则认为发射了一颗
bool Hurt_Data_Update = FALSE;//装甲板伤害数据是否更新,每受一次伤害置TRUE,然后立即置FALSE,给底盘闪避用


//当前等级对应的热量上限,18年版
#define HEAT_LEVEL1 120         //240
#define HEAT_LEVEL2 240         //360
#define HEAT_LEVEL3 480         //480

//当前等级对应的枪口冷却
#define COLD_LEVEL1 120         //40
#define COLD_LEVEL2 240         //60
#define COLD_LEVEL3 480         //80

portTickType shoot_time;//发射延时测试

portTickType shoot_ping;//计算出的最终发弹延迟

/*********************************裁判系统数据读取**************************************/

/**
  * @brief  读取裁判数据,loop中循环调用此函数来读取数据
  * @param  缓存数据
  * @retval 是否对正误判断做处理
  * @attention  在此判断帧头和CRC校验,无误再写入数据
  */
int CmdID_check = 0;//数据命令码解析
bool  Judge_Read_Data(uint8_t *ReadFromUsart)
{
//	bool retval_tf = FALSE;//数据正确与否标志,每次调用读取裁判系统数据函数都先默认为错误
	
//	uint16_t judge_length;//统计一帧数据长度 
	
	int CmdID = 0;//数据命令码解析
	
	/***------------------*****/
	//无数据包，则不作任何处理
//	if (ReadFromUsart == NULL)
//	{
//		return -1;
//	}
	
	//写入帧头数据,用于判断是否开始存储裁判数据
	memcpy(&FrameHeader, ReadFromUsart, LEN_HEADER);
	
	//判断帧头数据是否为0xA5
	if(ReadFromUsart[ SOF ] == JUDGE_FRAME_HEADER)
	{
//		//帧头CRC8校验
//		if (Verify_CRC8_Check_Sum( ReadFromUsart, LEN_HEADER ) == TRUE)
//		{
//			//统计一帧数据长度,用于CR16校验
//			judge_length = ReadFromUsart[ DATA_LENGTH ] + LEN_HEADER + LEN_CMDID + LEN_TAIL;;

//			//帧尾CRC16校验
//			if(Verify_CRC16_Check_Sum(ReadFromUsart,judge_length) == TRUE)
//			{
//				retval_tf = TRUE;//都校验过了则说明数据可用
				
				CmdID = (ReadFromUsart[6] << 8 | ReadFromUsart[5]);
				//解析数据命令码,将数据拷贝到相应结构体中(注意拷贝数据的长度)
		        CmdID_check = CmdID;
				switch(CmdID)
				{
					case ID_STATE:  
						memcpy(&RobotState, (ReadFromUsart + DATA), LEN_STATE);
					break;
					
					case ID_GAME_RESULT:  
						memcpy(&GameResultData, (ReadFromUsart + DATA), LEN_GAME_RESULT);
					break;
					
					case ID_POWER_HEAT:    
						memcpy(&PowerHeatData, (ReadFromUsart + DATA), LEN_POWER_HEAT);
					break;
					
					case ID_POSITION:    
						memcpy(&GameRobotPosData, (ReadFromUsart + DATA), LEN_POSITIO);
					break;
					
					case ID_BUFF_GET:   
						memcpy(&GetBuffData, (ReadFromUsart + DATA), LEN_BUFF_GET);
					break;
					
					case ID_HURT:      
						memcpy(&HurtData, (ReadFromUsart + DATA), LEN_HURT);
						if(HurtData.hurt_type == 0)//非装甲板离线造成伤害
						{	Hurt_Data_Update = TRUE;	}//装甲数据每更新一次则判定为受到一次伤害
						
					break;
					
					case ID_SHOOT:      
						memcpy(&ShootData, (ReadFromUsart + DATA), LEN_SHOOT);
						JUDGE_ShootNumCount();//发弹量统计,不适用于双枪管,不准
					break;
	
					case ID_RFID:        
						memcpy(&RfidDetect, (ReadFromUsart + DATA), LEN_RFID);
					break;

					
				}
				
				//首地址加帧长度,指向CRC16下一字节,用来判断是否为0xA5,用来判断一个数据包是否有多帧数据
				if(*(ReadFromUsart + sizeof(xFrameHeader) + LEN_CMDID + FrameHeader.DataLength + LEN_TAIL) == 0xA5)
				{
					//如果一个数据包出现了多帧数据,则再次读取
					Judge_Read_Data(ReadFromUsart + sizeof(xFrameHeader) + LEN_CMDID + FrameHeader.DataLength + LEN_TAIL);
				}
//			}
//		}
	}
	
//	if (retval_tf == TRUE)
//	{
//		Judge_Data_TF = TRUE;//辅助函数用
//	}
//	else		//只要CRC16校验不通过就为FALSE
//	{
//		Judge_Data_TF = FALSE;//辅助函数用
//	}
//	
//	return retval_tf;//对数据正误做处理
}


/**************************用户自定义数据上传到客户端******************************/

/**
  * @brief  上传自定义数据
  * @param  void
  * @retval void
  * @attention  数据打包,打包完成后通过串口发送到裁判系统
  */
//void JUDGE_Show_Data(void)
//{
//	//其实大于22就好,分配多点空间也无所谓
//	uint8_t Show_Pack[50] = {0};//长度大于头5+指令2+数据13+尾2就行,定义成u8是为了好计算字节数
//	int i;//循环发送次数
//	
//	//帧头协议
//	FrameHeader.SOF        = JUDGE_FRAME_HEADER;
//	FrameHeader.Seq        = 0;		//为什么是0,官方好像没有说
//	FrameHeader.DataLength = LEN_SHOW;
//	
//	//写入帧头
//	memcpy( Show_Pack, &FrameHeader, LEN_HEADER );
//	
//	//帧头CRC8校验协议
//	Append_CRC8_Check_Sum( Show_Pack, LEN_HEADER );
//	
//	//写入命令码
//	ShowData.CmdID = ID_SHOW;
//	
//	//写入数据
//	ShowData.data1 = Capvoltage_Percent();//显示电容剩余电量      //(float)ShootNum;//第一个显示的是发弹量
//	ShowData.data2 = 66.66;//Fric_GetHeatInc();//第二个显示当前射速(目标射速)
//	ShowData.data3 = 88.88;
//	ShowData.mask  = 0x00;//灯全关,低6位有效
//	
//	if(VISION_IfAutoRed() == TRUE)
//	{
//		ShowData.mask &= 0x1f;//第6位置0
//	}
//	else if(VISION_IfAutoRed() == FALSE)
//	{
//		ShowData.mask |= 0x20;//第6位置1
//	}
//	
//	
//	memcpy( Show_Pack + LEN_HEADER, &ShowData, (LEN_CMDID + LEN_SHOW) );
//	
//	//帧尾CRC16校验协议
//	Append_CRC16_Check_Sum( Show_Pack, (LEN_HEADER + LEN_CMDID + LEN_SHOW + LEN_TAIL) );
//	
//	//将打包好的数据通过串口移位发送到裁判系统
//	for (i = 0; i < (LEN_HEADER + LEN_CMDID + LEN_SHOW + LEN_TAIL); i++)
//	{
//		UART5_SendChar( Show_Pack[i] );
//	}
//}


/********************裁判数据辅助判断函数***************************/

/**
  * @brief  数据是否可用
  * @param  void
  * @retval  TRUE可用   FALSE不可用
  * @attention  在裁判读取函数中实时改变返回值
  */
bool JUDGE_sGetDataState(void)
{
	return Judge_Data_TF;
}

/**
  * @brief  读取瞬时功率
  * @param  void
  * @retval 实时功率值
  * @attention  
  */
float JUDGE_fGetChassisPower(void)
{
	return (PowerHeatData.chassis_power);
}

/**
  * @brief  读取剩余焦耳能量
  * @param  void
  * @retval 剩余缓冲焦耳能量(最大60)
  * @attention  
  */
float JUDGE_fGetRemainEnergy(void)
{
	return (PowerHeatData.chassis_power_buffer);
}

/**
  * @brief  读取当前等级
  * @param  void
  * @retval 当前等级
  * @attention  
  */
uint8_t JUDGE_ucGetRobotLevel(void)
{
    return	RobotState.robotLevel;
}

/**
  * @brief  读取枪口热量
  * @param  void
  * @retval 17mm
  * @attention  
  */
uint16_t JUDGE_usGetRemoteHeat17(void)
{
	return PowerHeatData.shooter_id1_17mm_cooling_heat;
}

/**
  * @brief  统计发弹量
  * @param  void
  * @retval void
  * @attention  
  */
void JUDGE_ShootNumCount(void)    //0是一个函数，没找到REVOL_uiGetRevolTime()
{
	ShootNum++;
	shoot_time = xTaskGetTickCount();//获取弹丸发射时的系统时间
	shoot_ping = shoot_time - 0;//计算延迟
}

/**
  * @brief  读取发弹量
  * @param  void
  * @retval 发弹量
  * @attention 不适用于双枪管
  */
uint16_t JUDGE_usGetShootNum(void)
{
	return ShootNum;
}

/**
  * @brief  发弹量清零
  * @param  void
  * @retval void
  * @attention 
  */
void JUDGE_ShootNum_Clear(void)
{
	ShootNum = 0;
}

/**
  * @brief  读取枪口热量
  * @param  void
  * @retval 当前等级热量上限
  * @attention  
  */
uint16_t JUDGE_usGetHeatLimit(void)
{
	if (RobotState.robotLevel == 1)//1级
	{
		return HEAT_LEVEL1;
	}
	else if (RobotState.robotLevel == 2)//2级
	{
		return HEAT_LEVEL2;
	}
	else if (RobotState.robotLevel == 3)//3级
	{
		return HEAT_LEVEL3;
	}
	else//防止读不到数据,强制限到最小
	{
		return HEAT_LEVEL1;
	}
}

/**
  * @brief  当前等级对应的枪口每秒冷却值
  * @param  void
  * @retval 当前等级冷却速度
  * @attention  
  */
uint16_t JUDGE_usGetShootCold(void)
{
	if (RobotState.robotLevel == 1)//1级
	{
		return COLD_LEVEL1;
	}
	else if (RobotState.robotLevel == 2)//2级
	{
		return COLD_LEVEL2;
	}
	else if (RobotState.robotLevel == 3)//3级
	{
		return COLD_LEVEL3;
	}
	else//防止读不到数据,强制限到最小
	{
		return COLD_LEVEL1;
	}
}

/****************底盘自动闪避判断用*******************/
/**
  * @brief  装甲板伤害数据是否更新
  * @param  void
  * @retval TRUE已更新   FALSE没更新
  * @attention  
  */
bool JUDGE_IfArmorHurt(void)
{
	static portTickType ulCurrent = 0;
	static uint32_t ulDelay = 0;
	static bool IfHurt = FALSE;//默认装甲板处于离线状态

	
	ulCurrent = xTaskGetTickCount();

	if (Hurt_Data_Update == TRUE)//装甲板数据更新
	{
		Hurt_Data_Update = FALSE;//保证能判断到下次装甲板伤害更新
		ulDelay = ulCurrent + 200;//
		IfHurt = TRUE;
	}
	else if (ulCurrent > ulDelay)//
	{
		IfHurt = FALSE;
	}
	
	return IfHurt;
}



///************************超级电容预留用函数****************************/

///**
//  * @brief  计算剩余功率
//  * @param  void
//  * @retval 剩余功率W
//  * @attention  最大功率减实时功率,超级电容备用
//  */
//float JUDGE_fGetChassisResiduePower(void)
//{
//	return (80 - PowerHeatData.chassisPower);
//}

///**
//  * @brief  计算供给超级电容的电流值
//  * @param  void
//  * @retval 供给电流值
//  * @attention  超级电容备用
//  */
//float JUDGE_fGetSuper_Cap_Ele(void)
//{
//	return ((80 - PowerHeatData.chassisPower) / PowerHeatData.chassisCurrent);
//}

