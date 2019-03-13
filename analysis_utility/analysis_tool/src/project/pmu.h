/******************************************************************************
 *
 *  This file is part of the TULIPP Analysis Utility
 *
 *  Copyright 2018 Asbjørn Djupdal, NTNU, TULIPP EU Project
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#ifndef PMU_H
#define PMU_H

#include <libusb.h>

#include <QDataStream>
#include <QtSql>
#include <QQueue>

#include <usbprotocol.h>

#include "analysis_tool.h"

#define STOP_AT_BREAKPOINT 0
#define STOP_AT_TIME       1

#define LYNSYN_FREQ 48000000

class Measurement;

///////////////////////////////////////////////////////////////////////////////

class Sample;

class Pmu : public QObject {
  Q_OBJECT

public:
  static const unsigned sensors = SENSORS;
  static const unsigned maxCores = MAX_CORES;

private:
  QThread dbThread;

	struct libusb_device_handle *lynsynHandle;
  uint8_t outEndpoint;
  uint8_t inEndpoint;
	struct libusb_context *usbContext;
	libusb_device **devs;
  uint8_t swVersion;
  uint8_t hwVersion;
  double sensorCalibration[sensors]; // V1.0 - V1.3
  double sensorOffset[sensors];      // V1.4
  double sensorGain[sensors];        // V1.4

  void sendBytes(uint8_t *bytes, int numBytes);
  bool getBytes(uint8_t *bytes, int numBytes, uint32_t timeout = 0);
  bool getArray(uint8_t *bytes, int maxNum, int numBytes, unsigned *elementsReceived, uint32_t timeout = 0);

  static uint32_t crc32(uint32_t crc, uint32_t *data, int length);

public:

  double rl[sensors];
  double supplyVoltage[sensors];

  Pmu() {}
  ~Pmu() {
    dbThread.quit();
    dbThread.wait();
  }

  Pmu(double rl[sensors], double supplyVoltage[sensors]) {
    for(unsigned i = 0; i < sensors; i++) {
      this->rl[i] = rl[i];
      this->supplyVoltage[i] = supplyVoltage[i];
    }
  }

  bool init();
  void release();

  double currentToPower(unsigned sensor, double current);
  static double currentToPower(unsigned sensor, double current, double *rl, double *supplyVoltage, double *sensorOffset, double *sensorGain);
  bool checkForUpgrade(QString filename);

  bool collectSamples(bool useFrame, 
                      uint64_t frameAddr, bool startAtBp, unsigned stopAt, bool samplePc, bool samplingModeGpio,
                      int64_t samplePeriod, uint64_t startAddr, uint64_t stopAddr, 
                      uint64_t *samples, int64_t *minTime, int64_t *maxTime, double *minPower, double *maxPower,
                      double *runtime, double *energy);

  unsigned numSensors() { return sensors; }
  unsigned numCores() { return maxCores; }

  static double cyclesToSeconds(uint64_t cycles) { return cycles / (double)LYNSYN_FREQ; }
  static uint64_t secondsToCycles(double seconds) { return (uint64_t)(seconds * LYNSYN_FREQ); }

signals:
  void initTransaction();
  void commitTransaction();
  void storeRawSample(Sample *sample);

};

///////////////////////////////////////////////////////////////////////////////

class Sample {
public:
  int64_t timeSinceLast;
  SampleReplyPacket sample;
  double power[Pmu::sensors];

  Sample(int64_t t, SampleReplyPacket s, double *p) {
    timeSinceLast = t;
    sample = s;
    for(unsigned i = 0; i < Pmu::sensors; i++) {
      power[i] = p[i];
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

class DBStorer : public QObject {
  Q_OBJECT

private:
  QSqlQuery *query;
  uint8_t swVersion;

public:
  DBStorer(uint8_t swVersion);
  ~DBStorer();

public slots:
  void initTransaction();
  void commitTransaction();
  void storeRawSample(Sample *sample);

};

///////////////////////////////////////////////////////////////////////////////

#endif

