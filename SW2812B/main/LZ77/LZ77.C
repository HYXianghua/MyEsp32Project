// #include <stdio.h>
// #include <stdint.h>
// #include <string.h>
// #include "LZ77.H"
// #include "Screen_drive/Screen_drive.h"

// static uint32_t seek;
// static uint16_t FileState;
// static FILE *pFile; //文件指针

// enum
// {
//   FileClose,
//   FileOpen
// } FileStateEnum;

// int LZ77OpenFile(char *url)
// {
//   uint8_t databuff[20];
//   FILE *data = fopen(url, "rb");
//   if (data == NULL)
//   {
//     return 1;
//   }
//   seek = 0;
//   FileState = FileOpen;
//   return 0;
// }

// void LZ77CloseFile(void)
// {
//   fclose(pFile);
//   pFile = NULL;
// }

// void LZ77Example(void)
// {
// }
// typedef struct
// {
//   uint8_t len;
//   uint8_t location;
//   uint8_t data;
// } LZ77Type;
// //XXXXX....XXXXX    n=50
// //             L    =n-1
// //
// void LZ77ReadFrame(frame_type *pOldFrame, frame_type *pNewFrame)
// {
//   uint16_t np = (uint8_t *)(pNewFrame) + sizeof(frame_type); //终点地址
//   uint8_t *pWindou = (uint8_t *)(pOldFrame) + (sizeof(frame_type) - 256);
//   uint8_t *seek = pNewFrame, seek_n;
//   uint8_t pDataA[2], pDataB[1], lo, le;
//   while (seek < np)
//   {
//     //取两个数据
//     fread(pDataA, 1, 2, pFile);
//     /*判断是否匹配 (首位)*/
//     if (pDataA[0] >> 7) //匹配
//     {
//       fread(pDataB, 1, 1, pFile);
//       le = pDataA[0] - 127;
//       lo = pDataA[1];
//       uint8_t flagBit = 1;
//       uint8_t needLen;
//       if (flagBit)
//       {
//         needLen = lo + le + seek_n;
//         if (needLen > 255)
//         {
//           memcpy(seek, pWindou + seek_n, 255 - seek_n);
//           seek += (255 - seek_n);
//           memcpy(seek, pNewFrame, needLen % 256);
//           seek += (needLen % 256);
//           *seek = pDataB;
//           seek += le + 1;
//         }
//         else
//         {
//           memcpy(seek, pWindou + seek_n, needLen - seek_n);
//         }
//         seek_n = seek - pNewFrame;
//         if (seek_n > 255)
//         {
//           flagBit = 0;
//         }
//       }
//       else
//       {
//         needLen = lo + le;
//         memcpy(seek, pWindou + seek_n, needLen);
//         seek += needLen;
//         (*seek) = pDataB;
//         seek++;
//         seek_n += needLen + 1;
//       }
//     }
//     else
//     {
//       //不匹配
//       memset(seek, pDataA[1], pDataA[0]);
//       seek += pDataA[0];
//     }
//   }
// }