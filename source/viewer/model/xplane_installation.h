//
// Created by Sidney on 21/12/2024.
//

#ifndef XPLANE_INSTALLATION_H
#define XPLANE_INSTALLATION_H

#include <QString>
#include <QVector>

class xplane_installation
{
public:
	xplane_installation(const QString &path);

	const QString &get_path() const { return m_path;}

	const QString &get_telemetry_path() const { return m_telemetry_path; }
	const QString &get_replay_path() const { return m_replay_path; }

	QVector<QString> get_executables() const;

private:
	QString m_path;
	QString m_telemetry_path;
	QString m_replay_path;
};

#endif //XPLANE_INSTALLATION_H
