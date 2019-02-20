//
//  main.cpp
//  CreateImg
//
//  Created by 辛济远 on 2/18/19.
//  Copyright © 2019 辛济远. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef unsigned char UCHAR;

struct FileInfo {
    char name[8], ext[3];
    char attr;
    char keep[10];
    long wrtTime;
    int FstClus;
    long fileSize;
};


char* getBootSector(int argc, char *argv[]) {
    // 读取二进制引导扇区文件
    FILE *fp = NULL;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-')
            continue;
        if (argv[i][1] == 'B') {
            fp = fopen(argv[i + 1], "rb");
            break;
        }
    }
    if (fp == NULL)
        return NULL;
    char *bootSector =(char *) malloc(512);
    if (fread(bootSector, sizeof(unsigned char), 512, fp) < 512)
        bootSector = NULL;
    fclose(fp);
    return bootSector;
}


char* getOutputFile(int argc, char *argv[]) {
    char* s = NULL;
    int len;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-')
            continue;
        if (argv[i][1] == 'o') {
            len = strlen(argv[i + 1]);
            s = (char *) malloc(len * sizeof(char));
            strcpy(s, argv[i + 1]);
            break;
        }
    }
    return s;
}

char* getSector(int startAddr, int endAddr) {
    if (startAddr < endAddr) {
        char *sec = (char*) malloc(endAddr - startAddr);
        memset(sec, 0, endAddr - startAddr);
        return sec;
    }
    return NULL;
}

long getTime() {
    time_t t = time(NULL);
    return t;
}



struct FileInfo getFileInfo(char* fileName) {
    struct FileInfo res;
    int len = strlen(fileName);
    int i;
    for (i = len - 1; i >= 0; i--)
        if (fileName[i] == '\\' || fileName[i] == '/')
            break;
    i++;
    memset(&res.name, 0, 8 * sizeof(char));
    memset(&res.ext, 0, 3 * sizeof(char));
    for (int j = 0; i < len && j < 8; i++, j++) {
        if (fileName[i] == '.')
           break;
        res.name[j] = fileName[i];
    }
    while(fileName[i] != '.') i++;
    i++;
    for (int j = 0; i < len && j < 3; i++, j++)
        res.ext[j] = fileName[i];
    res.wrtTime = getTime();
    res.attr = 0x00;
    return res;
}

void wrtRoot(struct FileInfo file, char* start) { // 将文件信息写入root区
    int i = 0;
    strcpy(start, file.name);
    i += 8;
    strcpy(start + i, file.ext);
    i += 3;
    start[i] = file.attr;
    i++;
    strcpy(start + i, file.keep);
    i += 10;
    for (int j = 0; j < 4; j++)
        start[i + j] = ((file.wrtTime >> (j * 8)) & 0x000f);
    i += 4;
    for (int j = 0; j < 2; j++)
        start[i + j] = ((file.FstClus >> (j * 8)) & 0x000f);
    i += 2;
    for (int j = 0; j < 4; j++)
        start[i + j] = ((file.fileSize >> (j * 8)) & 0x000f);
}



void m_readFiles(int argc, char * argv[], char * fat, char * root, char * data, int dataLength) {
    long size;
    FILE *fp;
    char *dataptr = data, *fatptr = fat, *rootptr = root;
    int fc = 2;
    int rest = dataLength;
    long numClus;
    int fatCont; // 写入fat分区的内容
    fat[0] = 0xf0;
    fat[1] = 0xff;
    fat[2] = 0xff;
    // fat分区前两个簇弃用
    fatptr += 3;
    printf("Hello World!\n");
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            i++;
            continue;
        }
        struct FileInfo file = getFileInfo(argv[i]);
        file.FstClus = fc;
        fp = fopen(argv[i], "rb");
        size = fread(dataptr, sizeof(UCHAR), rest, fp);  // 将文件读入缓冲区
        printf("%d\n", size);
        file.fileSize = size;
        fclose(fp);
        wrtRoot(file, root);
        numClus = size / 512 + (size % 512 != 0);
        dataptr = dataptr + 512 * numClus; // 更新dataptr指向下一个簇
      

        printf("File: %s.%s\n", file.name, file.ext);
        printf("Write Time: %04x\n", file.wrtTime);
        printf("File Size: %04x\n", file.fileSize);
        // 开始处理fat区
        numClus += fc;
        for (; fc < numClus; fc++) {
            if (fc < numClus - 1)
                fatCont = fc + 1;
            else
                fatCont = 0x0fff;
            if (fc % 2) {
                *fatptr = *fatptr | ((fatCont & 0x000f) << 4);
                fatptr++;
                *fatptr = fatCont >> 4;
                fatptr++;
            }
            else {
                *fatptr = fatCont & 0x00ff;
                fatptr++;
                *fatptr = fatCont >> 8;
            }
        }
      
    }
}

int main(int argc, char * argv[]) {
  char* bootSector = getBootSector(argc, argv);
  if (!bootSector) {
      printf("Can not find boot sector!\n");
      exit(1);
  }
  int REC = (UCHAR) bootSector[18]  + ((UCHAR) bootSector[19] << 8);
  int BPS = (UCHAR) bootSector[12] + ((UCHAR) bootSector[13] << 8);
  int TS16 = ((UCHAR) bootSector[21] << 8) + (UCHAR) bootSector[20];
  int RDS = ((REC << 5) + BPS - 1) / BPS;
  
  int rootSectorAddr = 0x2600;
  int dataSectorAddr = (19 + RDS) * BPS;
  int tot = BPS * TS16;
  
  char * outFile = getOutputFile(argc, argv);
  char* fat = getSector(1 * BPS, rootSectorAddr);
  char* root = getSector(rootSectorAddr, dataSectorAddr);
  char* data = getSector(dataSectorAddr, tot);
  
  if (!fat) {
      printf("Bad FAT sector!\n");
      exit(1);
  }
  if (!root) {
      printf("Bad ROOT sector!\n");
      exit(1);
  }
  if (!data) {
      printf("Bad DATA sector!\n");
      exit(1);
  }
  if (outFile == NULL)
     outFile = "untitled.img";
  
  m_readFiles(argc, argv, fat, root, data, tot - dataSectorAddr);
  
  FILE *fp = fopen(outFile, "wb");
  fwrite(bootSector, sizeof(char), 512, fp);
  fwrite(fat, sizeof(char), 18 * BPS, fp);
  fwrite(root, sizeof(char), (19 + RDS) * BPS - rootSectorAddr, fp);
  fwrite(data, sizeof(char), tot - dataSectorAddr, fp);
  fclose(fp);
  return 0;
}
