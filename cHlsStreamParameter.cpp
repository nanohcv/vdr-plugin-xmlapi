/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cHlsStreamParameter.cpp
 * Author: karl
 * 
 * Created on 24. Juni 2016, 07:04
 */

#include "cHlsStreamParameter.h"

cHlsStreamParameter::cHlsStreamParameter(string ffmpegCmd, string channelId, string profileName, string baseUrl, cHlsPreset preset) 
    : ffmpegCmd(ffmpegCmd), channelId(channelId), profileName(profileName), baseUrl(baseUrl), segmentDuration(preset.SegmentDuration()),
      segmentBufferSize(preset.SegmentBuffer()), numSegments(preset.NumSegments()), m3u8waitTimeout(preset.M3U8WaitTimeout()), streamTimeout(preset.StreamTimeout()) {
}

cHlsStreamParameter::cHlsStreamParameter(string ffmpegCmd, string channelId, 
        string profileName, string baseUrl, int segmentDuration, size_t segmentBufferSize,
        int numSegments, int m3u8waitTimeout, int streamTimeout)
    : ffmpegCmd(ffmpegCmd), channelId(channelId), profileName(profileName),
      baseUrl(baseUrl), segmentDuration(segmentDuration), segmentBufferSize(segmentBufferSize),
      numSegments(numSegments), m3u8waitTimeout(m3u8waitTimeout),
      streamTimeout(streamTimeout) {
}

cHlsStreamParameter::cHlsStreamParameter(const cHlsStreamParameter& src)
    : ffmpegCmd(src.ffmpegCmd), channelId(src.channelId),
      profileName(src.profileName), baseUrl(src.baseUrl),
      segmentDuration(src.segmentDuration), segmentBufferSize(src.segmentBufferSize), numSegments(src.numSegments), 
      m3u8waitTimeout(src.m3u8waitTimeout), streamTimeout(src.streamTimeout) {
}

cHlsStreamParameter::~cHlsStreamParameter() {
}

cHlsStreamParameter& cHlsStreamParameter::operator =(const cHlsStreamParameter& src) {
    if(this != &src) {
        this->ffmpegCmd = src.ffmpegCmd;
        this->channelId = src.channelId;
        this->profileName = src.profileName;
        this->baseUrl = src.baseUrl;
        this->segmentDuration = src.segmentDuration;
        this->segmentBufferSize = src.segmentBufferSize;
        this->numSegments = src.numSegments;
        this->m3u8waitTimeout = src.m3u8waitTimeout;
        this->streamTimeout = src.streamTimeout;
    }
    return *this;
}

string cHlsStreamParameter::FFmpegCmd() {
    return this->ffmpegCmd;
}

string cHlsStreamParameter::ChannelId() {
    return this->channelId;
}

string cHlsStreamParameter::ProfileName() {
    return this->profileName;
}

string cHlsStreamParameter::BaseUrl() {
    return this->baseUrl;
}

int cHlsStreamParameter::SegmentDuration() {
    return this->segmentDuration;
}

size_t cHlsStreamParameter::SegmentBufferSize() {
    return this->segmentBufferSize;
}

int cHlsStreamParameter::NumSegments() {
    return this->numSegments;
}

int cHlsStreamParameter::M3U8WaitTimeout() {
    return this->m3u8waitTimeout;
}

int cHlsStreamParameter::StreamTimeout() {
    return this->streamTimeout;
}