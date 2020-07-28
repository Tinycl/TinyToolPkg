#include <Uefi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>


UINT64 str2llhex(char* ptr)
{
	UINT64 data;
	UINT32 i;
	if(((ptr[0] == '0') && (ptr[1] == 'x')) || ((ptr[0] == '0') && (ptr[1] == 'X'))) 
	{
		ptr += 2;
	}
	for(i = 0, data = 0; (i < 16 && *ptr != 0); i++, ptr++)
	{
		data <<= 4;
		if(*ptr >= '0' && *ptr <= '9')
		{
			data |= (*ptr - '0');
		}
		else if(*ptr >= 'A' && *ptr <= 'F')
		{
			data |= (*ptr - 'A' + 10);
		}
		else if(*ptr >= 'a' && *ptr <= 'f')
		{
			data |= (*ptr - 'a' + 10);
		}
	}
	return data;
}

int main (int argc, char **argv)
{
	UINT32 port;
	UINT8 value;
	if(2 == argc)
	{
		port = str2llhex(argv[1]);
		value = IoRead8(port);
		printf("IO port 0x%x, value 0x%x\n", port, value);
	}
	else
	{
		printf("useage: inb.efi port  \n");
	}
	
	return 0;
}

