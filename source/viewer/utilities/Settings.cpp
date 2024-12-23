//
// Created by Sidney on 11/03/2024.
//

#include <QApplication>
#include "Settings.h"

QSettings open_settings()
{
	QString settings_file = QApplication::applicationDirPath() + "/settings.ini";
	return QSettings(settings_file, QSettings::IniFormat);
}
