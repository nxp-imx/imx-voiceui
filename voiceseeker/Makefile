VERSION = 2.0

ifeq ($(BUILD_ARCH),CortexA55)
	VS_PATH = ./platforms/iMX9_CortexA55
else
	BUILD_ARCH = CortexA53
	VS_PATH = ./platforms/iMX8M_CortexA53
endif

MACROS = -D${BUILD_ARCH}

ifdef AEC
$(info Building with AEC)
VS_LIB = libvoiceseeker.a
MACROS += -DAEC
else
$(info Building without AEC. Please contact NXP for AEC enablement.)
VS_LIB = libvoiceseeker_no_aec.a
endif

BUILD_DIR = ./build/$(BUILD_ARCH)

NE10_DIR = ../utils/ne10
RDSP_DIR = ../utils/rdsp_common_utils
AFE_DIR = ../utils/afe_config

VS_DIR1 = $(VS_PATH)/include
VS_DIR2 = $(VS_PATH)/rdsp_utilities_public/include
VS_DIR3 = $(VS_PATH)/rdsp_utilities_public/rdsp_memory_utils_public

CPPFLAGS += -O3 -DNDEBUG -DRDSP_DISABLE_FILEIO -Wno-format-security
LIBRARY = $(VS_PATH)/lib/$(VS_LIB) $(NE10_DIR)/lib/libNE10.a

INCLUDES = $(addprefix -I, ./include $(VS_DIR1)		\
		$(VS_DIR2) $(VS_DIR3) $(AFE_DIR)			\
		$(NE10_DIR)/include $(RDSP_DIR)/src)

SRCS =	./src/SignalProcessor_VoiceSeekerLight.cpp 	\
		$(RDSP_DIR)/src/RdspWavfile.cpp 			\
		$(RDSP_DIR)/src/RdspAppUtilities.cpp 		\
		$(AFE_DIR)/AFEConfigState.cpp 				\
		$(VS_DIR3)/RdspMemoryUtilsPublic.c 			\
		$(VS_DIR3)/memcheck.c

vpath %.cpp $(dir $(SRCS))
vpath %.c $(dir $(SRCS))

OBJ =	$(addsuffix .o, $(notdir  $(basename $(SRCS))))
LIST = $(addprefix $(BUILD_DIR)/, $(OBJ))

PROGRAM  := libvoiceseekerlight

all: VOICESEEKER

VOICESEEKER : $(BUILD_DIR) $(OBJ)
	$(CXX) -o $(BUILD_DIR)/$(PROGRAM).so -shared $(LIST) $(LIBRARY) $(LDFLAGS) -Wl,-soname,$(PROGRAM).so.$(VERSION)

$(BUILD_DIR):
	@mkdir -p $@

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) ${MACROS} -fPIC -c -o $(BUILD_DIR)/$@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -fPIC -c -o $(BUILD_DIR)/$@ $<

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Removed '$(BUILD_DIR)' directory"

distclean:
	@rm -rf build
	@echo "Removed 'build' directory"
