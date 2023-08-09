# Copyright 2021 NXP

CFLAGS += -DNDEBUG -DRDSP_DISABLE_FILEIO -Wno-format-security

CPLUS_FLAGS =

INSTALLDIR := ./release

BUILD_ARCH ?= CortexA53
export BUILD_ARCH

all: VOICESEEKER VOICESPOT

VOICESEEKER: | $(INSTALLDIR)
	@echo "--- Build voiceseeker library ---"
	make -C ./voiceseeker/
	cp ./voiceseeker/build/$(BUILD_ARCH)/libvoiceseekerlight.so $(INSTALLDIR)/libvoiceseekerlight.so.2.0
	cp ./voiceseeker/src/Config.ini $(INSTALLDIR)/

VOICESPOT: | $(INSTALLDIR)
	@echo "--- Build voicespot app ---"
	make -C ./voicespot
	cp ./voicespot/build/$(BUILD_ARCH)/voice_ui_app $(INSTALLDIR)/
	cp ./voicespot/platforms/models/NXP/HeyNXP_en-US_1.bin $(INSTALLDIR)/
	cp ./voicespot/platforms/models/NXP/HeyNXP_1_params.bin $(INSTALLDIR)/

$(INSTALLDIR) :
	mkdir $@

clean:
	rm -rf ./release
	make -C ./voiceseeker clean
	make -C ./voicespot clean

