/**
 * src/main.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "mainwindow.h"

#include <QApplication>

#include <kddockwidgets/Config.h>
#include <kddockwidgets/MainWindow.h>

#include "info.h"

#include <QDateTime>
#include <QStandardPaths>
#include <QDir>

#include <QLibraryInfo>
#include <QTranslator>

#include <QSettings>

#include "views/layoutloader.h"

QString locateAppDataPath()
{
    QString appDataPath = QStringLiteral("%1/%2/%3")
            .arg(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation),
                 AppCompany, AppProductShort);
    appDataPath = QDir::cleanPath(appDataPath);
    qDebug() << appDataPath;
    return appDataPath;
}

QString locateAppSettings()
{
    return locateAppDataPath() + "/simra_settings.ini";
}

constexpr const char *simraTranslationName     = "simra";
static const QLatin1String translationsFolder = QLatin1String("/translations");

static inline QTranslator *loadTranslatorInternal(const QLocale &loc, const QString &path,
                                                  const QString &prefix)
{
    QTranslator *translator = new QTranslator(qApp);
    if (!translator->load(loc, prefix, QStringLiteral("_"), path))
    {
        qDebug() << "Cannot load translations for:" << prefix.toUpper() << loc.name()
                 << loc.uiLanguages();
        delete translator;
        return nullptr;
    }
    return translator;
}

QTranslator *loadAppTranslator(const QLocale &loc)
{
    const QString path = qApp->applicationDirPath() + translationsFolder;
    return loadTranslatorInternal(loc, path, QString::fromLatin1(simraTranslationName));
}

bool loadTranslationsFromSettings()
{
    const QString localPath = QCoreApplication::applicationDirPath() + translationsFolder;
    const QString qtLibPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    QLocale loc = QLocale::English;

    QSettings settings(locateAppSettings(), QSettings::IniFormat);
    QString lang = settings.value("language", QLatin1String("it")).toString();
    if(lang == QLatin1String("it"))
        loc = QLocale::Italian;

    // Sync settings with default value
    settings.setValue("language", lang);

    // NOTE: If locale is English with default country we do not need translations
    // because they are already embedded in the executable strings so skip it
    if (loc == QLocale::English)
        return true;

    QTranslator *qtTransl = ::loadTranslatorInternal(loc, qtLibPath, QLatin1String("qt"));
    if (qtTransl)
    {
        QCoreApplication::installTranslator(qtTransl);
    }

    QTranslator *simraTransl =
            ::loadTranslatorInternal(loc, localPath, QString::fromLatin1(simraTranslationName));
    if (simraTransl)
    {
        QCoreApplication::installTranslator(simraTransl);
        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(AppCompany);
    QApplication::setApplicationName(AppProduct);
    QApplication::setApplicationDisplayName(AppDisplayName);
    QApplication::setApplicationVersion(AppVersion);

    loadTranslationsFromSettings();

    qDebug() << QApplication::applicationDisplayName()
             << "Version:" << QApplication::applicationVersion() << "Built:" << AppBuildDate
             << "Website: " << AppProjectWebSite;
    qDebug() << "Qt:" << QT_VERSION_STR;
    qDebug() << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");

    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);

    LayoutLoader::registerLoader();

    auto& config = KDDockWidgets::Config::self();

    auto flags = config.flags();
    flags.setFlag(KDDockWidgets::Config::Flag_AllowReorderTabs);
    config.setFlags(flags);

    MainWindow w(QLatin1String("mainwindow1"), locateAppSettings());

    if (argc > 1) // FIXME: better handling if there are extra arguments
    {
        QString fileName = app.arguments().at(1);
        qDebug() << "Trying to load:" << fileName;
        if (QFile(fileName).exists())
        {
            w.loadFile(app.arguments().at(1));
        }
    }

    w.showMaximized();
    return app.exec();
}
