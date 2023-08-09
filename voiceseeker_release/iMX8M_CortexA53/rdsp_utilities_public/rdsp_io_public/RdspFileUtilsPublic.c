/*Copyright 2021 Retune DSP 
* Copyright 2022 NXP 
*
* NXP Confidential. This software is owned or controlled by NXP 
* and may only be used strictly in accordance with the applicable license terms.  
* By expressly accepting such terms or by downloading, installing, 
* activating and/or otherwise using the software, you are agreeing that you have read, 
* and that you agree to comply with and are bound by, such license terms.  
* If you do not agree to be bound by the applicable license terms, 
* then you may not retain, install, activate or otherwise use the software.
*
*/
#include "RdspFileUtilsPublic.h"
#include "RdspStringUtilsPublic.h"

//#if RDSP_CONVERSA_LIB_ENABLE_FILEIO


RdspFileStatus rdsp__fopen(void* Afile, char* Afilename, const char* Arw) {
#ifdef _WIN32
    FILE** file = (FILE**) Afile;
	errno_t err = fopen_s(file, Afilename, Arw);
	if (err != 0) {
		printf("Error opening %s\n", Afilename);
		return FILE_GENERAL_ERROR;
	}
#elif defined(USE_FATFS)
	FIL* file = (FIL*) Afile;
	if (strcmp(Arw, "wb") == 0) {
	    FRESULT res = f_open(file, Afilename, FA_CREATE_ALWAYS | FA_WRITE);
	    if(res != FR_OK) {
	        rdsp_printf("Error opening %s\r\n", Afilename);
	        f_close(file);
	        return FILE_GENERAL_ERROR;
	    }
	}
	else {
	    rdsp_printf("Error: file mode %s not supported when opening %s.", Arw, Afilename);
	    return FILE_GENERAL_ERROR;
	}
#else
	FILE** file = (FILE**) Afile;
	*file = fopen(Afilename, Arw);
	if (file == NULL) {
	    rdsp_printf("Error opening %s\r\n", Afilename);
        return FILE_GENERAL_ERROR;
	}
#endif

    return FILE_OK;
}

RdspFileStatus rdsp__fwrite(void* Ain, int32_t Asize, int32_t Anum, void* Afile) {
#ifdef USE_FATFS
    int32_t num_bytes = Asize * Anum;
    FIL* file = (FIL*) Afile;
    uint32_t num_written;

    FRESULT res = f_write(file, Ain, num_bytes, &num_written);

    if (res != FR_OK)
        return FILE_GENERAL_ERROR;

    // Sync file to clear buffers in case we can't close
    res = f_sync(file);
    if (res != FR_OK)
        return FILE_GENERAL_ERROR;

#else
	FILE** file = (FILE**)Afile;
    if (fwrite(Ain, Asize, Anum, *file) != Anum)
        return FILE_GENERAL_ERROR;
#endif
    return FILE_OK;
}

//#endif // RDSP_CONVERSA_LIB_ENABLE_FILEIO
