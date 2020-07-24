#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "clock.h"

const uint8_t strnum[] = {
    0xFE, 0x82, 0x82, 0xFE, //0
    0x00, 0x84, 0xFE, 0x80, //1
    0xF2, 0x92, 0x92, 0x9E, //2
    0x92, 0x92, 0x92, 0xFE, //3
    0x1E, 0x10, 0x10, 0xFE, //4
    0x9E, 0x92, 0x92, 0xF2, //5
    0xFE, 0x92, 0x92, 0xF2, //6
    0x02, 0x02, 0x02, 0xFE, //7
    0xFE, 0x92, 0x92, 0xFE, //8
    0x1E, 0x12, 0x12, 0xFE, //9
};

void vClockInit(frame_type *pDisplayBuff)
{
  ScreenLRGBUnion BUFF = {0};
  //复制亮度
  memcpy((*pDisplayBuff), BrightnessMatrix, sizeof(BrightnessMatrix));

  BUFF.LRGB.L = 0;
  BUFF.LRGB.R = 0XFF;
  BUFF.LRGB.G = 0XFF;
  BUFF.LRGB.B = 0XFF;
  //中点 1-5
  for (uint16_t d = 4; d <= 5; d++)
  {
    (*pDisplayBuff)[0][d] |= BUFF.DATA;
  }
  //外线 32
  (*pDisplayBuff)[0][32] |= BUFF.DATA;
  //复制到整圈
  for (uint16_t i = 1; i < LINE_NUMBER; i++)
  {
    memcpy((*pDisplayBuff) + i, (*pDisplayBuff), sizeof(BrightnessMatrix));
  }
  //字符25-31
  BUFF.LRGB.R = 0;
  BUFF.LRGB.G = 0;
  BUFF.LRGB.B = 0XFF;
  for (uint16_t z = 0; z < 12; z++)
  {
    uint16_t line = 0;
    for (uint16_t l = z * 16; l < ((z * 16) + 8); l++)
    {
      for (uint16_t d = 0; d < 7; d++)
      {
        if (z < 9)
        {
          if (line > 2 && line < 7)
          {
            BUFF.LRGB.B = 0XFF * ((strnum[(z + 1) * 4 + line - 3] >> (7 - d)) % 2);
          }
          else
          {
            BUFF.LRGB.B = 0;
          }
        }
        else
        {
          if (line < 4)
          {
            BUFF.LRGB.B = 0XFF * ((strnum[4 + line] >> (7 - d)) % 2);
          }
          else
          {
            BUFF.LRGB.B = 0XFF * ((strnum[(z - 9) * 4 + line - 4] >> (7 - d)) % 2);
          }
        }
        (*pDisplayBuff)[l][25 + d] |= BUFF.DATA;
      }
      line++;
    }
  }
}

void vClockUpData(frame_type *p, uint16_t H, uint16_t M, uint16_t S)
{
  static uint16_t oldH[2], oldM[2], oldS[2];
  static uint16_t pf;
  H %= 12; //24化12小时制
  uint16_t angS = S * 16 / 5, angM = M * 16 / 5 + angS / 60, angH = H * 16 + angM / 60;
  angS = (angS + 8) % LINE_NUMBER;
  angS = (angM + 8) % LINE_NUMBER;
  angS = (angH + 8) % LINE_NUMBER;
  ScreenLRGBUnion BUFF = {0};
  //擦除
  // for (uint16_t i = 6; i <= 20; i++)
  // {
  //   (*p)[oldH[pf]][i] = BrightnessMatrix[i];
  //   (*p)[oldM[pf]][i] = BrightnessMatrix[i];
  //   (*p)[oldS[pf]][i] = BrightnessMatrix[i];
  // }

  for (uint16_t l = 0; l < LINE_NUMBER; l++)
  {
    memcpy((*p)[l] + 6, BrightnessMatrix + 6, 20 - 6);
    // for (uint16_t i = 6; i < 20; i++)
    // {
    //   (*p)[l][i] = BrightnessMatrix[i];
    // }
  }

  pf = !pf;
  oldH[pf] = angH;
  oldM[pf] = angM;
  oldS[pf] = angS;
  BUFF.LRGB.R = 0;
  BUFF.LRGB.G = 0XFF;
  BUFF.LRGB.B = 0;
  for (uint16_t i = 7; i < 14; i++)
  {
    (*p)[angH][i] |= BUFF.DATA;
  }
  BUFF.LRGB.R = 0XFF;
  BUFF.LRGB.G = 0;
  BUFF.LRGB.B = 0;
  for (uint16_t i = 7; i < 16; i++)
  {
    (*p)[angM][i] |= BUFF.DATA;
  }
  BUFF.LRGB.R = 0;
  BUFF.LRGB.G = 0;
  BUFF.LRGB.B = 0XFF;
  for (uint16_t i = 7; i < 20; i++)
  {
    (*p)[angS][i] |= BUFF.DATA;
  }
}
