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

#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QVector>
#include <QDir>
#include <QDomDocument>

#include "elfsupport.h"
#include "projectacc.h"
#include "config/config.h"
#include "profile/measurement.h"
#include "cfg/cfg.h"
#include "pmu.h"
#include "location.h"

#define SAMPLEBUF_SIZE (128*1024*1024)

//////////////////////////////////////////////////////////////////////////////

class Project : public QObject {
  Q_OBJECT

protected:
  void writeTulippCompileRule(QString compiler, QFile &makefile, QString path, QString opt);
  void writeLinkRule(QString linker, QFile &makefile, QStringList objects);
  void writeCleanRule(QFile &makefile);

  bool createXmlMakefile();
  virtual bool createMakefile(QFile &makefile);
  virtual bool createMakefile() = 0;
  void copy(Project *p);
  Location *getLocation(unsigned core, uint64_t pc, ElfSupport *elfSupport, std::map<BasicBlock*,Location*> *locations);

public:
  bool opened;
  bool isCpp;

  QString path;

  // user specified settings
  QStringList systemXmls;
  int cfgOptLevel;
  QString tcfUploadScript;
  Pmu pmu;
  QString startFunc;
  unsigned startCore;
  QString stopFunc;
  unsigned stopCore;
  bool createBbInfo;
  bool useCustomElf;
  bool samplePc;
  bool samplingModeGpio;
  int64_t samplePeriod;

  // settings from either sdsoc project or user
  QStringList sources;
  QString name;
  QString cOptions;
  int cOptLevel;
  QString cppOptions;
  int cppOptLevel;
  QString linkerOptions;
  bool ultrascale;

  // settings from sdsoc project, unused otherwise
  QString configType;
  QVector<ProjectAcc> accelerators;
  QString cSysInc;
  QString cppSysInc;

  QString customElfFile;
  QString elfFile;

  Cfg *cfg;

  int errorCode;

  Project();
  Project(Project *p);
  virtual ~Project();

  void close();
  void clear();

  void clean();

  virtual void print();
  int runSourceTool(QString inputFilename, QString outputFilename, QStringList loopsToPipeline, QString opt);

  QString elfFilename() {
    return name + ".elf";
  }

  void parseProfFile(QString fileName, Profile *profile);

  void loadFiles();
  void loadXmlFile(const QString &fileName);
  void loadProjectFile();
  void saveProjectFile();

  int xmlBuildSteps() { return 1; }
  int binBuildSteps() { return 2; }
  int profileSteps() { return 3; }
  int runSteps() { return 1; }

  virtual bool isSdSocProject() { return false; }

public slots:
  void makeXml();
  virtual void makeBin();
  void runProfiler();
  void runApp();

signals:
  void advance(int step, QString msg);
  void finished(int ret, QString msg);
};

#endif
