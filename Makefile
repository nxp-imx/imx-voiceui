# Copyright 2021 NXP

CFLAGS += -DNDEBUG -DRDSP_DISABLE_FILEIO -Wno-format-security

CPLUS_FLAGS =

INSTALLDIR := ./release

BUILD_ARCH = CortexA53

all: VOICESEEKER VOICESPOT

VOICESEEKER: | $(INSTALLDIR)
	echo "--- Build voiceseeker library ---"
	make -C ./VoiceSeeker_wrapper
	cp ./VoiceSeeker_wrapper/libvoiceseekerlight.so $(INSTALLDIR)/
	cp ./VoiceSeeker_wrapper/Config.ini  $(INSTALLDIR)/
	cp ./voicespot_release/models/NXP/HeyNXP_en-US_1.bin $(INSTALLDIR)/
	cp ./voicespot_release/models/NXP/HeyNXP_1_params.bin $(INSTALLDIR)/

VOICESPOT: | $(INSTALLDIR)
	echo "--- Build voicespot app ---"
	make -C ./Voice_UI_Test_app
	cp ./Voice_UI_Test_app/voice_ui_app $(INSTALLDIR)/

$(INSTALLDIR) :
	mkdir $@

clean:
	rm -f ./release/libvoiceseekerlight.so
	rm -f ./release/voice_ui_app
	rm -f ./release/Config.ini
	rm -f ./release/HeyNXP_en-US_1.bin
	rm -f ./release/HeyNXP_1_params.bin
	make -C ./VoiceSeeker_wrapper clean
	make -C ./Voice_UI_Test_app clean
