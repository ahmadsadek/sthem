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

#ifndef PROFILE_H
#define PROFILE_H

#include <QString>
#include <QFile>
#include <QtSql>

#include <sstream>

#include "profline.h"
#include "cfg/module.h"
#include "cfg/basicblock.h"
#include "measurement.h"

class Profile {

private:
  void clear();
  void addMeasurement(Measurement measurement);

public:
  std::map<BasicBlock*, std::vector<Measurement>*> measurementsPerBb[Pmu::MAX_CORES];
  QVector<Measurement> measurements;

  Profile();
  virtual ~Profile();

  void setMeasurements(QVector<Measurement> *measurements);
  void getProfData(unsigned core, BasicBlock *bb, double *runtime, double *energy);
  void getMeasurements(unsigned core, BasicBlock *bb, QVector<Measurement> *measurements);

  double getRuntime() const {
    return 0; // FIXME
  }
  double getEnergy(unsigned sensor) const {
    return 0; // FIXME
  }

  void setRuntime(double runtime) {
    // FIXME
  }
  void setEnergy(unsigned sensor, double energy) {
    // FIXME
  }

  void clean();

  void addExternalFunctions(Cfg *cfg);

	friend std::ostream& operator<<(std::ostream &os, const Profile &p) {
    // only streams out the necessary parts for DSE
		os << p.getRuntime() << '\n';
    for(unsigned i = 0; i < (Pmu::MAX_SENSORS-1); i++) {
      os << p.getEnergy(i) << '\n';
    }
    os << p.getEnergy(Pmu::MAX_SENSORS-1);
		return os;
	}

	friend std::istream& operator>>(std::istream &is, Profile &p) {
    double runtime;
		is >> runtime;
    p.setRuntime(runtime);

    for(unsigned i = 0; i < Pmu::MAX_SENSORS; i++) {
      double energy;
      is >> energy;
      p.setEnergy(i, energy);
    }
		return is;
	}

};

#endif
