SRC_DIR1  = ../voicespot_release/iMX8M_CortexA53
SRC_DIR2  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/source
SRC_DIR3  = ./
SRC_DIR4  = ./AudioStream

INC_DIR1  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/ne10
INC_DIR2  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/include
INC_DIR3  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/include/public
INC_DIR4  = ../voiceseeker_release/iMX8M_CortexA53/ExApp/VisualStudioProjectRPi_ext/RDSP/source
INC_DIR5  = ./AudioStream
INC_DIR6  = ../vit/i.MX8M_A53/Lib/Inc
INC_DIR7  = ../vit/i.MX8M_A53/Lib

INCLUDES = -I$(INC_DIR1) -I$(INC_DIR2) -I$(INC_DIR3) -I$(INC_DIR4) -I$(INC_DIR5) -I$(INC_DIR6) -I$(INC_DIR7)

CPPFLAGS += -O3 -DNDEBUG -DRDSP_DISABLE_FILEIO -Wno-format-security

LIBRARY = $(SRC_DIR1)/Lib/librdsp_libvoicespotRPi.a $(INC_DIR1)/aarch64/libNE10.a $(INC_DIR7)/libVIT_IMX8_A53_4.6.1.a

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
	$(CXX) $(CPPFLAGS) $(INCLUDES) -fPIC -c -o $@ $<

clean:
	rm -f $(SRC_DIR2)/*.o
	rm -f $(SRC_DIR3)/*.o
	rm -f $(SRC_DIR3)/$(PROGRAM)
	rm -f $(SRC_DIR4)/*.o
