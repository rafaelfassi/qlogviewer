// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include <QFont>

class QSettings;

class Settings
{
public:
    static void initSettings();

    static const QFont &getFont() { return inst().m_font; }

    static QDir getSettingsDir(const QString &subDir = {});

    static tp::SInt getMaxRecentFiles();
    static std::vector<FileConf::Ptr> getRecentFiles();
    static void setRecentFile(const FileConf::Ptr &conf);

    static FileConf::Ptr findConfByTemplateFileName(const std::string &templateFileName);
    static void loadTemplates();
    static void saveTemplate(FileConf::Ptr conf);
    static void saveTemplateAs(FileConf::Ptr conf, const QString &name);
    static std::vector<FileConf::Ptr> getTemplates();

    static void setStyle(const QString styleName);
    static QString getStyle();

private:
    Settings() = default;
    static Settings &inst();

    QFont m_font;
    QDir m_settingsDir;
    QDir m_templatesDir;
    QSettings *m_settings;
    std::vector<FileConf::Ptr> m_templates;
};
