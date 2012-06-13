/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Szymon Kałasz                                                         *
**************************************************************************/

#include "midi_file_common.hpp"
#include "stream_utils.hpp"

#include <sstream>
#include <iostream>

const char * FILE_FORMAT[] = {
	"single-track",
	"multiple tracks, synchronous",
	"multiple tracks, asynchronous"
};

const char * cmd2str[] = {
	"0", "1", "2", "3", "4", "5", "6", "7",
	"CMD_NOTE_OFF",
	"CMD_NOTE_ON",
	"CMD_KEY_AFTER_TOUCH",
	"CMD_CONTROL_CHANGE",
	"CMD_PROGRAM_CHANGE",
	"CMD_CHANNEL_AFTER_TOUCH",
	"CMD_PITCH_WHEEL_CHANGE",
	"CMD_META_EVENT",
};


using namespace std;


// ========= common_pevent =========== //

common_pevent::common_pevent() : raw(NULL) {};
common_pevent::~common_pevent() { if (raw != NULL) delete [] raw; }
void common_pevent::setBytes(uint8_t * data, int datalen) {
	if (raw != NULL) delete [] raw;
	raw = new uint8_t[datalen];
	copy(data, data + datalen, raw);
	rawlen = datalen;
}
/*virtual event& operator=(const event& o) {
	if (this == &o) return *this;
	uint8_t buf[260];
	int buflen = o.getBytes(buf, 260);
	setBytes(buf, buflen);
	puts("common_pevent::operator= called");
	return *this;
}*/
unsigned int common_pevent::getBytes(uint8_t * buffer, unsigned int length) const {
	return copy(raw, raw + min(length, (unsigned) rawlen), buffer) - buffer;
}

uint8_t common_pevent::getCommand() const {
	return (raw[0] >> 4);
}

uint8_t common_pevent::getMetaCommand() const {
	if (getCommand() != CMD_META_EVENT)
		throw "getMetaCommand: Not a meta command!";
	return raw[1];
}

bool common_pevent::isNote() const {
	if (rawlen == 0) return false;
	return getCommand() == CMD_NOTE_ON;
}


// ============ pevent ============= //


pevent::pevent() {}
pevent::pevent(const pevent& o) {
	setBytes(o.raw, o.rawlen);
}
//	virtual unsigned int getBytes(uint8_t * buffer, unsigned int length);
void pevent::getDescription(char * buffer, unsigned int length) const {
	stringstream ss(stringstream::in | stringstream::out);
	ss.setf(ios::showbase);
	ss << cmd2str[getCommand()];
	if (getCommand() == CMD_META_EVENT) {
		ss << ", meta command: " << hex << (int) raw[1];
		if (getMetaCommand() == META_TEMPO_CHANGE) {
			ss << ", META_TEMPO_CHANGE";
		}
	}
	ss.getline(buffer, length);
}


// ============ pnote ============== //


pnote::pnote() : noteoff_raw(NULL) {}
pnote::pnote(const pnote& o) {
	setBytes(o.raw, o.rawlen);
	setNoteOffBytes(o.noteoff_raw, o.noteoff_rawlen);
}
unsigned int pnote::getBytes(uint8_t * buf, unsigned int len) const {
	return common_pevent::getBytes(buf, len);
}
unsigned int pnote::getNoteOffBytes(uint8_t * buffer, unsigned int length) const {
	return copy(noteoff_raw, noteoff_raw +
		min(length, (unsigned) noteoff_rawlen),buffer) - buffer;
}
void pnote::setNoteOffBytes(uint8_t * data, int datalen) {
	if (noteoff_raw != NULL) delete [] noteoff_raw;
	noteoff_raw = new uint8_t[datalen];
	copy(data, data + datalen, noteoff_raw);
	noteoff_rawlen = datalen;
}
void pnote::getDescription(char * buffer, unsigned int length) const {
	stringstream ss(stringstream::in | stringstream::out);
	ss.setf(ios::showbase);
	ss << cmd2str[getCommand()];
	ss << ", pitch: " << hex << (int) raw[1];
	ss.getline(buffer, length);
}
