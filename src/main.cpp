// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "MainWindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QStyleFactory>
#include <QStyle>
#include <QDebug>
#include "Settings.h"
#include "Style.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<tp::SInt>("tp::SInt");
    qRegisterMetaType<tp::UInt>("tp::UInt");
    qRegisterMetaType<tp::SharedSIntList>("tp::SharedSIntList");

    QApplication a(argc, argv);
    QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
    Settings::initSettings();
    Style::initStyle();
    Style::loadStyle(Settings::getStyle());

    qDebug() << Style::availableStyles();

    MainWindow w;
    w.show();
    return a.exec();
}
