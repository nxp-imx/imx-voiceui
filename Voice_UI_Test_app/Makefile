SRC_DIR1  = ../voicespot_release/iMX8M_CortexA53
SRC_DIR2  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/source
SRC_DIR3  = ./
SRC_DIR4  = ./AudioStream
SRC_DIR5  = ../voicespot_release/iMX93_CortexA55

INC_DIR1  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/ne10
INC_DIR2  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/include
INC_DIR3  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/include/public
INC_DIR4  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/source
INC_DIR5  = ./AudioStream
INC_DIR6  = ../vit/i.MX8M_A53/Lib/Inc
INC_DIR7  = ../vit/i.MX8M_A53/Lib
INC_DIR8  = ../voiceseeker_release/rdsp_memory_utils_public
INC_DIR9  = ../vit/i.MX9X_A55/Lib

INCLUDES = -I$(INC_DIR1) -I$(INC_DIR2) -I$(INC_DIR3) -I$(INC_DIR4) -I$(INC_DIR5) -I$(INC_DIR6) -I$(INC_DIR7) -I$(INC_DIR8)

CPPFLAGS += -O3 -DNDEBUG -DRDSP_DISABLE_FILEIO -Wno-format-security

ifeq ($(BUILD_ARCH),CortexA55)
	LIBRARY = $(SRC_DIR5)/Lib/librdsp_libvoicespotRPi.a $(INC_DIR1)/aarch64/libNE10.a $(INC_DIR9)/libVIT_iMX9X_A55_v4.7.3_EAR.a
else
	LIBRARY = $(SRC_DIR1)/Lib/librdsp_libvoicespotRPi.a $(INC_DIR1)/aarch64/libNE10.a $(INC_DIR7)/libVIT_iMX8M_A53_v4.7.3_EAR.a
endif

CPP_OBJS  = $(SRC_DIR3)/VoiceSpot.o \
            $(SRC_DIR3)/SignalProcessor_VoiceSpot.o \
            $(SRC_DIR3)/SignalProcessor_NotifyTrigger.o \
            $(SRC_DIR3)/SignalProcessor_VIT.o \
            $(SRC_DIR3)/rdsp_buffer.o \
            $(SRC_DIR3)/RdspAppUtilities.o \
            $(SRC_DIR2)/RdspVslAppUtilities.o \
            $(SRC_DIR4)/AudioStreamBase.o \
            $(SRC_DIR4)/AudioStream.o \
            $(SRC_DIR4)/AudioStreamException.o


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
