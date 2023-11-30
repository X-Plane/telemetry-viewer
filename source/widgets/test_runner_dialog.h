//
// Created by Sidney on 29/11/2023.
//

#ifndef TEST_RUNNER_DIALOG_H
#define TEST_RUNNER_DIALOG_H

#include <QDialog>
#include <ui_test_runner_dialog.h>
#include "../utilities/xplane_installations.h"

class test_runner_dialog final : public QDialog, public Ui::test_runner_dialog
{
Q_OBJECT

public:
	test_runner_dialog(xplane_installation *installation);

	QString get_executable() const;
	QStringList get_arguments(const QString &telemetry_path = "") const;

private:
	uint32_t get_fps_test() const;

	xplane_installation *m_installation;

	QVector<QString> m_replay_files;
};

#endif //TEST_RUNNER_DIALOG_H
