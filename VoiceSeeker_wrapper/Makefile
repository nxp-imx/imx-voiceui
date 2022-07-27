VERSION = 2.0

SRC_DIR1  = ../voiceseeker_release/iMX8M_CortexA53
SRC_DIR2  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/source
SRC_DIR3  = ./
SRC_DIR4  = ../voiceseeker_release/iMX93_CortexA55
SRC_DIR5  = ../voiceseeker_release/rdsp_memory_utils_public

INC_DIR1  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/ne10
INC_DIR2  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/include
INC_DIR3  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/include/public
INC_DIR4  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/source
INC_DIR5  = ./
INC_DIR6  = ../voiceseeker_release/rdsp_memory_utils_public

INCLUDES = -I$(INC_DIR1) -I$(INC_DIR2) -I$(INC_DIR3) -I$(INC_DIR4) -I$(INC_DIR5) -I$(INC_DIR6)

CPPFLAGS += -O3 -DNDEBUG -DRDSP_DISABLE_FILEIO -Wno-format-security

ifeq ($(BUILD_ARCH),CortexA55)
	LIBRARY = $(SRC_DIR4)/lib/libVoiceSeekerLightRPi_noAEC.a $(INC_DIR1)/aarch64/libNE10.a
else
	LIBRARY = $(SRC_DIR1)/Lib/libVoiceSeekerLightRPi_noAEC.a $(INC_DIR1)/aarch64/libNE10.a
endif

CPP_OBJS  = $(SRC_DIR3)/SignalProcessor_VoiceSeekerLight.o \
            $(SRC_DIR3)/RdspAppUtilities.o \
            $(SRC_DIR3)/AFEConfigState.o \
            $(SRC_DIR2)/RdspWavfile.o \
            $(SRC_DIR5)/RdspMemoryUtilsPublic.o

PROGRAM  := libvoiceseekerlight

all: VOICESEEKER

VOICESEEKER : $(CPP_OBJS)
	$(CXX) -o $(PROGRAM).so -shared $^ $(LIBRARY) $(LDFLAGS) -Wl,-soname,$(PROGRAM).so.$(VERSION)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -fPIC -c -o $@ $<

clean:
	rm -f $(SRC_DIR3)/*.o
	rm -f $(SRC_DIR3)/$(PROGRAM)
	rm -f $(SRC_DIR3)/libvoiceseekerlight.so
