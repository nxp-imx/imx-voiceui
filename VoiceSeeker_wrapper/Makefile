VERSION = 1.0

SRC_DIR1  = ../voiceseeker_release/iMX8M_CortexA53
SRC_DIR2  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/source
SRC_DIR3  = ./

INC_DIR1  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/ne10
INC_DIR2  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/include
INC_DIR3  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/include/public
INC_DIR4  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/source
INC_DIR5  = ./

INCLUDES = -I$(INC_DIR1) -I$(INC_DIR2) -I$(INC_DIR3) -I$(INC_DIR4) -I$(INC_DIR5)

CPPFLAGS += -O3 -DNDEBUG -DRDSP_DISABLE_FILEIO -Wno-format-security

LIBRARY = $(SRC_DIR1)/Lib/libVoiceSeekerLightRPi_noAEC.a $(INC_DIR1)/aarch64/libNE10.a

CPP_OBJS  = $(SRC_DIR3)/SignalProcessor_VoiceSeekerLight.o \
            $(SRC_DIR3)/RdspAppUtilities.o \
            $(SRC_DIR3)/AFEConfigState.o \
            $(SRC_DIR2)/RdspWavfile.o

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
