CC          = aarch64-linux-gnu-gcc
AR          = aarch64-linux-gnu-ar

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
	-I../lib/inc \
	-I../lib

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
	 -march=armv8-a \
	 -mcpu=cortex-a35 \
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
	 -L../lib -lVIT_iMX8_CortexA35_v$(LIB_VERSION) \
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
