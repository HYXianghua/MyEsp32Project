#ifndef __PT6311_FUTABA_H_
#define __PT6311_FUTABA_H_

/***********VFD接口设置**************/

#define VFDPW 12
#define DATA 14
#define CLK 27
#define STB 26
void vVfdChangeOptions(uint8_t Atarget, uint8_t Btarget);
void vVfdTaskInit(void);
void vVfdReFush(uint8_t ucAddr, uint8_t dat);
void vVfdStrFush(uint8_t *dat);
void vVfdTimeUpData(long ucHH, long ucMM, long ucSS);
void vVfdWifiLinkOk(void);
void vVfdWifiLinking(void);
void vVfdTimeTaskInit(void);
void vVfdNewMiAnimation(const char ch[5]);
/*void PT6311_Wrt(uint8_t dat);
void PT6311_Func(void);
void PT6311_Mode(void);
void PT6311_Start(void);
void PT6311_Dim(uint8_t Light);
void PT6311_Boot(void);
void print_vfd(void);
void PT6311_Init(void);
void PT6311_ReFush(uint8_t Addr, uint8_t dat);
void PT6311_NumDis(char *Sp);
void PT6311_MiDis(uint16_t *Sp);
void PT6311_MiStr(char *ch);
void PT6311_RoundDis(uint16_t Speed);
void PT6311_ConvertTime(uint8_t Hou, uint8_t Min, uint8_t Sec);
void PT6311_TimeMt(uint8_t Speed);
void PT6311_NetDis(uint16_t Speed);
void PT6311_StrMiMt(uint8_t Speed);
void PT6311_DobReFush(uint8_t Addr, uint16_t dat);
void PT6311_NumRAM(void);
void PT6311_MiRAM(void);
void PT6311_TimeMp(uint8_t Speed, uint8_t Point);
void PT6311_NetWorkMt(void);
void PT6311_TimeInit(uint8_t Title, uint8_t Icon);
void PT6311_HRDRMt(uint8_t Speed);
void PT6311_LongStr(char *ch, uint8_t Speed);
void PT6311_PutYear(uint8_t Year);
void PT6311_PutMonDay(uint8_t Month, uint8_t Day);
void PT6311_PutWeek(uint8_t Week);
void PT6311_PutTemp(uint8_t Temp);
void PT6311_PutHumi(uint8_t Humi);
void PT6311_ChType(uint8_t Type, uint8_t Error);
void PT6311_PutTime(uint8_t Tempoary);
void PT6311_PutYTime(uint8_t Tempoary);
void PT6311_CTType(uint8_t Type, uint8_t Error);
void PT6311_State(uint8_t State);
void PT6311_CYCLEMt(uint8_t RoundCount, uint8_t Speed);
void PT6311_SWMt(uint8_t Speed);
void PT6311trMiMt(uint8_t Speed);*/

#endif
