
#include <stdint.h>

#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "esp_sntp.h"

#include "app_main.h"
#include "xxh_pt6311.h"
#include "xxh_gpio.h"
#define FPS 40
// ucADDR
static const char *TAG = "VFD";
#define VMAIL 32
#define VMUSIC 31   // MUSIC AND ↑7*D
#define VNETWORK 30 // NETWORK AND ↓7*D
#define VUSB 29     // USB

#define VBOXUP 28 //方格外圈
#define VBOXDN 27 // 4*4方格

#define VHRDR 25  //四横点 AND DVD REC HDD REC
#define VROUND 24 //圆盘
#define VEPG 23
#define VAUTO 20
#define VTRANS 17
#define VTITLE 14
#define VRATE 11
#define VCHP 8
#define VTRK 5
#define VCH 2

#define VMISEG0 12
#define VMISEG1 9
#define VMISEG2 6
#define VMISEG3 3
#define VMISEG4 0

#define VNUMSEG0 15
#define VNUMSEG1 21
#define VNUMSEG2 22
#define VNUMSEG3 18
#define VNUMSEG4 19
#define VNUMSEG5 16

/************VFD驱动部分***************/
uint8_t Start = 0xff;
const uint8_t RoundTab[] = {0x03, 0x06, 0x0C, 0x18,
                            0x30, 0x60, 0xC0, 0x81}; //转盘用
const uint8_t CYCLETab[] = {0X00, 0x40, 0xC0, 0xC1, 0xC3, 0xC7,
                            0xCF, 0xDF, 0xFF}; //转盘用
const uint8_t LoadTab[] = {0x01, 0x03, 0x07, 0x0E, 0x1C, 0x38,
                           0x70, 0xE0, 0xC0, 0x80, 0x00}; //转盘用
const uint8_t HRDRTab[] = {0x0, 0x1, 0x3, 0x7,
                           0xE, 0xC, 0x8, 0x0};           // HDHR用
const uint8_t ucNumTab[] = {0x77, 0x12, 0x6b, 0x5b, 0x1e, // 0123456789-
                            0x5d, 0x7d, 0x13, 0x7f, 0x5f, 0x88};
const uint8_t WekTab[] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe}; //星期显示
uint16_t MiTab[] = {
    0x3146, 0x1002, 0xd145, 0xd143, 0xf003, 0xe143, // 0,1,2,3,4,5
    0xe147, 0x1102, 0xf147, 0xf143, 0x0000, 0xc001, // 6,7,8,9, ,-
    0x8808, 0xf107, 0xd552, 0x2144, 0x9552, 0xe145, // /,A,B,C,D,E
    0xe105, 0x6146, 0xf007, 0x8550, 0x1046, 0xa825, // F,G,H,I,J,K
    0x2044, 0xba06, 0xb226, 0x3146, 0xf105, 0x3166, // L,M,N,O,P,Q
    0xf125, 0xe143, 0x8510, 0x3046, 0xa80c, 0xb02e, // R,S,T,U,V,W
    0x8a28, 0x8a10, 0x8948, 0xc411, 0x8000, 0x0040, // X,Y,Z,+,.,_
    0x0220, 0xA101, 0xEA2B, 0x0808                  // \,℃,%
};

const uint8_t ucAddr[] = {VAUTO, VEPG, VTRANS, VMAIL, VRATE,
                          VTITLE, VCHP, VTRK, VCH, VUSB,
                          VMISEG0, VMISEG1, VMISEG2, VMISEG3, VMISEG4,
                          VNUMSEG0, VNUMSEG1, VNUMSEG2, VNUMSEG3, VNUMSEG4,
                          VNUMSEG5, VNETWORK, VBOXDN, VROUND, VHRDR};

typedef struct xVfdTextType
{ //独立文字
    uint8_t Frame;
    uint8_t Next;
    uint16_t data; // AUTO EPG TRANS MAIL RATE
                   //  TITLE CHP TRK CH USB
} xVfdTextType;

typedef struct xVfdDotsType
{ //点
    uint8_t Frame;
    uint8_t Next;
    uint8_t Data; //四点
    uint8_t Text; //文字
} xVfdDotsType;

typedef struct xVfdMiType
{
    uint8_t Frame;
    uint8_t Next;
    uint16_t Data[5]; //米字
} xVfdMiType;

typedef struct xVfdVboxupType
{
    uint8_t Frame;
    uint8_t Next;
    uint16_t Data; //棋盘格
} xVfdVboxupType;

typedef struct xVfdRoundType
{
    uint8_t Frame;
    uint8_t Next;
    uint8_t Data; //圆盘
} xVfdRoundType;

typedef struct xVfdCircleType
{
    uint8_t Frame;
    uint8_t Next;
    uint16_t Data; //圆圈
    uint16_t Text; // UPDot DNDot NETWORK MUSICc
} xVfdCircleType;

typedef struct xVfdNumType
{
    uint8_t Frame;
    uint8_t Next;
    uint8_t Data[6]; //数字
    uint8_t Dot;     // LU LD RU RD
} xVfdNumType;
#define Vfd_FRAMEMAX 12
xVfdTextType Text[Vfd_FRAMEMAX] = {0};     //文字
xVfdMiType Mi[Vfd_FRAMEMAX];               //米字
xVfdNumType Num[Vfd_FRAMEMAX] = {0};       //数字
xVfdCircleType Circle[Vfd_FRAMEMAX] = {0}; //圆圈
xVfdVboxupType Vboxup[Vfd_FRAMEMAX] = {0}; //棋格
xVfdRoundType Round[Vfd_FRAMEMAX] = {0};   //圆盘
xVfdDotsType Dots[Vfd_FRAMEMAX] = {0};     //四点
uint8_t ucFrameHeaderAddrS[8] = {0};       //元素头地址
uint8_t ucFrameUpDataFlag[8] = {0};        //帧更新
uint8_t ucElementUpDataFlag[8] = {0};      //元素更新
enum eFrameHeaderAddr
{
    eText,
    eMi,
    eNum,
    eCircle,
    eVboxup,
    eRound,
    eDots
}; //元素序号
//向6311发送数据
static void vVfdWrtData(uint8_t dat)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        vMyGpioControl(DATA, dat % 2);
        dat = dat >> 1;
        vMyGpioControl(CLK, 1);
        vMyGpioControl(CLK, 0);
    }
}
static void vVfdStbBlink(void)
{
    vMyGpioControl(STB, 0);
    vMyGpioControl(STB, 1);
}

static void vVfdFunc(void)
{
    vVfdStbBlink();
    vVfdWrtData(0x0a);
}

static void vVfdSetMode(void)
{
    vVfdStbBlink();
    vVfdWrtData(0x40);
}

static void vVfdTart(void)
{
    vVfdStbBlink();
    vVfdWrtData(0xc0);
}

void vVfdDimming(uint8_t Light) //亮度 136~143
{
    vVfdStbBlink();
    vVfdWrtData(136 + Light); // 138 143
}

//
void vVfdReFush(uint8_t ucAddr, uint8_t dat)
{
    vVfdStbBlink();
    vVfdWrtData(0xc0 + ucAddr);
    vVfdWrtData(dat);
    vMyGpioControl(STB, 0);
}

void vVfdStrFush(uint8_t *dat)
{
    vVfdStbBlink();
    vVfdWrtData(0xc0);
    for (uint8_t gg = 0; gg < 33; gg++)
    {
        vVfdWrtData(*(dat + gg));
    }
    vMyGpioControl(STB, 0);
}

static void vVfdInit(void)
{
    uint8_t i;
    vMyGpioControl(VFDPW, 1);
    vVfdFunc();    //设置显示方式
    vVfdSetMode(); //数据设定 写显存 自动增地址 正常模式
    vVfdTart();
    for (i = 0; i < 40; i++)
    {
        vVfdWrtData(0x00);
    }
}

void Vfdtate(uint8_t State) { vMyGpioControl(VFDPW, State & 0x1); }

//帧地址更新
static void vVfdFrameHeaderUpData(uint8_t head, uint8_t *now)
{
    static uint8_t ucFrameTimeBuff[8]; //播放时间
    ucFrameUpDataFlag[head] = 0;
    if (ucElementUpDataFlag[head])
    {
        ucElementUpDataFlag[head] = 0;
        ucFrameTimeBuff[head] = (*now) - 1;
        ucFrameUpDataFlag[head] = 1;
    }
    else if (ucFrameTimeBuff[head] > 1)
    {
        ucFrameTimeBuff[head] -= 1;
    }
    else
    {
        ucFrameHeaderAddrS[head] = *(now + 1);
        ucElementUpDataFlag[head] = 1;
    }
}

//帧更新
static void vVfdFrameUpData(void)
{
    static uint8_t ucFrameBuff[33]; //帧数据
    uint8_t i = 0,                  //帧地址
        ucFrameState = 0;           //帧状态
    uint16_t usDataBuff = 0;
    // text
    uint8_t *ucFrameHeader = &ucFrameHeaderAddrS[eText]; //元素指针
    vVfdFrameHeaderUpData(eText, (uint8_t *)&(Text[*ucFrameHeader]));
    if (ucFrameUpDataFlag[eText])
    {
        ucFrameState = 1;
        usDataBuff = Text[*ucFrameHeader].data;
        for (i = 0; i < 10; i++)
        {
            ucFrameBuff[ucAddr[i]] = usDataBuff % 2;
            usDataBuff = usDataBuff >> 1;
        }
    }

    // mi
    ucFrameHeader = &ucFrameHeaderAddrS[eMi];
    vVfdFrameHeaderUpData(eMi, (uint8_t *)&(Mi[*ucFrameHeader]));

    if (ucFrameUpDataFlag[eMi])
    {
        ucFrameState = 1;
        uint16_t *spucFrameBuff = Mi[*ucFrameHeader].Data;
        for (i = 10; i < 15; i++)
        {
            ucFrameBuff[ucAddr[i]] = (*(spucFrameBuff)) >> 8;
            ucFrameBuff[ucAddr[i] + 1] = *(spucFrameBuff);
            spucFrameBuff++;
        }
    }

    // num
    ucFrameHeader = &ucFrameHeaderAddrS[eNum];
    vVfdFrameHeaderUpData(eNum, (uint8_t *)&(Num[*ucFrameHeader]));

    if (ucFrameUpDataFlag[eNum])
    {
        ucFrameState = 1;
        uint8_t *pucFrameBuff = Num[*ucFrameHeader].Data;
        usDataBuff = Num[*ucFrameHeader].Dot;
        usDataBuff = usDataBuff << 1;
        for (i = 15; i < 21; i++)
        {
            ucFrameBuff[ucAddr[i]] = *(pucFrameBuff++) | (usDataBuff % 2) << 7;
            usDataBuff = usDataBuff >> 1;
        }
    }

    // 圆圈
    ucFrameHeader = &ucFrameHeaderAddrS[eCircle];
    vVfdFrameHeaderUpData(eCircle, (uint8_t *)&(Circle[*ucFrameHeader]));

    if (ucFrameUpDataFlag[eCircle])
    {
        ucFrameState = 1;
        //融合文字和点
        i = 21;
        usDataBuff = Circle[*ucFrameHeader].Data | Circle[*ucFrameHeader].Text;
        ucFrameBuff[ucAddr[i]] = usDataBuff;
        ucFrameBuff[ucAddr[i] + 1] = usDataBuff >> 8;
    }
    // 棋盘
    ucFrameHeader = &ucFrameHeaderAddrS[eVboxup];
    vVfdFrameHeaderUpData(eVboxup, (uint8_t *)&(Vboxup[*ucFrameHeader]));
    if (ucFrameUpDataFlag[eVboxup])
    {
        ucFrameState = 1;
        i = 22;
        usDataBuff = Vboxup[*ucFrameHeader].Data;
        ucFrameBuff[ucAddr[i]] = usDataBuff;
        ucFrameBuff[ucAddr[i] + 1] = usDataBuff >> 8;
    }
    // 圆盘
    ucFrameHeader = &ucFrameHeaderAddrS[eRound];
    vVfdFrameHeaderUpData(eRound, (uint8_t *)&(Round[*ucFrameHeader]));

    if (ucFrameUpDataFlag[eRound])
    {
        ucFrameState = 1;
        i = 23;
        usDataBuff = Round[*ucFrameHeader].Data;
        ucFrameBuff[ucAddr[i]] = usDataBuff;
    }
    // 四点
    ucFrameHeader = &ucFrameHeaderAddrS[eDots];
    vVfdFrameHeaderUpData(eDots, (uint8_t *)&(Dots[*ucFrameHeader]));

    if (ucFrameUpDataFlag[eDots])
    {
        ucFrameState = 1;
        //融合文字和点
        i = 24;
        usDataBuff = Dots[*ucFrameHeader].Data | Dots[*ucFrameHeader].Text;
        ucFrameBuff[ucAddr[i]] = usDataBuff;
    }
    if (ucFrameState)
    {
        vVfdStrFush(ucFrameBuff);
    }
}

//寻找新帧地址
static void vVfdGetNextFrameAddr(uint8_t *ucFrameAddrBuff)
{
    if (*ucFrameAddrBuff == Vfd_FRAMEMAX - 1)
    {
        *ucFrameAddrBuff = 0;
    }
    else
    {
        *ucFrameAddrBuff += 1;
    }
}

static void vVfdDotsLoad(uint8_t ucLoop)
{
    const uint8_t ucSpeed = 4;
    ucElementUpDataFlag[eDots] = 1;
    uint8_t ucFrameAddrBuff = ucFrameHeaderAddrS[eDots], //当前帧
        ucOldFrameAddr = 0;                              //原帧
    xVfdDotsType *pDot = &Dots[ucFrameAddrBuff];
    ucOldFrameAddr = ucFrameAddrBuff;
    for (uint8_t ucFor = 0; ucFor < 8; ucFor++)
    {
        pDot = &Dots[ucFrameAddrBuff];
        pDot->Data = HRDRTab[ucFor];
        pDot->Frame = ucSpeed;
        if (!ucLoop)
        {
            ucOldFrameAddr = ucFrameAddrBuff;
        }
        vVfdGetNextFrameAddr(&ucFrameAddrBuff);
        pDot->Text = Dots[ucOldFrameAddr].Text;
        pDot->Next = ucFrameAddrBuff;
    }
    pDot->Frame = ucSpeed * 2 * ucLoop;
    pDot->Next = ucOldFrameAddr;
}

void vVfdNewMiAnimation(const char ch[5])
{ // char转米字
    ucElementUpDataFlag[eMi] = 1;
    const uint8_t ucSpeed = 3;
    uint16_t usNewMiData[5];

    for (uint8_t ucFor = 0; ucFor < 5; ucFor++)
    {
        if (ch[ucFor] >= 'A' && ch[ucFor] <= 'Z')
        {
            usNewMiData[ucFor] = MiTab[ch[ucFor] - 52];
        }
        else if (ch[ucFor] >= '0' && ch[ucFor] <= '9')
        {
            usNewMiData[ucFor] = MiTab[ch[ucFor] - 48];
        }
        else if (ch[ucFor] == '_')
        {
            usNewMiData[ucFor] = MiTab[41];
        }
        else if (ch[ucFor] == ' ')
        {
            usNewMiData[ucFor] = 0;
        }
        else
        {
            usNewMiData[ucFor] = MiTab[(uint8_t)ch[ucFor]];
        }
    }

    uint8_t ucFrameAddrBuff = ucFrameHeaderAddrS[eMi], ucOldFrameAddrBuff;
    xVfdMiType *pMi;

    for (uint8_t ucFor = 12; ucFor < 16; ucFor++)
    {
        pMi = &Mi[ucFrameAddrBuff];
        for (uint8_t ucFora = 0; ucFora < 5; ucFora++)
        {
            pMi->Data[ucFora] = usNewMiData[ucFora] >> (15 - ucFor);
        }
        ucOldFrameAddrBuff = ucFrameAddrBuff;
        vVfdGetNextFrameAddr(&ucFrameAddrBuff);
        pMi->Next = ucFrameAddrBuff;
        pMi->Frame = ucSpeed;
    }
    pMi->Frame = 0;
    pMi->Next = ucOldFrameAddrBuff;
    vVfdDotsLoad(0);
}

void vVfdWifiLinking(void)
{
    ucElementUpDataFlag[eCircle] = 1;
    const uint8_t speed = 4;
    uint8_t ucFrameAddrBuff = ucFrameHeaderAddrS[eCircle], //当前帧
        ucOldFrameAddr;                                    //原帧
    uint16_t usFrameDataBuff = 0;                          //缓冲
    ucOldFrameAddr = ucFrameAddrBuff;

    xVfdCircleType *pCIr;
    usFrameDataBuff = 0b0000000100000001;
    for (uint8_t ucFor = 0; ucFor < 7; ucFor++)
    {
        pCIr = &Circle[ucFrameAddrBuff];
        pCIr->Data = usFrameDataBuff;
        usFrameDataBuff = pCIr->Data << 1;
        vVfdGetNextFrameAddr(&ucFrameAddrBuff);
        pCIr->Next = ucFrameAddrBuff;
        pCIr->Frame = speed;
        if (ucFor > 3)
        {
            pCIr->Text = 0b10000000;
        }
        else
        {
            pCIr->Text = 0;
        }
    }
    pCIr->Next = ucOldFrameAddr;

    vVfdNewMiAnimation("_WIFI");
}

void vVfdWifiLinkOk(void)
{
    ucElementUpDataFlag[eCircle] = 1;
    uint8_t ucFrameAddrBuff = ucFrameHeaderAddrS[eCircle];
    xVfdCircleType *pCIr = &Circle[ucFrameAddrBuff];
    pCIr->Data = 0b0111111101111111;
    pCIr->Frame = 0;
    pCIr->Text = 0b10000000;
    pCIr->Next = ucFrameAddrBuff;

    ucElementUpDataFlag[eNum] = 1;
    ucFrameAddrBuff = ucFrameHeaderAddrS[eNum];
    xVfdNumType *pNum = &Num[ucFrameAddrBuff];
    for (uint8_t ucFor = 0; ucFor < 6; ucFor++)
    {
        pNum->Data[ucFor] = 8;
    }
    pNum->Next = ucFrameAddrBuff;
    pNum->Frame = 0;
    pNum->Dot = 0b1111;
    vVfdDotsLoad(1);
    vVfdNewMiAnimation("_STNP");
}

static void vVfdSntpOk(void)
{
    ucElementUpDataFlag[eCircle] = 1;
    const uint8_t speed = FPS;
    uint8_t ucFrameAddrBuff = ucFrameHeaderAddrS[eCircle], //当前帧
        ucOldFrameAddr;                                    //原帧
    uint16_t usFrameDataBuff = 0;                          //缓冲
    xVfdCircleType *pCIr;
    ucOldFrameAddr = ucFrameAddrBuff;
    usFrameDataBuff = 0b0000000100000001;
    for (uint8_t ucFor = 0; ucFor < 7; ucFor++)
    {
        pCIr = &Circle[ucFrameAddrBuff];
        pCIr->Data = usFrameDataBuff;
        usFrameDataBuff = pCIr->Data << 1;
        vVfdGetNextFrameAddr(&ucFrameAddrBuff);
        pCIr->Next = ucFrameAddrBuff;
        pCIr->Frame = speed;
        pCIr->Text = 0b10000000;
    }
    pCIr->Next = ucOldFrameAddr;
    vVfdNewMiAnimation("_TIME");
}

static uint8_t ucVfdRoundLoad(uint8_t ucRoundCount)
{
    static uint8_t ucRoundFlag = 8;
    const uint8_t ucSpeed = 2;
    ucElementUpDataFlag[eRound] = 1;
    uint8_t ucFrameAddrBuff = ucFrameHeaderAddrS[eRound], //当前帧
        ucOldFrameAddr = 0;                               //原帧
    xVfdRoundType *pRou = &Round[ucFrameAddrBuff];
    if (ucRoundCount == 0)
    {
        int8_t i = ucRoundFlag;
        if (i != 8)
        {
            i++;
        }
        for (; i >= 0; i--)
        {
            pRou = &Round[ucFrameAddrBuff];
            ucOldFrameAddr = ucFrameAddrBuff;
            vVfdGetNextFrameAddr(&ucFrameAddrBuff);
            pRou->Frame = ucSpeed;
            pRou->Next = ucFrameAddrBuff;
            pRou->Data = CYCLETab[i];
        }
        pRou->Next = ucOldFrameAddr;
        pRou->Frame = 0;
        ucRoundFlag = 0;
    }
    else
    {
        ucRoundFlag++;
        if (ucRoundFlag == 9)
        {
            ucRoundFlag = 0;
        }
        pRou = &Round[ucFrameAddrBuff];
        pRou->Frame = 0;
        pRou->Next = ucFrameAddrBuff;
        pRou->Data = CYCLETab[ucRoundFlag];
        if (ucRoundFlag == 7)
        {
            return 1;
        }
    }
    return 0;
}

static void vVfdVboxRand(void)
{
    ucElementUpDataFlag[eVboxup] = 1;
    const uint8_t ucSpeed = 3;
    static uint16_t usNowVboxData, usState = 0;
    uint8_t ucFrameAddrBuff = ucFrameHeaderAddrS[eVboxup], ucOldFrameAddr = 0;
    xVfdVboxupType *pBox = &Vboxup[ucFrameAddrBuff];
    uint16_t usNewVboxDataBuff = rand();
    uint32_t ulNewVboxData = 0;
    switch (usState)
    {
    case 0:
        ulNewVboxData = usNowVboxData | usNewVboxDataBuff << 16;
        for (uint8_t ucFor = 0; ucFor < 4; ucFor++)
        {
            pBox = &Vboxup[ucFrameAddrBuff];
            ulNewVboxData = ulNewVboxData >> 4;
            pBox->Data = ulNewVboxData;
            pBox->Frame = ucSpeed;
            ucOldFrameAddr = ucFrameAddrBuff;
            vVfdGetNextFrameAddr(&ucFrameAddrBuff);
            pBox->Next = ucFrameAddrBuff;
        }
        break;
    case 1:
        ulNewVboxData = pBox->Data;
        for (uint8_t ucFor = 0; ucFor < 4; ucFor++)
        {
            pBox = &Vboxup[ucFrameAddrBuff];
            pBox->Data = ((ulNewVboxData >> 1) & 0b0111011101110111) | ((usNewVboxDataBuff << ucFor) & 0b1000100010001000);
            ulNewVboxData = pBox->Data;
            pBox->Frame = ucSpeed;
            ucOldFrameAddr = ucFrameAddrBuff;
            vVfdGetNextFrameAddr(&ucFrameAddrBuff);
            pBox->Next = ucFrameAddrBuff;
        }
        break;
    case 2:
        ulNewVboxData = usNowVboxData << 16 | usNewVboxDataBuff;
        for (uint8_t ucFor = 0; ucFor < 4; ucFor++)
        {
            pBox = &Vboxup[ucFrameAddrBuff];
            ulNewVboxData = ulNewVboxData << 4;
            pBox->Data = ulNewVboxData >> 16;
            pBox->Frame = ucSpeed;
            ucOldFrameAddr = ucFrameAddrBuff;
            vVfdGetNextFrameAddr(&ucFrameAddrBuff);
            pBox->Next = ucFrameAddrBuff;
        }
        break;
    case 3:
        ulNewVboxData = pBox->Data;
        for (uint8_t ucFor = 0; ucFor < 4; ucFor++)
        {
            pBox = &Vboxup[ucFrameAddrBuff];
            pBox->Data = ((ulNewVboxData << 1) & 0b1110111011101110) | ((usNewVboxDataBuff >> ucFor) & 0b0001000100010001);
            ulNewVboxData = pBox->Data;
            pBox->Frame = ucSpeed;
            ucOldFrameAddr = ucFrameAddrBuff;
            vVfdGetNextFrameAddr(&ucFrameAddrBuff);
            pBox->Next = ucFrameAddrBuff;
        }
        break;
    default:
        break;
    }
    usNowVboxData = pBox->Data;
    pBox->Frame = 0;
    pBox->Next = ucOldFrameAddr;
    usState++;
    if (usState == 4)
    {
        usState = 0;
    }
}

void vVfdChangeOptions(uint8_t Atarget, uint8_t Btarget)
{
    static uint8_t ucDevA = 1, ucDevB = 1;
    const uint8_t ucSpeed = 3;
    uint8_t ucFrameAddrBuff = ucFrameHeaderAddrS[eText], //当前帧
        ucOldFrameAddr = 0;                              //原帧
    xVfdTextType *pText;

    pText = &Text[ucFrameAddrBuff];
    vVfdGetNextFrameAddr(&ucFrameAddrBuff);
    pText->Next = ucFrameAddrBuff;
    pText->Frame = ucSpeed;
    if (Atarget < 1 || Atarget > 5)
    {
        Atarget = 1;
    }
    if (Btarget < 1 || Btarget > 3)
    {
        Btarget = 1;
    }
    for (uint8_t ucFor = 0; (ucDevA != Atarget) || (ucDevB != Btarget); ucFor++)
    {
        pText = &Text[ucFrameAddrBuff];
        if (ucDevA != Atarget)
        {
            ucDevA++;
        }
        if (ucDevA == 6)
        {
            ucDevA = 1;
        }
        if (ucDevB != Btarget)
        {
            ucDevB++;
        }
        if (ucDevB == 4)
        {
            ucDevB = 1;
        }
        pText->data = 1 << (ucDevA - 1) | 1 << (ucDevB - 1 + 5);
        ucOldFrameAddr = ucFrameAddrBuff;
        vVfdGetNextFrameAddr(&ucFrameAddrBuff);
        pText->Next = ucFrameAddrBuff;
        pText->Frame = ucSpeed;
    }
    pText->Next = ucOldFrameAddr;
    pText->Frame = 0;
    ucElementUpDataFlag[eText] = 1;
}

static void vVfdPutWeek(uint8_t ucWeek)
{ //显示星期
    // const char cWeekBuff[][7] = {"  MON", " TUES", "  WED", " THUR",
    //                              "  FRI", "  SAT", "  SUN"};
    char cBuff[5];
    sprintf(cBuff, "WEEK%u", ucWeek);
    vVfdNewMiAnimation(cBuff);
}
static void vVfdPutYear(uint32_t Year)
{ //显示年份
    char cBuff[5];
    sprintf(cBuff, " %u", Year + 1900);
    vVfdNewMiAnimation(cBuff);
}

static void vVfdPutMonDay(uint32_t Month, uint32_t Day)
{ //显示月份+日期
    char cBuff[5];
    sprintf(cBuff, "%02u%c%02u", Month, 11, Day);
    vVfdNewMiAnimation(cBuff);
}

static void vVfdPutHyxxhNextYearDays(struct tm *xTimeInfo, const char cn, uint32_t ulm, uint32_t uld)
{ //显示月份+日期

    uint32_t ulDay;
    time_t now = 0, t1 = 0;
    struct tm NowTimeInfo = {0};
    NowTimeInfo = *xTimeInfo;
    NowTimeInfo.tm_year = NowTimeInfo.tm_year;
    NowTimeInfo.tm_mday = uld;
    NowTimeInfo.tm_mon = ulm - 1;
    if ((xTimeInfo->tm_mon < ulm - 1) || (xTimeInfo->tm_mon == ulm - 1 && xTimeInfo->tm_mday < uld))
    {
        NowTimeInfo.tm_year = NowTimeInfo.tm_year - 1;
    }

    t1 = mktime(&NowTimeInfo); //2011-12-6
    now = mktime(xTimeInfo);

    ulDay = (now - t1) / (24 * 3600);
    char cBuff[5];
    sprintf(cBuff, "%c_%03u", cn, ulDay);
    vVfdNewMiAnimation(cBuff);
}

static void vVfdPutHyxxhYear(struct tm *xTimeInfo)
{ //显示月份+日期
    uint32_t ulYear, ulDay;
    time_t now = 0, t1 = 0;
    struct tm NowTimeInfo = {0};
    NowTimeInfo = *xTimeInfo;
    NowTimeInfo.tm_mday = 13;
    NowTimeInfo.tm_mon = 2;
    ulYear = xTimeInfo->tm_year - 116;
    if ((xTimeInfo->tm_mon < 2) || (xTimeInfo->tm_mon == 2 && xTimeInfo->tm_mday < 13))
    {
        NowTimeInfo.tm_year = NowTimeInfo.tm_year - 1;
        ulYear--;
    }

    t1 = mktime(&NowTimeInfo); //2011-12-6
    now = mktime(xTimeInfo);

    ulDay = (now - t1) / (24 * 3600);
    char cBuff[5];
    sprintf(cBuff, "%01u_%03u", ulYear, ulDay);
    vVfdNewMiAnimation(cBuff);
}

//更新时间
void vVfdTimeUpData(long ucHH, long ucMM, long ucSS)
{
    ucElementUpDataFlag[eNum] = 1;
    const uint8_t ucSpeed = 3;
    uint8_t ucBuff[6];
    ucBuff[0] = ucNumTab[ucHH / 10];
    ucBuff[1] = ucNumTab[ucHH % 10];
    ucBuff[2] = ucNumTab[ucMM / 10];
    ucBuff[3] = ucNumTab[ucMM % 10];
    ucBuff[4] = ucNumTab[ucSS / 10];
    ucBuff[5] = ucNumTab[ucSS % 10];

    uint8_t ucFrameAddrBuff = ucFrameHeaderAddrS[eNum], ucOldFrameAddr = 0;
    xVfdNumType *pNum = &Num[ucFrameAddrBuff];
    uint8_t ucNumUpDataState[6];
    uint8_t ucFor, ucFora;
    for (ucFora = 0; ucFora < 6; ucFora++)
    {
        ucNumUpDataState[ucFora] = (pNum->Data[ucFora] == ucBuff[ucFora]);
    }

    for (ucFor = 3; ucFor < 8; ucFor++)
    {
        pNum = &Num[ucFrameAddrBuff];
        ucOldFrameAddr = ucFrameAddrBuff;
        vVfdGetNextFrameAddr(&ucFrameAddrBuff);
        pNum->Next = ucFrameAddrBuff;
        pNum->Frame = ucSpeed;
        pNum->Dot = 0;
        for (ucFora = 0; ucFora < 6; ucFora++)
        {
            if (ucNumUpDataState[ucFora] == 1)
            {
                pNum->Data[ucFora] = ucBuff[ucFora];
            }
            else
            {
                pNum->Data[ucFora] = ucBuff[ucFora] >> (7 - ucFor);
            }
        }
    }
    pNum->Frame = 0;
    pNum->Dot = 0b1111;
    pNum->Next = ucOldFrameAddr;
}

/*

void Vfd_PutMonDay(uint8_t Month, uint8_t Day) { //显示月份+日期
  MiNEW[0] = MiTab[Month / 10];
  MiNEW[1] = MiTab[Month % 10];
  MiNEW[2] = MiTab[11];
  MiNEW[3] = MiTab[Day / 10];
  MiNEW[4] = MiTab[Day % 10];
}

void Vfd_CYCLEMt(uint8_t RoundCount, uint8_t Speed) { //圆盘
  int i;
  if (RoundCount == 0) {
    for (i = 8; i >= 0; i--) {
      vVfdReFush(VROUND, CYCLETab[i]);
      vTaskDelay(Speed / portTICK_RATE_MS);
    }
  } else {
    vVfdReFush(VROUND, CYCLETab[RoundCount]);
  }
}

void VfdWMt(uint8_t Speed) {
  int i;
  for (i = 0; i < 11; i++) {
    vVfdReFush(VROUND, LoadTab[i]);
    vTaskDelay(Speed / portTICK_RATE_MS);
  }
}

void Vfd_ChType(uint8_t Type, uint8_t Error) {
  uint8_t i;
  if (Error) {
    for (i = 0; i < 5; i++) {
      vVfdReFush(CType[i], 0xFF);
    }
  } else if (Type < 5) {
    for (i = 0; i < 5; i++) {
      if (i == Type) {
        vVfdReFush(CType[i], 0xFF);
      } else {
        vVfdReFush(CType[i], 0x00);
      }
    }
  } else {
    for (i = 0; i < 5; i++) {
      vVfdReFush(CType[i], 0x00);
    }
  }
}

void Vfd_CTType(uint8_t Type, uint8_t Error) {
  uint8_t i;
  if (Error) {
    for (i = 0; i < 3; i++) {
      vVfdReFush(TType[i], 0xFF);
    }
  } else if (Type > 4) {
    for (i = 4; i < 8; i++)
      if (i == Type) {
        vVfdReFush(TType[i - 5], 0xFF);
      } else {
        vVfdReFush(TType[i - 5], 0x00);
      }
  } else {
    for (i = 0; i < 3; i++) {
      vVfdReFush(TType[i], 0x00);
    }
  }
}*/

//闲置轮播画面
static void vVfdCarousel(struct tm *xTimeInfo)
{
    static uint8_t ucVfdTaskState, ucRoundLoadBuff;
    vVfdVboxRand();
    vVfdTimeUpData(xTimeInfo->tm_hour, xTimeInfo->tm_min, xTimeInfo->tm_sec);
    if (ucRoundLoadBuff)
    {
        switch (ucVfdTaskState)
        {
        case 0:
            vVfdPutYear(xTimeInfo->tm_year);
            break;
        case 1:
            vVfdPutMonDay(xTimeInfo->tm_mon + 1, xTimeInfo->tm_mday);
            break;
        case 2:
            vVfdPutWeek(xTimeInfo->tm_wday);
            break;
        case 3:
            vVfdPutHyxxhYear(xTimeInfo);
            break;
        case 4:
            vVfdPutHyxxhNextYearDays(xTimeInfo, 'A', 3, 13);
            break;
        case 5:
            vVfdPutHyxxhNextYearDays(xTimeInfo, 'B', 1, 29);
            break;
        default:
            break;
        }
        ucVfdTaskState++;
        if (ucVfdTaskState == 6)
        {
            ucVfdTaskState = 0;
        }
    }
    ucRoundLoadBuff = ucVfdRoundLoad(1 - ucRoundLoadBuff);
}
// Vfd任务函数
static void vVfdTask(void *arg)
{
    printf("[VfdFrameTask][Start]\n");
    vVfdInit();
    vVfdDimming(4);
    vVfdNewMiAnimation("HELLO");
    static TickType_t xVfd_taskWakeTime;
    xVfd_taskWakeTime = xTaskGetTickCount();
    for (;;)
    {
        vVfdFrameUpData();
        vTaskDelayUntil(&xVfd_taskWakeTime, (1000 / FPS) DMS);
    }
}

static void vVfdTimeTask(void *arg)
{
    static TickType_t xVfd_TimeTaskWakeTime;
    xVfd_TimeTaskWakeTime = xTaskGetTickCount();

    time_t now = 0;
    struct tm xTimeInfo = {0};
    printf("[VfdTimeTask][DelayStnp]\n");
    do
    {
        vTaskDelayUntil(&xVfd_TimeTaskWakeTime, 1000 DMS);
        time(&now); //获取网络时间， 64bit的秒计数
        localtime_r(&now, &xTimeInfo);
    } while (xTimeInfo.tm_year == 70);
    vMainStopSntp();
    vVfdSntpOk();
    printf("[VfdTimeTask][Start]\n");
    for (;;)
    {
        time(&now);                    //获取网络时间， 64bit的秒计数
        localtime_r(&now, &xTimeInfo); //时间结构体转换
        vVfdCarousel(&xTimeInfo);
        vTaskDelayUntil(&xVfd_TimeTaskWakeTime, 1000 DMS);
    }
}

// 6311任务启动函数
void vVfdTaskInit(void)
{
    vMyGpioInit(1ULL << VFDPW | 1ULL << DATA | 1ULL << CLK | 1ULL << STB);
    printf("GPIO_INIT!\n");
    xTaskCreate(vVfdTask, "Vfd_task", 1536, NULL, 1, NULL);
}

void vVfdTimeTaskInit(void)
{
    xTaskCreate(vVfdTimeTask, "time_task", 2048, NULL, 2, NULL);
}
