#include "fmt.h"
#include <stdio.h>
#include <Windows.h>

Fat12Image::Fat12Image(char *FileName)
{
	/* 读取文件至缓存 m_lpBuffer */
	HANDLE hFile;
	DWORD cbFileSize;
	DWORD cbRead;

	hFile = CreateFileA(
		FileName,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile Error!\n");
		return;
	}

	cbFileSize = GetFileSize(hFile, NULL);
	m_lpBuffer = (char*)malloc(cbFileSize);
	if (m_lpBuffer == NULL)
	{
		printf("malloc Error!\n");
		return;
	}
	
	if (!ReadFile(hFile, m_lpBuffer, cbFileSize, &cbRead, NULL))
	{
		printf("ReadFile Error!\n");
		return;
	}
	CloseHandle(hFile);
	/* 文件读取结束 */

	GetFatEntry(); /* 获取FAT表 */
	GetBootData(NULL); /* 获取引导扇区数据 */
}

Fat12Image::~Fat12Image()
{
	free(m_lpBuffer);
	free(m_FatEnt);
}

void Fat12Image::GetFatEntry()
{
	int ClusCnt;
	u8 *pch;

	ClusCnt = GetFatClusCnt();
	pch = (u8*)(m_lpBuffer + OFF_FAT1);
	m_FatEnt = (u16*)malloc(ClusCnt*sizeof(u16));

	for (int i = 0;i < ClusCnt - 1;i += 2, pch += 3)
	{
		m_FatEnt[i] = ((*(pch + 1) & 0x0f) << 8) | (*pch);
		if (i + 1 >= ClusCnt) break;
		m_FatEnt[i + 1] = (*(pch + 2) << 4) | ((*(pch + 1) >> 4) & 0x0f);
	}
}

int Fat12Image::GetFatClusCnt()
{
	int ClusCnt;
	u8 *pch;
	u16 nextEnt; /* 等于0时表示FAT表结束 */

	ClusCnt = 0;
	pch = (u8 *)(m_lpBuffer + OFF_FAT1);
	while (1)
	{
		nextEnt = ((*(pch + 1) & 0x0f) << 8) | (*pch);
		if (nextEnt == 0) break;
		ClusCnt++;

		nextEnt = (*(pch + 2) << 4) | ((*(pch + 1) >> 4) & 0x0f);
		if (nextEnt == 0) break;
		ClusCnt++;

		pch += 3;
	}
	return ClusCnt;
}

void Fat12Image::PrintFileData(u16 FstClus, u8 Attr, u32 FileSize)
{
	int BytesPerClus; // 每簇字节数
	int ClusNo; // 簇号
	char *pch;
	u32 cbToPrs;	// 待处理的字节数

	if (Attr == FILE_NORMAL && FileSize > 0)
	{
		printf("This is a normal file, data is as follow:\n");
		BytesPerClus = m_Boot.BPB_BytesPerSec*m_Boot.BPB_SecPerClus;
		ClusNo = FstClus;
		cbToPrs = FileSize;
		do {
			/* 定位到文件数据 */
			pch = m_lpBuffer + OFF_DATA + (ClusNo - 2) * BytesPerClus;

			/**
			* n=循环次数
			* 如果 FileSize < BytesPerClus, 只需处理 FileSize 数量的字节;
			* 如果 FileSize > BytesPerClus, 一次循环只处理一个簇的文件数据
			**/
			int n = (cbToPrs < BytesPerClus) ? cbToPrs : BytesPerClus;
			for (int i = 0;n > 0;i++, n--, cbToPrs--)
			{
				printf("%c", pch[i]);
			}

			ClusNo = m_FatEnt[ClusNo]; // 下一个簇
		} while (ClusNo != 0xfff && cbToPrs > 0);
		printf("\n");
	}
	else if (Attr == FILE_DIR && FileSize == 0)
	{
		printf("This is a dir. ^_^\n");
	}
}

void Fat12Image::GetBootData(Fat12Boot *pBoot)
{
	ZeroMemory(&m_Boot, sizeof(Fat12Boot));
	m_Boot.BPB_BytesPerSec = *(u16*)(m_lpBuffer + OFF_BPB_BytesPerSec);
	m_Boot.BPB_SecPerClus = *(u8*)(m_lpBuffer + OFF_BPB_SecPerClus);
	m_Boot.BPB_RsvdSecCnt = *(u16*)(m_lpBuffer + OFF_BPB_RsvdSecCnt);
	m_Boot.BPB_NumFATs = *(u8*)(m_lpBuffer + OFF_BPB_NumFATs);

	printf("BPB_BytesPerSec = 0x%x\n", m_Boot.BPB_BytesPerSec);
	printf("BPB_SecPerClus = 0x%x\n", m_Boot.BPB_SecPerClus);
	printf("BPB_RsvdSecCnt = 0x%x\n", m_Boot.BPB_RsvdSecCnt);
	printf("BPB_NumFATs = 0x%x\n", m_Boot.BPB_NumFATs);

	if (pBoot != NULL)
		CopyMemory(pBoot, &m_Boot, sizeof(Fat12Boot));
}

void Fat12Image::ProcessAllFiles()
{
	RootDirEnt RootDir;
	char szDIR_Name[0xB + 1];

	for (char *pch = m_lpBuffer + OFF_RootDir; *pch; pch += sizeof(RootDirEnt))
	{
		ZeroMemory(&RootDir, sizeof(RootDirEnt));
		CopyMemory(RootDir.DIR_Name, (pch + OFF_DIR_Name), 0xB);
		RootDir.DIR_Attr = *(u8*)(pch + OFF_DIR_Attr);
		RootDir.DIR_FstClus = *(u8*)(pch + OFF_DIR_FstClus);
		RootDir.DIR_FileSize = *(u32*)(pch + OFF_DIR_FileSize);

		if (RootDir.DIR_FstClus == 0x0000) continue;

		CopyMemory(szDIR_Name, RootDir.DIR_Name, 0xB);
		szDIR_Name[0xB] = 0;
		printf("\nDIR_Name: %s\n", szDIR_Name);
		printf("DIR_Attr: 0x%x\n", RootDir.DIR_Attr);
		printf("DIR_FstClus: 0x%x\n", RootDir.DIR_FstClus);
		printf("DIR_FileSize: 0x%x\n", RootDir.DIR_FileSize);

		PrintFileData(RootDir.DIR_FstClus, RootDir.DIR_Attr, RootDir.DIR_FileSize);
	}
}
