/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Szymon Kałasz                                                         *
**************************************************************************/

#ifndef MIDI_FILE_COMMON_H
#define MIDI_FILE_COMMON_H

#include <inttypes.h>

#include "midi.hpp"

using namespace std;

#define CMD_NOTE_OFF               0x8
#define CMD_NOTE_ON                0x9
#define CMD_KEY_AFTER_TOUCH        0xA
#define CMD_CONTROL_CHANGE         0xB
#define CMD_PROGRAM_CHANGE         0xC
#define CMD_CHANNEL_AFTER_TOUCH    0xD
#define CMD_PITCH_WHEEL_CHANGE     0xE
#define CMD_META_EVENT             0xF

#define META_TEMPO_CHANGE          0x51
#define META_TRACK_END             0x2F

const char * getCommandStr(uint8_t cmd);

class common_pevent : public event {
	public:
	common_pevent();
	~common_pevent();
	void setBytes(uint8_t * data, int datalen);
	virtual unsigned int getBytes(uint8_t * buffer, unsigned int length) const;
	virtual bool isNote() const;
	uint8_t getCommand() const;
	uint8_t getMetaCommand() const;
	protected:
	uint8_t * raw;
	int rawlen;
};

class pevent : public common_pevent {
	public:
	pevent();
	pevent(const pevent& o);
	virtual void getDescription(char * buffer, unsigned int length) const;
};

class pnote : public pevent, public note {
	public:
	pnote();
	pnote(const pnote& o);
	virtual unsigned int getBytes(uint8_t * buf, unsigned int len) const;
	virtual unsigned int getNoteOffBytes(uint8_t * buffer, unsigned int length) const;
	void setNoteOffBytes(uint8_t * data, int datalen);
	virtual void getDescription(char * buffer, unsigned int length) const;
	protected:
	uint8_t * noteoff_raw;
	int noteoff_rawlen;
};

#endif /* MIDI_FILE_COMMON_H */
