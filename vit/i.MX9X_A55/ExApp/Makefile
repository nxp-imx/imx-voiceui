CC          = aarch64-linux-gnu-gcc
AR          = aarch64-linux-gnu-ar

REQUIRED_GCC_MAJOR_VERSION=10

CURRENT_GCC_VERSION=$(shell $(CC) --version | grep ^$(CC) | sed 's/^.* //g' )
CURRENT_GCC_MAJOR_VERSION=$(shell (echo $(CURRENT_GCC_VERSION) | cut -d. -f1))

eval := $(shell [ $(CURRENT_GCC_MAJOR_VERSION) -ge $(REQUIRED_GCC_MAJOR_VERSION) ] && echo true)

ifneq ($(eval),true)
$(error Please upgrade gcc to v$(REQUIRED_GCC_MAJOR_VERSION) minimum (current = $(CURRENT_GCC_VERSION)))
endif

VERSION?=
BLDDIR=build

ifdef VERSION
LIB_VERSION=$(VERSION)
else
$(error VERSION is undefined)
endif

TEST_FILES= \
	VIT_alsa_test_app.c \
	VIT_action_executor.c \
	VIT_helper.c

IFLAGS=-IExApp \
			 -I../Lib/Inc \
			 -I../Lib

CFLAGS=-std=gnu99 \
	 -Wno-unused-variable \
	 -Wsign-compare \
	 -Wno-unused-result \
	 -Wno-unused-function \
	 -Wno-unused-but-set-variable \
	 -no-pie \
	 -DPLATFORM_AARCH64 \
	 -Werror \
	 -DTOOLCHAIN_GCC \
	 -DARM_MATH_NEON \
	 -g3 \
	 -Wall \
	 -D_FILE_OFFSET_BITS=64 \
	 -DARCH_AARCH64 \
	 -march=armv8.2-a \
	 -mcpu=cortex-a55+i8mm+f32mm \
	 $(IFLAGS)

EXE_OBJ_FILES   = $(addprefix $(BLDDIR)/,$(patsubst %.c,%.o,$(notdir $(TEST_FILES))))

vpath %.c \
	$(BLDDIR) \
	ExApp

EXE       = $(BLDDIR)/test_vit

all : mkdir $(EXE)

mkdir : $(BLDDIR)/.mkdir

$(BLDDIR)/.mkdir :
	@mkdir -p $(BLDDIR)


-include $(wildcard $(BLDDIR)/*.d)


$(EXE) : $(EXE_OBJ_FILES)
	$(CC) $(CFLAGS)  $^  \
	 -L../Lib -lVIT_iMX9X_A55_v$(LIB_VERSION) \
	 -L. -lasound \
	 -lm  \
	 -o $@

$(BLDDIR)/%.o : %.c $(BLDDIR)/compiler_flags
	$(CC) $(CFLAGS) -MD -MF $(subst .o,.d,$@) -c -o $@ $<

clean :
	rm -f $(BLDDIR)/*

.PHONY : all mkdir clean

.PHONY: force
$(BLDDIR)/compiler_flags: force
	@echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@
