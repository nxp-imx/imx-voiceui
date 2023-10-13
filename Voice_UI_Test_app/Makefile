ifeq ($(BUILD_ARCH),CortexA55)
	BOARD = 9_CortexA55
else
	BOARD = 8M_CortexA53
endif

SRC_DIR1  = ../voicespot_release/iMX8M_CortexA53
SRC_DIR2  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/source
SRC_DIR3  = .
SRC_DIR4  = ./AudioStream
SRC_DIR5  = ../voicespot_release/iMX93_CortexA55
SRC_DIR6  = ../VoiceSeeker_wrapper

INC_DIR1  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/ne10
INC_DIR2  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/include
INC_DIR3  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/include/public
INC_DIR4  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/source
INC_DIR5  = ./AudioStream
INC_DIR6  = ../vit/iMX$(BOARD)/lib/inc
INC_DIR7  = ../vit/iMX$(BOARD)/lib
INC_DIR8  = ../voiceseeker_release/rdsp_utilities_public/rdsp_memory_utils_public
INC_DIR10  = ../voiceseeker_release/rdsp_utilities_public/include
INC_DIR11  = ../VoiceSeeker_wrapper

CPPFLAGS += -O3 -DNDEBUG -DRDSP_DISABLE_FILEIO -Wno-format-security

LIBRARY = $(SRC_DIR5)/Lib/librdsp_libvoicespotRPi.a $(INC_DIR1)/aarch64/libNE10.a $(INC_DIR7)/libVIT_iMX$(BOARD)_v04_09_00_EngineeringRelease.a

INCLUDES = -I$(INC_DIR1) -I$(INC_DIR2) -I$(INC_DIR3) -I$(INC_DIR4) -I$(INC_DIR5) -I$(INC_DIR6) -I$(INC_DIR7) -I$(INC_DIR8) -I$(INC_DIR10) \
           -I$(INC_DIR11)

CPP_OBJS  = $(SRC_DIR3)/VoiceSpot.o \
            $(SRC_DIR3)/SignalProcessor_VoiceSpot.o \
            $(SRC_DIR3)/SignalProcessor_NotifyTrigger.o \
            $(SRC_DIR3)/SignalProcessor_VIT.o \
            $(SRC_DIR3)/rdsp_buffer.o \
            $(SRC_DIR3)/RdspAppUtilities.o \
            $(SRC_DIR2)/RdspVslAppUtilities.o \
            $(SRC_DIR4)/AudioStreamBase.o \
            $(SRC_DIR4)/AudioStream.o \
            $(SRC_DIR4)/AudioStreamException.o \
            $(SRC_DIR6)/AFEConfigState.o


PROGRAM  := voice_ui_app

all: $(PROGRAM)

$(PROGRAM) : $(CPP_OBJS)
	$(CXX) $(CPP_OBJS) $(LIBRARY) $(LDFLAGS) -o $(PROGRAM) -lrt -lasound

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -D ${BUILD_ARCH} -fPIC -c -o $@ $<

clean:
	rm -f $(SRC_DIR2)/*.o
	rm -f $(SRC_DIR3)/*.o
	rm -f $(SRC_DIR3)/$(PROGRAM)
	rm -f $(SRC_DIR4)/*.o
