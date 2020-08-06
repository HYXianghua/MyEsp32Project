#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "playmv.h"
#include "esp_attr.h"
#include "Screen_drive/Screen_drive.h"

enum
{
  FileClose,
  FileOpen
} FileStateEnum;

static uint32_t fileSize;
static uint16_t FileState;

/*
playMod=0：循环  
playMod>0: 播放循环playMod次
*/
static uint16_t playMod;
static FILE *pFile; //文件指针

void IRAM_ATTR playMvUpData(frame_type *p)
{
  if (fileSize == 0)
  {
    if (playMod == 1)
    {
      closeMv();
      return;
    }
    else
    {
      fseek(pFile, 0, SEEK_END);
      fileSize = ftell(pFile);
      fseek(pFile, 0, SEEK_SET);
      if (playMod)
      {
        playMod--;
      }
    }
  }
  fread((*p), sizeof(frame_type), 1, pFile);
  fileSize -= sizeof(frame_type);
}

static void hd(frame_type *pDisplayBuff)
{
  uint8_t L = 1;
  uint8_t buff = 0XA;
  for (uint16_t l = 0; l < LINE_NUMBER; l++)
  {
    if (l > (LINE_NUMBER - 10))
    {
      (*pDisplayBuff)[l][1] = ((L) | 0B11100000) | (buff << 24);
    }
    else
    {
      (*pDisplayBuff)[l][1] = ((L) | 0B11100000) | (buff << 8) | (buff << 16) | (buff << 24);
    }

    for (uint16_t d = 0; d < COUNT_NUMBER + 2; d++)
    {
      if (d == 0 || d == 25)
      {
        (*pDisplayBuff)[l][d] = 0;
      }
      else
      {
        (*pDisplayBuff)[l][d] = (*pDisplayBuff)[l][1] + (uint8_t)(d >> 1);
      }
    }
    //(*pDisplayBuff)[l][COUNT_NUMBER + 1] = UINT32_MAX;
  }
}

void closeMv(void)
{
  fclose(pFile);
  pFile = NULL;
  RegisterPlayer(ShowVoid);
}

int playMv(char *url, uint16_t mod)
{
  if (pFile != NULL)
  {
    closeMv();
  }
  pFile = fopen(url, "rb");
  if (pFile == NULL)
  {
    printf("platMv open err!\n");
    return 1;
  }
  printf("platMv open!\n");
  playMod = mod;
  fseek(pFile, 0, SEEK_END);
  fileSize = ftell(pFile);
  fseek(pFile, 0, SEEK_SET);
  printf("[palyMv]: name: %s,file siez=%dKByte\n", url, fileSize >> 10);
  RegisterPlayer(playMvUpData);
  return 0;
}
