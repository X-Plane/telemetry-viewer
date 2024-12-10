//
// Created by Sidney on 29/11/2023.
//

#ifndef XPLANE_INSTALLATIONS_H
#define XPLANE_INSTALLATIONS_H

#include <QString>
#include <QVector>

struct xplane_installation
{
	QString path;
	QString telemetry_path;
	QString replay_path;

	QVector<QString> executables;
};

QVector<xplane_installation> get_xplane_installations();
QString xplaneify_path(const QString &full_path);

#endif //XPLANE_INSTALLATIONS_H
