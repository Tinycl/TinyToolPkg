#include <Uefi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  //isdigit

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Pi/PiDxeCis.h>
#include <Protocol/MpService.h>

typedef struct THRAED_PARAM
{
	UINT32 msr;
}THRAED_PARAM;


EFI_MP_SERVICES_PROTOCOL *g_pmpp;
UINT32 g_sedx = 0, g_seax = 0, g_redx = 0, g_reax = 0;

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

void EFIAPI thread_wrmsr_with_processorid(void* param)
{
	THRAED_PARAM* tparam = (THRAED_PARAM*)param;
    UINT64 old_value, new_value;
    UINT32 old_edx, old_eax, new_edx, new_eax;
    UINTN core_id;

    old_value =  AsmReadMsr64(tparam->msr);
	old_eax = (UINT32)(old_value & 0xffffffff);
	old_edx = old_value >> 32;

    new_eax = old_eax & g_reax;
    new_eax |= g_seax;

    new_edx = old_edx & g_redx;
    new_edx |= g_sedx;

    new_value = new_eax | (((UINT64)new_edx) << 32);
    
    AsmWriteMsr64(tparam->msr,new_value);

    new_value =  AsmReadMsr64(tparam->msr);
	new_eax = (UINT32)(new_value & 0xffffffff);
	new_edx = new_value >> 32;


	g_pmpp->WhoAmI(g_pmpp, &core_id);
	printf("CORE_%d MSR 0x%04X = 0x%08X 0x%08X  ==> 0x%08X 0x%08X\n", core_id, tparam->msr, old_edx, old_eax, new_edx, new_eax);
}

int main (int argc, char **argv)
{
	UINT32 msr;
	//unsigned long long  value;
    UINT32 hi, low;
	EFI_STATUS status;
	EFI_PROCESSOR_INFORMATION processor_info;
	UINTN processors_num, processors_en_num;
	UINT32 iloop, jloop;
	UINTN processor_id;
    UINT8 index = 0;
    char* argstr = NULL;
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

	if(3 == argc)
	{
		// all cores
		msr = str2llhex(argv[1]);
        argstr = argv[2];

        while((argstr != NULL) && (*argstr != 0))
        {
            index = 0;
            switch(*argstr)
            {

                // set 
                case 's':
                case 'S':
                    if( isdigit(argstr[1]) && isdigit(argstr[2]))
                    {
                        index = (argstr[1] - '0') * 10 + (argstr[2] - '0');
                        argstr += 3;
                        if(index > 31)
                        {
                            g_sedx |= (1 << (index - 32)); 
                        }
                        else
                        {
                            g_seax |= (1 << (index - 0));
                        }                       
                    }
                    else if(isdigit(argstr[1]) && (!isdigit(argstr[2])))
                    {
                        index = argstr[1] - '0';
                        argstr += 2;
                        if(index > 31)
                        {
                            g_sedx |= (1 << (index - 32)); 
                        }
                        else
                        {
                            g_seax |= (1 << (index - 0));
                        }                        
                    }
                    else
                    {
                        argstr = NULL;
                        g_seax = 0;
                        g_sedx = 0;                        
                    }
                    break;

                // clear reset
                case 'c':
                case 'C':
                case 'r':
                case 'R':
                    if( isdigit(argstr[1]) && isdigit(argstr[2]))
                    {
                        index = (argstr[1] - '0') * 10 + (argstr[2] - '0');
                        argstr += 3;
                        if(index > 31)
                        {
                            g_redx |= (1 << (index - 32)); 
                        }
                        else
                        {
                            g_reax |= (1 << (index - 0));
                        }
                    }
                    else if(isdigit(argstr[1]) && (!isdigit(argstr[2])))
                    {
                        index = argstr[1] - '0';
                        argstr += 2;
                        if(index > 31)
                        {
                            g_redx |= (1 << (index - 32)); 
                        }
                        else
                        {
                            g_reax |= (1 << (index - 0));
                        }                        
                    }
                    else
                    {
                        argstr = NULL;
                        g_reax = 0;
                        g_redx = 0;                         
                    } 
                    break;

                default:
                    argstr = NULL;
                    g_reax = 0;
                    g_redx = 0;
                    g_seax = 0;
                    g_sedx = 0;                    
                    break;

            }
        }
        g_reax = ~g_reax;
        g_redx = ~g_redx;

		for(iloop = 0; iloop < processors_num; iloop++)
		{
			g_pmpp->GetProcessorInfo(g_pmpp,iloop,&processor_info);
			if(processor_info.StatusFlag & 0x01)
			{
				//BSP
                THRAED_PARAM thread_param;
				thread_param.msr = msr;
				thread_wrmsr_with_processorid(&thread_param);
				g_pmpp->StartupAllAPs(g_pmpp,thread_wrmsr_with_processorid,TRUE,NULL,0,(void*)&thread_param,NULL);
				break;
			}
		}
	}
	else if (4 == argc)
	{
        processor_id =  str2llhex(&argv[1][1]);
        msr = str2llhex(argv[2]);
        argstr = argv[3];

        while((argstr != NULL) && (*argstr != 0))
        {
            index = 0;
            switch(*argstr)
            {

                // set 
                case 's':
                case 'S':
                    if( isdigit(argstr[1]) && isdigit(argstr[2]))
                    {
                        index = (argstr[1] - '0') * 10 + (argstr[2] - '0');
                        argstr += 3;
                        if(index > 31)
                        {
                            g_sedx |= (1 << (index - 32)); 
                        }
                        else
                        {
                            g_seax |= (1 << (index - 0));
                        }                       
                    }
                    else if(isdigit(argstr[1]) && (!isdigit(argstr[2])))
                    {
                        index = argstr[1] - '0';
                        argstr += 2;
                        if(index > 31)
                        {
                            g_sedx |= (1 << (index - 32)); 
                        }
                        else
                        {
                            g_seax |= (1 << (index - 0));
                        }                        
                    }
                    else
                    {
                        argstr = NULL;
                        g_seax = 0;
                        g_sedx = 0;
                    }
                    break;

                // clear reset
                case 'c':
                case 'C':
                case 'r':
                case 'R':
                    if( isdigit(argstr[1]) && isdigit(argstr[2]))
                    {
                        index = (argstr[1] - '0') * 10 + (argstr[2] - '0');
                        argstr += 3;
                        if(index > 31)
                        {
                            g_redx |= (1 << (index - 32)); 
                        }
                        else
                        {
                            g_reax |= (1 << (index - 0));
                        }
                    }
                    else if(isdigit(argstr[1]) && (!isdigit(argstr[2])))
                    {
                        index = argstr[1] - '0';
                        argstr += 2;
                        if(index > 31)
                        {
                            g_redx |= (1 << (index - 32)); 
                        }
                        else
                        {
                            g_reax |= (1 << (index - 0));
                        }                        
                    }
                    else
                    {
                         argstr = NULL;
                         g_reax = 0;
                         g_redx = 0;
                    } 
                    
                    break;

                default:
                    argstr = NULL;
                    g_reax = 0;
                    g_redx = 0;
                    g_seax = 0;
                    g_sedx = 0;
                    break;
            }
        }

        g_reax = ~g_reax;
        g_redx = ~g_redx;


		for(iloop = 0; iloop < processors_num; iloop++)
		{
			g_pmpp->GetProcessorInfo(g_pmpp,iloop,&processor_info);
			if(processor_info.StatusFlag & 0x01)
			{
				//BSP
                THRAED_PARAM thread_param;
                thread_param.msr = msr;
				if(processor_id == iloop)
				{
					thread_wrmsr_with_processorid(&thread_param);
				}
				else
				{
					g_pmpp->StartupThisAP(g_pmpp,thread_wrmsr_with_processorid,processor_id,0,0,(void*)&thread_param,NULL);
				}
				break;
			}
		}	
	}
	else
	{
		printf("useage: wrmsr.efi  msr  s<0..63>r<0..63>\n");
		printf("useage: wrmsr.efi  =id msr s<0..63>r<0..63>\n");
	}
	
	return 0;
}
