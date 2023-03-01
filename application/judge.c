#include "judge.h"
#include "string.h"
#include "chassis_task.h"
#include "FreeRTOS.h"


/*****************ϵͳ���ݶ���**********************/
ext_game_status_t          RobotState;		
ext_robot_hurt_t           HurtData;			
ext_shoot_data_t           ShootData;		
ext_power_heat_data_t      PowerHeatData;	
ext_rfid_status_t          RfidDetect;		
ext_game_result_t		   GameResultData;	
ext_buff_t			       GetBuffData;		
ext_game_robot_pos_t	   GameRobotPosData;	


xFrameHeader              FrameHeader;		//����֡ͷ��Ϣ
xShowData                 ShowData;
/****************************************************/

bool Judge_Data_TF = FALSE;//���������Ƿ����,������������

/**************����ϵͳ���ݸ���****************/
uint16_t ShootNum;//ͳ�Ʒ�����,0x0003����һ������Ϊ������һ��
bool Hurt_Data_Update = FALSE;//װ�װ��˺������Ƿ����,ÿ��һ���˺���TRUE,Ȼ��������FALSE,������������


//��ǰ�ȼ���Ӧ����������,18���
#define HEAT_LEVEL1 120         //240
#define HEAT_LEVEL2 240         //360
#define HEAT_LEVEL3 480         //480

//��ǰ�ȼ���Ӧ��ǹ����ȴ
#define COLD_LEVEL1 120         //40
#define COLD_LEVEL2 240         //60
#define COLD_LEVEL3 480         //80

portTickType shoot_time;//������ʱ����

portTickType shoot_ping;//����������շ����ӳ�

/*********************************����ϵͳ���ݶ�ȡ**************************************/

/**
  * @brief  ��ȡ��������,loop��ѭ�����ô˺�������ȡ����
  * @param  ��������
  * @retval �Ƿ�������ж�������
  * @attention  �ڴ��ж�֡ͷ��CRCУ��,������д������
  */
int CmdID_check = 0;//�������������
bool  Judge_Read_Data(uint8_t *ReadFromUsart)
{
//	bool retval_tf = FALSE;//������ȷ����־,ÿ�ε��ö�ȡ����ϵͳ���ݺ�������Ĭ��Ϊ����
	
//	uint16_t judge_length;//ͳ��һ֡���ݳ��� 
	
	int CmdID = 0;//�������������
	
	/***------------------*****/
	//�����ݰ��������κδ���
//	if (ReadFromUsart == NULL)
//	{
//		return -1;
//	}
	
	//д��֡ͷ����,�����ж��Ƿ�ʼ�洢��������
	memcpy(&FrameHeader, ReadFromUsart, LEN_HEADER);
	
	//�ж�֡ͷ�����Ƿ�Ϊ0xA5
	if(ReadFromUsart[ SOF ] == JUDGE_FRAME_HEADER)
	{
//		//֡ͷCRC8У��
//		if (Verify_CRC8_Check_Sum( ReadFromUsart, LEN_HEADER ) == TRUE)
//		{
//			//ͳ��һ֡���ݳ���,����CR16У��
//			judge_length = ReadFromUsart[ DATA_LENGTH ] + LEN_HEADER + LEN_CMDID + LEN_TAIL;;

//			//֡βCRC16У��
//			if(Verify_CRC16_Check_Sum(ReadFromUsart,judge_length) == TRUE)
//			{
//				retval_tf = TRUE;//��У�������˵�����ݿ���
				
				CmdID = (ReadFromUsart[6] << 8 | ReadFromUsart[5]);
				//��������������,�����ݿ�������Ӧ�ṹ����(ע�⿽�����ݵĳ���)
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
						if(HurtData.hurt_type == 0)//��װ�װ���������˺�
						{	Hurt_Data_Update = TRUE;	}//װ������ÿ����һ�����ж�Ϊ�ܵ�һ���˺�
						
					break;
					
					case ID_SHOOT:      
						memcpy(&ShootData, (ReadFromUsart + DATA), LEN_SHOOT);
						JUDGE_ShootNumCount();//������ͳ��,��������˫ǹ��,��׼
					break;
	
					case ID_RFID:        
						memcpy(&RfidDetect, (ReadFromUsart + DATA), LEN_RFID);
					break;

					
				}
				
				//�׵�ַ��֡����,ָ��CRC16��һ�ֽ�,�����ж��Ƿ�Ϊ0xA5,�����ж�һ�����ݰ��Ƿ��ж�֡����
				if(*(ReadFromUsart + sizeof(xFrameHeader) + LEN_CMDID + FrameHeader.DataLength + LEN_TAIL) == 0xA5)
				{
					//���һ�����ݰ������˶�֡����,���ٴζ�ȡ
					Judge_Read_Data(ReadFromUsart + sizeof(xFrameHeader) + LEN_CMDID + FrameHeader.DataLength + LEN_TAIL);
				}
//			}
//		}
	}
	
//	if (retval_tf == TRUE)
//	{
//		Judge_Data_TF = TRUE;//����������
//	}
//	else		//ֻҪCRC16У�鲻ͨ����ΪFALSE
//	{
//		Judge_Data_TF = FALSE;//����������
//	}
//	
//	return retval_tf;//����������������
}


/**************************�û��Զ��������ϴ����ͻ���******************************/

/**
  * @brief  �ϴ��Զ�������
  * @param  void
  * @retval void
  * @attention  ���ݴ��,�����ɺ�ͨ�����ڷ��͵�����ϵͳ
  */
//void JUDGE_Show_Data(void)
//{
//	//��ʵ����22�ͺ�,������ռ�Ҳ����ν
//	uint8_t Show_Pack[50] = {0};//���ȴ���ͷ5+ָ��2+����13+β2����,�����u8��Ϊ�˺ü����ֽ���
//	int i;//ѭ�����ʹ���
//	
//	//֡ͷЭ��
//	FrameHeader.SOF        = JUDGE_FRAME_HEADER;
//	FrameHeader.Seq        = 0;		//Ϊʲô��0,�ٷ�����û��˵
//	FrameHeader.DataLength = LEN_SHOW;
//	
//	//д��֡ͷ
//	memcpy( Show_Pack, &FrameHeader, LEN_HEADER );
//	
//	//֡ͷCRC8У��Э��
//	Append_CRC8_Check_Sum( Show_Pack, LEN_HEADER );
//	
//	//д��������
//	ShowData.CmdID = ID_SHOW;
//	
//	//д������
//	ShowData.data1 = Capvoltage_Percent();//��ʾ����ʣ�����      //(float)ShootNum;//��һ����ʾ���Ƿ�����
//	ShowData.data2 = 66.66;//Fric_GetHeatInc();//�ڶ�����ʾ��ǰ����(Ŀ������)
//	ShowData.data3 = 88.88;
//	ShowData.mask  = 0x00;//��ȫ��,��6λ��Ч
//	
//	if(VISION_IfAutoRed() == TRUE)
//	{
//		ShowData.mask &= 0x1f;//��6λ��0
//	}
//	else if(VISION_IfAutoRed() == FALSE)
//	{
//		ShowData.mask |= 0x20;//��6λ��1
//	}
//	
//	
//	memcpy( Show_Pack + LEN_HEADER, &ShowData, (LEN_CMDID + LEN_SHOW) );
//	
//	//֡βCRC16У��Э��
//	Append_CRC16_Check_Sum( Show_Pack, (LEN_HEADER + LEN_CMDID + LEN_SHOW + LEN_TAIL) );
//	
//	//������õ�����ͨ��������λ���͵�����ϵͳ
//	for (i = 0; i < (LEN_HEADER + LEN_CMDID + LEN_SHOW + LEN_TAIL); i++)
//	{
//		UART5_SendChar( Show_Pack[i] );
//	}
//}


/********************�������ݸ����жϺ���***************************/

/**
  * @brief  �����Ƿ����
  * @param  void
  * @retval  TRUE����   FALSE������
  * @attention  �ڲ��ж�ȡ������ʵʱ�ı䷵��ֵ
  */
bool JUDGE_sGetDataState(void)
{
	return Judge_Data_TF;
}

/**
  * @brief  ��ȡ˲ʱ����
  * @param  void
  * @retval ʵʱ����ֵ
  * @attention  
  */
float JUDGE_fGetChassisPower(void)
{
	return (PowerHeatData.chassis_power);
}

/**
  * @brief  ��ȡʣ�ཹ������
  * @param  void
  * @retval ʣ�໺�役������(���60)
  * @attention  
  */
float JUDGE_fGetRemainEnergy(void)
{
	return (PowerHeatData.chassis_power_buffer);
}

/**
  * @brief  ��ȡ��ǰ�ȼ�
  * @param  void
  * @retval ��ǰ�ȼ�
  * @attention  
  */
uint8_t JUDGE_ucGetRobotLevel(void)
{
    return	RobotState.robotLevel;
}

/**
  * @brief  ��ȡǹ������
  * @param  void
  * @retval 17mm
  * @attention  
  */
uint16_t JUDGE_usGetRemoteHeat17(void)
{
	return PowerHeatData.shooter_id1_17mm_cooling_heat;
}

/**
  * @brief  ͳ�Ʒ�����
  * @param  void
  * @retval void
  * @attention  
  */
void JUDGE_ShootNumCount(void)    //0��һ��������û�ҵ�REVOL_uiGetRevolTime()
{
	ShootNum++;
	shoot_time = xTaskGetTickCount();//��ȡ���跢��ʱ��ϵͳʱ��
	shoot_ping = shoot_time - 0;//�����ӳ�
}

/**
  * @brief  ��ȡ������
  * @param  void
  * @retval ������
  * @attention ��������˫ǹ��
  */
uint16_t JUDGE_usGetShootNum(void)
{
	return ShootNum;
}

/**
  * @brief  ����������
  * @param  void
  * @retval void
  * @attention 
  */
void JUDGE_ShootNum_Clear(void)
{
	ShootNum = 0;
}

/**
  * @brief  ��ȡǹ������
  * @param  void
  * @retval ��ǰ�ȼ���������
  * @attention  
  */
uint16_t JUDGE_usGetHeatLimit(void)
{
	if (RobotState.robotLevel == 1)//1��
	{
		return HEAT_LEVEL1;
	}
	else if (RobotState.robotLevel == 2)//2��
	{
		return HEAT_LEVEL2;
	}
	else if (RobotState.robotLevel == 3)//3��
	{
		return HEAT_LEVEL3;
	}
	else//��ֹ����������,ǿ���޵���С
	{
		return HEAT_LEVEL1;
	}
}

/**
  * @brief  ��ǰ�ȼ���Ӧ��ǹ��ÿ����ȴֵ
  * @param  void
  * @retval ��ǰ�ȼ���ȴ�ٶ�
  * @attention  
  */
uint16_t JUDGE_usGetShootCold(void)
{
	if (RobotState.robotLevel == 1)//1��
	{
		return COLD_LEVEL1;
	}
	else if (RobotState.robotLevel == 2)//2��
	{
		return COLD_LEVEL2;
	}
	else if (RobotState.robotLevel == 3)//3��
	{
		return COLD_LEVEL3;
	}
	else//��ֹ����������,ǿ���޵���С
	{
		return COLD_LEVEL1;
	}
}

/****************�����Զ������ж���*******************/
/**
  * @brief  װ�װ��˺������Ƿ����
  * @param  void
  * @retval TRUE�Ѹ���   FALSEû����
  * @attention  
  */
bool JUDGE_IfArmorHurt(void)
{
	static portTickType ulCurrent = 0;
	static uint32_t ulDelay = 0;
	static bool IfHurt = FALSE;//Ĭ��װ�װ崦������״̬

	
	ulCurrent = xTaskGetTickCount();

	if (Hurt_Data_Update == TRUE)//װ�װ����ݸ���
	{
		Hurt_Data_Update = FALSE;//��֤���жϵ��´�װ�װ��˺�����
		ulDelay = ulCurrent + 200;//
		IfHurt = TRUE;
	}
	else if (ulCurrent > ulDelay)//
	{
		IfHurt = FALSE;
	}
	
	return IfHurt;
}



///************************��������Ԥ���ú���****************************/

///**
//  * @brief  ����ʣ�๦��
//  * @param  void
//  * @retval ʣ�๦��W
//  * @attention  ����ʼ�ʵʱ����,�������ݱ���
//  */
//float JUDGE_fGetChassisResiduePower(void)
//{
//	return (80 - PowerHeatData.chassisPower);
//}

///**
//  * @brief  ���㹩���������ݵĵ���ֵ
//  * @param  void
//  * @retval ��������ֵ
//  * @attention  �������ݱ���
//  */
//float JUDGE_fGetSuper_Cap_Ele(void)
//{
//	return ((80 - PowerHeatData.chassisPower) / PowerHeatData.chassisCurrent);
//}

