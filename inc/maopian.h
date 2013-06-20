#ifndef __MAOPIAN_H__
#define __MAOPIAN_H__

#define HAV_PORT_NAME L"\\HavPort"

#define MAJ_VERSION 1
#define MIN_VERSION 0
typedef struct _HAVVER
{
	USHORT Major;
	USHORT Minor;
} HAVVER,*PHAVVER;

typedef enum _HAV_COMMAND
{
	SetDir,
	GetVer
} HAV_COMMAND,*PHAV_COMMAND;

#pragma warning(push)
#pragma warning(disable:4200)
typedef struct _COMMAND_MESSAGE
{
	HAV_COMMAND Command;
	ULONG Reserved;	//Alignment on IA64
	UCHAR Data[];
} COMMAND_MESSAGE,*PCOMMAND_MESSAGE;
#pragma warning(pop)


#endif