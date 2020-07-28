#include <Uefi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Pi/PiDxeCis.h>
#include <Protocol/MpService.h>



extern UINT64 TinyReadMsr(unsigned int index);

// /cvbin/edk2/MdePkg/Library/BaseLib/X64/GccInline.c
//1, use edk2 api
//MdePkg/Include/Library/BaseLib.h
//MdePkg/Library/BaseLib/X64/GccInline.c
//value = AsmReadMsr64(msr);
//hi = value >> 32;
//low = (unsigned int)(value & 0xffffffff);

//2, use self gnu asm
//value = TinyReadMsr(msr);
//hi = value >> 32;
//low = (unsigned int)(value & 0xffffffff);

//3, use self gnu inline 
//__asm__ __volatile__ ("rdmsr":"=d"(hi),"=a"(low):"c"(msr));

typedef struct THRAED_PARAM
{
	UINT32 msr;
}THRAED_PARAM;


EFI_MP_SERVICES_PROTOCOL *g_pmpp;

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

void EFIAPI thread_rdmsr(void* param)
{
	THRAED_PARAM* tparam = (THRAED_PARAM*)param;
	UINT64 value =  AsmReadMsr64(tparam->msr);
	UINT32 low = (UINT32)(value & 0xffffffff);
	UINT32 hi = value >> 32;
	UINTN core_id;
	g_pmpp->WhoAmI(g_pmpp, &core_id);
	printf("CORE_%d MSR 0x%04X = 0x%08X 0x%08X\n", core_id, tparam->msr, hi, low);
}

int main (int argc, char **argv)
{
	UINT32 msr;
	//unsigned long long  value;
    UINT32 hi, low;
	EFI_STATUS status;
	EFI_PROCESSOR_INFORMATION processor_info;
	UINTN processors_num, processors_en_num;
	THRAED_PARAM thread_param;
	UINT32 iloop, jloop;
	UINTN processor_id;
	status = gBS->LocateProtocol(&gEfiMpServiceProtocolGuid,NULL,(void**)&g_pmpp);
	status = g_pmpp -> GetNumberOfProcessors(g_pmpp, &processors_num, &processors_en_num);

	/* 
	printf("processors_num is %d\n", processors_num);
	for(iloop = 0; iloop < processors_num; iloop++)
	{
		status = g_pmpp->GetProcessorInfo(g_pmpp,iloop,&processor_info);
		printf("processor number %d:\t", iloop);
		printf("processor id %d\t",processor_info.ProcessorId);
		//status[0] 0 AP, 1 BSP; status[1] 0 disable, 1 enable; status[2] 0 fault, 1 normal
		printf("processor statusflag 0x%x\t", processor_info.StatusFlag); 
		printf("processor location package %d core %d thread %d\n", processor_info.Location.Package, processor_info.Location.Core, processor_info.Location.Thread);

	}
	*/

	if(2 == argc)
	{
		// all cores
		msr = str2llhex(argv[1]);
		for(iloop = 0; iloop < processors_num; iloop++)
		{
			g_pmpp->GetProcessorInfo(g_pmpp,iloop,&processor_info);
			if(processor_info.StatusFlag & 0x01)
			{
				//BSP
				thread_param.msr = msr;
				thread_rdmsr(&thread_param);
				g_pmpp->StartupAllAPs(g_pmpp,thread_rdmsr,TRUE,NULL,0,(void*)&thread_param,NULL);
				break;
			}
		}
	}
	else if (3 == argc)
	{
		for(iloop = 1; iloop < argc; iloop++)
		{
			if(argv[iloop][0] == '=')
			{
				processor_id =  str2llhex(&argv[iloop][1]);
			}
			else
			{
				msr = str2llhex(argv[iloop]);
			}
		}
		thread_param.msr = msr;

		for(iloop = 0; iloop < processors_num; iloop++)
		{
			g_pmpp->GetProcessorInfo(g_pmpp,iloop,&processor_info);
			if(processor_info.StatusFlag & 0x01)
			{
				//BSP
				if(processor_id == iloop)
				{
					thread_rdmsr(&thread_param);
				}
				else
				{
					g_pmpp->StartupThisAP(g_pmpp,thread_rdmsr,processor_id,0,0,(void*)&thread_param,NULL);
				}
				break;
			}
		}	
	}
	else
	{
		printf("useage: rdmsr.efi  msr \n");
		printf("useage: rdmsr.efi  =id msr \n");
	}
	
	return 0;
}
