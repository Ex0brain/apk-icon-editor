///
/// \file
/// This file declares global constants and functions.
///

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QApplication>
#include <QStandardPaths>

///
/// This namespace incorporates the GUI-related constants and functions.
///

namespace Gui {

    /// This namespace contains the functions related to the screen properties.

    namespace Screen {
        int dpi();                  ///< Returns the current screen DPI value.
        int scaled(int size);       ///< Returns the size scaled to the current screen DPI value.
        QSize scaled(int w, int h); ///< Returns the size scaled to the current screen DPI value.
        QSize scaled(QSize size);   ///< Returns the size scaled to the current screen DPI value.
    }
}

///
/// This namespace incorporates the constants and functions related to the application and system paths.
///

namespace Path {

    /// This namespace contains the application executable paths.

    namespace App {
        QString dir();  ///< Returns the directory containing the application executable.
        QString file(); ///< Returns the file path of the application executable.
    }

    /// This namespace contains the log paths.

    namespace Log {
        /// Full path to the log directory.
        const QString DIR = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/apk-icon-editor/";

        /// Full path to the log file.
        const QString FILE = DIR + "apk-icon-editor.log";
    }
}

///
/// This namespace contains the URL constants.
///

namespace Url {
    const QString WEBSITE   = "http://kefir500.github.io/apk-icon-editor/";
    const QString VERSION   = WEBSITE + "VERSION-2";
    const QString UPDATE    = WEBSITE + "download/#update";
    const QString CONTACT   = WEBSITE + "#contact";
    const QString TRANSLATE = "https://www.transifex.com/projects/p/apk-icon-editor/";
    const QString JRE       = "https://www.java.com/en/download/manual.jsp";
    const QString JDK       = "http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html";
}

#endif // GLOBALS_H
