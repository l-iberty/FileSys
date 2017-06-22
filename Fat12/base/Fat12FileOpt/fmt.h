#ifndef FMT_H
#define FMT_H

typedef unsigned char	u8;
typedef unsigned short u16;
typedef unsigned int		u32;


/* 引导扇区结构体定义 */
typedef struct {
	u8						BS_jmpBoot[3];
	u8						BS_OEMName[8];
	u16						BPB_BytesPerSec;
	u8						BPB_SecPerClus;
	u16						BPB_RsvdSecCnt;
	u8						BPB_NumFATs;
	u16						BPB_RootEntCnt;
	u16						BPB_TotSec16;
	u8						BPB_Media;
	u16						FATSz16;
	u16						BPB_SecPerTrk;
	u16						BPB_NumHeads;
	u32						BPB_HiddSec;
	u32						BPB_TotSec32;
	u8						BS_DrvNum;
	u8						BS_Reserved1;
	u8						BS_BootSig;
	u32						BS_VolID;
	u8						BS_VolLab[11];
	u8						BS_FileSysType[8];
	u8						BS_CodeAndOthers[448];
	u8						BS_EndFlag[2];
} Fat12Boot;

/* 引导扇区各字段偏移 */
#define OFF_BS_jmpBoot					0
#define OFF_BS_OEMName				3
#define OFF_BPB_BytesPerSec			11
#define OFF_BPB_SecPerClus			13
#define OFF_BPB_RsvdSecCnt			14
#define OFF_BPB_NumFATs				16
#define OFF_BPB_RootEntCnt			17
#define OFF_BPB_TotSec16				19
#define OFF_BPB_Media						21
#define OFF_FATSz16							22
#define OFF_BPB_SecPerTrk				24
#define OFF_BPB_NumHeads			26
#define OFF_BPB_HiddSec					28
#define OFF_TotSec32						32
#define OFF_BS_DrvNum					36
#define OFF_BS_Reserved1				37
#define OFF_BS_BootSig					38
#define OFF_BS_VolID							39
#define OFF_BS_VolLab						43
#define OFF_FileSysType					54
#define OFF_BS_CodeAndOthers		62
#define OFF_BS_EndFlag					510

/* FAT1和FAT2的起始扇区号与偏移 */
#define FAT1_NO								0x1
#define FAT2_NO								0xA
#define OFF_FAT1								0x1*0x200
#define OFF_FAT2								0xA*0x200

/* 根目录区的起始扇区号与偏移 */
#define RootDir_NO							19
#define OFF_RootDir							19*0x200

/* 根目录区各条目的偏移 */
#define OFF_DIR_Name				0
#define OFF_DIR_Attr					0xB
#define OFF_DIR_Reserved		0xC
#define OFF_DIR_WrtTime			0x16
#define OFF_DIR_WrtDate			0x18
#define OFF_DIR_FstClus			0x1A
#define OFF_DIR_FileSize			0x1C

/* 数据区的扇区号与偏移 */
#define DATA_NO						33
#define OFF_DATA						33*0x200

/* 文件属性 */
#define FILE_NORMAL				0x0020
#define FILE_DIR							0x0010

/* 根目录区中的条目结构体定义 */
typedef struct
{
	u8			DIR_Name[0xB];
	u8			DIR_Attr;
	u8			DIR_Reserved[0xA];
	u8			DIR_WrtTime[2];
	u8			DIR_WrtDate[2];
	u16			DIR_FstClus;
	u32			DIR_FileSize;
} RootDirEnt;


class Fat12Image {
private:
	Fat12Boot m_Boot;
	char *m_lpBuffer;	// 文件数据缓存
	u16 *m_FatEnt;

	void GetFatEntry();
	int GetFatClusCnt(); /* FAT表指明了多少个簇? */
	void PrintFileData(u16 FstClus, u8 Attr, u32 FileSize);

public:
	Fat12Image(char *FileName);
	~Fat12Image();

	void GetBootData(Fat12Boot *pBoot);
	void ProcessAllFiles();
};

#endif // FMT_H