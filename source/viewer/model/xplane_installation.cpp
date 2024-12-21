//
// Created by Sidney on 21/12/2024.
//

#include <QFileInfo>
#include <QDir>
#include "xplane_installation.h"

xplane_installation::xplane_installation(const QString &path) :
	m_path(path)
{
	QFileInfo info(path);

	Q_ASSERT(info.exists());
	Q_ASSERT(info.isDir());

	m_path = info.absoluteFilePath();
	m_telemetry_path = m_path + "/Output/diagnostic reports";
	m_replay_path = m_path + "/Output/replays";

	qDebug() << m_path << m_telemetry_path << m_replay_path;
}

QVector<QString> xplane_installation::get_executables() const
{
	QVector<QString> result;

	QDir directory(m_path);

	for(auto &file : directory.entryInfoList())
	{
		// Skip over the installer
		if(file.fileName().contains("Installer"))
			continue;

#if WIN
		if(file.isFile() && file.fileName().startsWith("X-Plane") && file.fileName().endsWith("exe"))
		{
			result.push_back(file.fileName());
		}
#elif APL
		if(file.isDir() && file.fileName().startsWith("X-Plane") && file.fileName().endsWith("app"))
		{
			QDir subdir(file.filePath() + "/" + "Contents/MacOS");
			QFileInfoList subentries = subdir.entryInfoList();
			// e.g. X-Plane_NODEV_OPT.app/Contents/MacOS/X-Plane_NODEV_OPT
			for(auto const &subentry : subentries)
			{
				if(subentry.isFile() && subentry.fileName().startsWith("X-Plane"))
				{
					auto name = file.fileName() + "/Contents/MacOS/" + subentry.fileName();
					result.push_back(name);

					break;
				}
			}
		}
#else
		if(file.isFile() && file.fileName().startsWith("X-Plane") && file.fileName().endsWith("-x86_64"))
		{
			result.push_back(file.fileName());
		}
#endif
	}

	return result;
}


