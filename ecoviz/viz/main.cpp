/*******************************************************************************
 *
 * EcoSynth - Data-driven Authoring of Large-Scale Ecosystems (Undergrowth simulator)
 * Copyright (C) 2020  J.E. Gain  (jgain@cs.uct.ac.za)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ********************************************************************************/



/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "glheaders.h"
#include <QApplication>
#include <QGLFormat>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <string>
#include <stdexcept>
#include <utility>
#include <memory>
#include <QTimer>

#include "window.h"

void parseCommandLine(int agc, char *argv[]);
void printUsage(void);
void printError(const std::string & s);

std::string leftprefix="", rightprefix="";
std::string dir = "";

int main(int argc, char *argv[])
{
    //int run_id, nyears;

    parseCommandLine(argc, argv);

    std::string datadir = dir;
    while (datadir.back() == '/')
        datadir.pop_back();

    try
    {
        QApplication app(argc, argv);    

        Window * window = new Window(datadir, leftprefix, rightprefix);

        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedKingdom));

        window->resize(window->sizeHint());
        window->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
        // window->getView().setForcedFocus(window->getTerrain().getFocus());

        int desktopArea = QApplication::desktop()->width() *
            QApplication::desktop()->height();

        //window->loadSceneData(); // PCM - ensure all data loaded before events generated

        int widgetArea = window->width() * window->height();
        if (((float)widgetArea / (float)desktopArea) < 0.75f)
            window->show();
        else
            window->showMaximized();
        window->run_viewer();
        int status = app.exec();
        return status;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

void parseCommandLine(int argc, char *argv[])
{
    // parse command line
    if (argc < 4) printUsage();

    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];
        cerr << arg << endl;
        if (arg == "-lprefix")
        {
            if (i+1 >= argc) printError("-lprefix must have an argument");
            leftprefix = argv[++i];
        }
        if (arg == "-rprefix")
        {
            if (i+1 >= argc) printError("-rprefix must have an argument");
            rightprefix = argv[++i];
        }
        else if (arg == "-prefix")
        {
            if (i+1 >= argc) printError("-prefix must have an argument");
            leftprefix = argv[++i];
            rightprefix = leftprefix;
        }
        else if (arg[0] == '-')
        {
            if (i+1 >= argc)  printError("invalid paramter specified");
        }
        else
        {
           dir = argv[i];
           // break;
        }
    }

    if (leftprefix == "" || rightprefix == "")
        printError("Scene files basename not specied for left or right");
}

void printUsage(void)
{
 char info[] = "Usage: ecoviz <params> <data directory>\n\
parameters:\n\
-lprefix <string>     --- the base name of elevation file and for sequence of cohort maps in left window\n\
-rprefix <string>     --- the base name of elevation file and for sequence of cohort maps in right window\n\
-prefix <string>      --- the base name of elevation file and for sequence of cohort maps in the left and right windows (the same files used)\n\
where -[l|r]prefix specifies the prefix of files for left, right or both scenes. The elevation file will have this prefix as its name\n\
and the extension .elv or .elvb (the latter for a binary version). The cohort files will be <prefix>i.pdb where i starts at 0. If the\n\
binary version are to be loaded, the program will search for <prefix>i.pdbb. These files must be in the specified directory.\n";

   std::cerr<< info;
    exit(0);
}

void printError(const std::string & mesg)
{
  std::cerr << mesg << std::endl;
  exit(0);
}
