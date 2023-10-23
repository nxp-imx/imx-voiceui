ifeq ($(BUILD_ARCH),CortexA55)
	VSPOT = ./platforms/iMX9_CortexA55
	VIT_PATH = ../vit/platforms/iMX9_CortexA55
	VIT_LIB = ../vit/platforms/iMX9_CortexA55/lib/libVIT_iMX9_CortexA55_v04_09_00.a
	VSEEKER = ../voiceseeker/platforms/iMX9_CortexA55
else
	BUILD_ARCH = CortexA53
	VSPOT = ./platforms/iMX8M_CortexA53
	VIT_PATH = ../vit/platforms/iMX8M_CortexA53
	VIT_LIB = ../vit/platforms/iMX8M_CortexA53/lib/libVIT_iMX8M_CortexA53_v04_09_00.a
	VSEEKER = ../voiceseeker/platforms/iMX8M_CortexA53
endif

BUILD_DIR = ./build/$(BUILD_ARCH)

NE10_DIR = ../utils/ne10
RDSP_DIR = ../utils/rdsp_common_utils
AFE_DIR = ../utils/afe_config
AST_DIR = ../utils/audiostream

VS_DIR1 = $(VSPOT)/lib/include
VIT_DIR1 = ../vit/src

INC_DIR1 = $(VSEEKER)/include
INC_DIR2 = $(VSEEKER)/rdsp_utilities_public/include
INC_DIR3 = $(VSEEKER)/rdsp_utilities_public/rdsp_memory_utils_public

CPPFLAGS += -O3 -DNDEBUG -DRDSP_DISABLE_FILEIO -Wno-format-security
LIBRARY = 	$(VSPOT)/lib/libvoicespot.a  \
			$(VIT_LIB) $(NE10_DIR)/lib/libNE10.a

INCLUDES = $(addprefix -I, ./src $(VS_DIR1) 		\
		$(VIT_PATH)/lib $(VIT_PATH)/lib/inc 		\
		$(AFE_DIR) $(NE10_DIR)/include				\
		$(RDSP_DIR)/src $(AST_DIR) $(VIT_DIR1)		\
		$(INC_DIR1) $(INC_DIR2) $(INC_DIR3))

SRCS = 	./voice_ui_app.cpp							\
	   	./src/SignalProcessor_VoiceSpot.cpp 		\
	   	./src/SignalProcessor_NotifyTrigger.cpp		\
	   	$(VIT_DIR1)/SignalProcessor_VIT.cpp			\
		$(AFE_DIR)/AFEConfigState.cpp 				\
		$(RDSP_DIR)/src/RdspAppUtilities.cpp 		\
		$(RDSP_DIR)/src/RdspVslAppUtilities.cpp 	\
		$(RDSP_DIR)/src/RdspBuffer.c				\
		$(AST_DIR)/AudioStream.cpp					\
		$(AST_DIR)/AudioStreamBase.cpp				\
		$(AST_DIR)/AudioStreamException.cpp			\

vpath %.cpp $(dir $(SRCS))
vpath %.c $(dir $(SRCS))

OBJ =	$(addsuffix .o, $(notdir  $(basename $(SRCS))))
LIST = $(addprefix $(BUILD_DIR)/, $(OBJ))

PROGRAM  := voice_ui_app

all: $(PROGRAM)

$(PROGRAM): $(BUILD_DIR) $(OBJ)
	$(CXX) $(LIST) $(LIBRARY) $(LDFLAGS) -o $(BUILD_DIR)/$(PROGRAM) -lrt -lasound

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -D ${BUILD_ARCH} -fPIC -c -o $(BUILD_DIR)/$@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -fPIC -c -o $(BUILD_DIR)/$@ $<

$(BUILD_DIR):
	@mkdir -p $@/

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Removed '$(BUILD_DIR)' directory"

distclean:
	@rm -rf build
	@echo "Removed 'build' directory"
