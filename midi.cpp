/**************************************************************************
* This program is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation, version 2 or 3.                           *
* Authors:                                                                *
* - Szymon Kałasz                                                         *
**************************************************************************/

#include "midi.hpp"
#include "tracktempo.hpp"

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdint.h>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <tr1/memory>

using namespace std;
using namespace tr1;

#define trs(x) ((vector<track> *) (x)->b)
#define tempo(x) ((tracktempo *) (x)->a)
#define evs(x) ((vector<timed_event> *) (x)->b)

#define DEFAULT_TICKS_PER_QUARTER 480

#define FOREACH(i, v) for(__typeof__((v).begin()) i = (v).begin(); \
                          (i) != (v).end(); (i)++)

struct timed_event {
	shared_ptr<event> e;
	tick_t start, len;
	timed_event(shared_ptr<event> e, tick_t start)
		: e(e), start(start), len(0) {}
	timed_event(shared_ptr<event> e, tick_t start, tick_t len)
		: e(e), start(start), len(len) {}
};

// =========== track ===========

track::track() {
	this->a = new tracktempo(1.0 / (2 * DEFAULT_TICKS_PER_QUARTER));
	this->b = new vector<timed_event>;
}

track::track(const track & rhs) {
	this->a = new tracktempo(*tempo(&rhs));
	this->b = new vector<timed_event>(*evs(&rhs));
}

track::~track() {
	delete tempo(this);
	delete evs(this);
}

track & track::operator=(const track & rhs) {
	if (&rhs == this) return *this;
	delete tempo(this);
	delete evs(this);
	this->a = new tracktempo(*tempo(&rhs));
	this->b = new vector<timed_event>(*evs(&rhs));
	return *this;
}

unsigned int track::addEvent(event * e, tick_t start) {
	evs(this)->push_back(timed_event(shared_ptr<event>(e), start));
	return evs(this)->size() - 1;
}

unsigned int track::addNote(note * e, tick_t start, tick_t duration) {
	evs(this)->push_back(timed_event(shared_ptr<event>(e), start, duration));
	return evs(this)->size() - 1;
}

void track::delEvent(unsigned int i) {
	evs(this)->erase(evs(this)->begin() + i);
}

void track::replaceEvent(unsigned int i, event * e) {
	(*evs(this))[i].e = shared_ptr<event>(e);
}

unsigned int track::eventCount() const {
	return evs(this)->size();
}

const event & track::events(unsigned int i) const {
	return *((*evs(this))[i].e);
}

tick_t track::getEventTicks(unsigned int i) const {
	return (*evs(this))[i].start;
}

double track::getEventSeconds(unsigned int i) const {
	return tempo(this)->getTickTime(getEventTicks(i));
}

tick_t track::getNoteDurationTicks(unsigned int i) const {
	return (*evs(this))[i].len;
}

double track::getNoteDurationSeconds(unsigned int i) const {
	return tempo(this)->getTickTime
		(((*evs(this))[i].start) + ((*evs(this))[i].len)) -
		tempo(this)->getTickTime(getEventTicks(i));
}

void track::setEventTicks(unsigned int i, tick_t t) {
	(*evs(this))[i].start = t;
}

void track::setNoteDurationTicks(unsigned int i, tick_t t) {
	(*evs(this))[i].len = t;
}

const tracktempo & track::getTrackTempo() const {
	return *tempo(this);
}

void track::setTrackTempo(const tracktempo &t) {
	(*tempo(this)) = t;
}

// =========== midi ===========

midi::midi() : a(DEFAULT_TICKS_PER_QUARTER) {
	b = new vector<track>;
}

midi::midi(const midi & rhs) {
	a = rhs.a;
	b = new vector<track>(*trs(&rhs));
}

midi::~midi() {
	delete trs(this);
}

midi & midi::operator=(const midi & rhs) {
	if (&rhs == this) return *this;
	delete trs(this);
	a = rhs.a;
	b = new vector<track>(*trs(&rhs));
	return *this;
}

unsigned midi::addTrack(const track &t) {
	trs(this)->push_back(t);
	return trs(this)->size() - 1;
}

void midi::delTrack(unsigned int i) {
	trs(this)->erase(trs(this)->begin() + i);
}

unsigned int midi::trackCount() const {
	return trs(this)->size();
}

track & midi::tracks(unsigned int i) {
	return (*trs(this))[i];
}

const track & midi::tracks(unsigned int i) const {
	return (*trs(this))[i];
}

void midi::setTicksPerQuaterNote(tick_t t) {
	a = t;
}

tick_t midi::getTicksPerQuaterNote() const {
	return a;
}
