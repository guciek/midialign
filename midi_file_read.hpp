/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Karol Guciek                                                          *
**************************************************************************/

#ifndef MIDI_FILE_READ_H
#define MIDI_FILE_READ_H

#include "midi.hpp"
#include "midi_file_common.hpp"

extern const char * FILE_FORMAT[];
extern const char * cmd2str[];

void midi_file_read(midi &, const char *);

#endif
