#include "apkunpacker.h"
#include "globals.h"
#include <QApplication>
#include <QProcess>
#include <QDebug>
#include <QtXml/QDomDocument>
#include <QuaZIP/JlCompress.h>

using Apk::Unpacker;

bool Unpacker::unpack(QString filepath, QString destination, QString frameworks, bool apktool, bool smali)
{
    // Read APK manifest:

    emit loading(20, tr("Reading AndroidManifest.xml..."));
    const QString DEST = QDir::fromNativeSeparators(destination);
    const QString MANIFEST = getManifest(filepath);
    if (MANIFEST.isEmpty()) {
        return false;
    }

    // Unpack APK;

    // TODO: Loading text is repeated due to avoiding excessive translation values.
    emit loading(40, tr("Unpacking APK..."));
    QDir(DEST).removeRecursively();

    // TODO: Loading text is repeated due to avoiding excessive translation values.
    emit loading(60, tr("Unpacking APK..."));
    if (!apktool) {
        if (!unzip(filepath, DEST)) {
            return false;
        }
    }
    else {
        if (!unzip(filepath, DEST, frameworks, smali)) {
            return false;
        }
    }

    // Load APK icons and strings:

    // TODO: Loading text is repeated due to avoiding excessive translation values.
    emit loading(80, tr("Unpacking APK..."));
    QList<QSharedPointer<Icon> > icons = getIcons(MANIFEST, DEST);
    QList<Apk::String> strings;
    const QString VAR_APPNAME = getVarAppTitle(DEST);
    if (!VAR_APPNAME.isEmpty()) {
        emit loading(85, tr("Parsing string resources..."));
        strings = getTranslations(DEST, VAR_APPNAME);
    }

    // Done!

    qDebug() << "Done.\n";
    emit loading(100, tr("APK successfully loaded"));
    Apk::File *apk = new Apk::File;
    apk->setFilePath(filepath);
    apk->setDirectory(DEST);
    apk->setAppTitle(getApplicationLabel(MANIFEST));
    apk->setVarAppTitle(VAR_APPNAME);
    apk->setVersionCode(getVersionCode(MANIFEST));
    apk->setVersionName(getVersionName(MANIFEST));
    // Hack: handle the situation when the banner file is present but the manifest entry is not:
    if ((icons[Dpi::BANNER].data())->isNull()) {
        const QString bannerFile(DEST + "/res/drawable-xhdpi-v4/banner.png");
        if (QFile::exists(bannerFile)) {
            apk->addAndroidTV();
        }
        icons[Dpi::BANNER].data()->load(bannerFile);
    }
    // Hack end.
    apk->setIcons(icons);
    apk->setStrings(strings);
    emit unpacked(apk);
    return true;
}

bool Unpacker::unzip(QString filename, QString destination)
{
    QDir dir(destination);
    dir.mkpath(".");

    QTime sw;
    sw.start();
    QStringList result = JlCompress::extractDir(filename, destination);
    qDebug() << qPrintable(QString("Unpacked in %1s").arg(sw.elapsed() / 1000.0));

    if (!result.isEmpty()) {
        return true;
    }
    else {
        emit error(Apk::ERROR.arg("QuaZIP"));
        return false;
    }
}

bool Unpacker::unzip(QString filename, QString destination, QString frameworks, bool smali) const
{
    QProcess p;
    QTime sw;
    sw.start();
    p.start(QString("java -jar \"%1/apktool.jar\" d \"%2\" -f %3 -o \"%4\" -p \"%5\"")
            .arg(Path::Data::shared(), filename, (smali ? "" : "-s"), destination, frameworks));
    if (!p.waitForStarted(-1)) {
        if (isJavaInstalled()) {
            qDebug() << "Error starting Apktool";
            qDebug() << "Error:" << p.errorString();
            emit error(Apk::ERRORSTART.arg("Apktool"));
            return false;
        }
        else {
            emit error(Apk::NOJAVA + "<br>" + Apk::GETJAVA);
            return false;
        }
    }
    p.waitForFinished(-1);
    qDebug() << qPrintable(QString("Unpacked in %1s").arg(sw.elapsed() / 1000.0));
    if (!p.exitCode()) {
        return true;
    }
    else {
        qDebug() << p.readAllStandardError().replace("\r\n", "\n");
        emit error(Apk::ERROR.arg("Apktool"));
        return false;
    }
}

QString Unpacker::getManifest(QString filename) const
{
    QString manifest;

    QProcess p;
    p.start(QString("aapt dump badging \"%1\"").arg(filename));
    if (!p.waitForStarted(-1)) {
        qDebug() << qPrintable("Could not start aapt");
        qDebug() << "Error:" << p.errorString();
        emit error(Apk::ERRORSTART.arg("aapt"));
        return QString();
    }
    p.waitForFinished(-1);

    const int CODE = p.exitCode();
    qDebug() << "aapt exit code:" << CODE;

    switch (CODE) {
    case 1:
        qDebug() << "AndroidManifest.xml contains uncritical errors.";
        // Continue reading AndroidManifest.xml:
    case 0:
        manifest = p.readAllStandardOutput().replace("\r\n", "\n");
        qDebug() << "\n--- DUMP START ---\n" << qPrintable(manifest) << "--- DUMP END ---\n";
        qDebug() << "Application name:" << getApplicationLabel(manifest);
        qDebug() << "Application version code:" << getVersionCode(manifest);
        qDebug() << "Application version name:" << getVersionName(manifest);
        return manifest;
    default:
        qDebug() << p.readAllStandardError().replace("\r\n", "\n");
        emit error(tr("Error reading APK."));
        return QString();
    }
    return QString();
}

QList<QSharedPointer<Icon> > Unpacker::getIcons(QString manifest, QString contents)
{
    // Parse filenames:

    QStringList files;
    files.push_back(parse("application-icon-120:'(.*)'", manifest));
    files.push_back(parse("application-icon-160:'(.*)'", manifest));
    files.push_back(parse("application-icon-240:'(.*)'", manifest));
    files.push_back(parse("application-icon-320:'(.*)'", manifest));
    files.push_back(parse("application-icon-480:'(.*)'", manifest));
    files.push_back(parse("application-icon-640:'(.*)'", manifest));
    files.push_back(parse("banner='(.*)'", manifest));
    qDebug() << "Icons:" << files;

    // Load icons:

    QList<QSharedPointer<Icon> > icons;

    for (short i = Dpi::LDPI; i < Dpi::COUNT; ++i) {

        QString resource = files[i];
        if (!QFile::exists(contents + "/" + resource)) {
            QString directory = resource.section('/', 0, 1);
            QString filename = resource.section('/', 2);
            if (directory.endsWith("-v4")) {
                // Try to remove "-v4" qualifier to handle Apktool behaviour:
                directory.chop(3);
                const QString v0 = directory + "/" + filename;
                if (QFile::exists(contents + "/" + v0)) {
                    qDebug() << "Removing '-v4' qualifier.";
                    resource = v0;
                }
            } else {
                // Try to add "-v4" qualifier to handle Apktool behaviour:
                const QString v4 = directory + "-v4/" + filename;
                if (QFile::exists(contents + "/" + v4)) {
                    qDebug() << "Adding '-v4' qualifier.";
                    resource = v4;
                }
            }
        }
        if (resource.isEmpty()) {
            // Create dummy entry
            icons.push_back(QSharedPointer<Icon>(new Icon));
            continue;
        }
        const int DUPL = files.lastIndexOf(resource, i - 1);
        if (i == 0 || DUPL == -1) {
            // Create new entry
            icons.push_back(QSharedPointer<Icon>(new Icon(contents + "/" + resource)));
        }
        else {
            // Reuse existing entry
            icons.push_back(icons[DUPL]);
        }
    }

    return icons;
}

QString Unpacker::getVarAppTitle(QString contents) const
{
    QString manifest;

    QFile f(contents + "/AndroidManifest.xml");
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        manifest = f.readAll();
        f.close();
    }
    else {
        return QString();
    }

    // Parse application title variable name:

    QDomDocument dom;
    dom.setContent(manifest);
    QDomNodeList nodes = dom.elementsByTagName("application");
    if (!nodes.isEmpty()) {
        QDomNamedNodeMap attr = nodes.at(0).attributes();
        const QString LABEL = attr.namedItem("android:label").nodeValue();
        if (LABEL.startsWith("@string/")) {
            return LABEL.mid(8);
        }
        else {
            return QString();
        }
    }
    else {
        return QString();
    }
}

QList<Apk::String> Unpacker::getTranslations(QString contents, QString key)
{
    QList<Apk::String> strings;
    QDir dir(contents + "/res");
    QStringList langs = dir.entryList(QStringList("values*"), QDir::Dirs);
    for (int i = 0; i < langs.size(); ++i) {
        QFile f(dir.absolutePath() + '/' + langs[i] + "/strings.xml");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&f);
            while (!stream.atEnd()) {
                QString line = stream.readLine();
                QString rx = QString("<string name=\"%1\">(.+)</string>").arg(key);
                const QString TRANSLATION = parse(rx, line);
                if (!TRANSLATION.isEmpty()) {
                    strings << Apk::String(key, TRANSLATION, f.fileName());
                }
            }
            f.close();
        }
    }
    return strings;
}

QString Unpacker::getPackageName(QString manifest) const { return parse("package: name='(.+)'", manifest); }
QString Unpacker::getVersionCode(QString manifest) const { return parse("versionCode='(.+)'", manifest); }
QString Unpacker::getVersionName(QString manifest) const { return parse("versionName='(.+)'", manifest); }
QString Unpacker::getApplicationLabel(QString manifest) const { return parse("application-label:'(.+)'", manifest); }
QString Unpacker::getMinimumSdk(QString manifest) const { return parse("sdkVersion:'(.+)'", manifest); }
QString Unpacker::getTargetSdk(QString manifest) const { return parse("targetSdkVersion:'(.+)'", manifest); }

QString Unpacker::parse(QString regexp, QString str) const
{
    QRegExp rx;
    rx.setMinimal(true);
    rx.setPattern(regexp);
    rx.indexIn(str);
    if (rx.capturedTexts().size() > 1)
        return rx.capturedTexts().at(1);
    else
        return QString();
}
