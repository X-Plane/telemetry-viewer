//
// Created by Sidney on 29/11/2023.
//

#ifndef TEST_RUNNER_DIALOG_H
#define TEST_RUNNER_DIALOG_H

#include <ui_TestRunnerDialog.h>

class XplaneInstallation;

class TestRunnerDialog final : public QDialog, public Ui::TestRunnerDialog
{
Q_OBJECT

public:
	TestRunnerDialog(XplaneInstallation *installation);

	XplaneInstallation *get_installation() const { return m_installation; }

	QString get_executable() const;
	QStringList get_arguments(const QString &telemetry_path, bool escape_paths) const;

	QString get_name() const;

private:
	uint32_t get_fps_test() const;

	void combo_box_selection_changed(int index);
	void line_text_changed(const QString &text);

	void load_settings();
	void save_settings();

	XplaneInstallation *m_installation;

	QVector<QString> m_replay_files;
	QVector<QString> m_executables;
};

#endif //TEST_RUNNER_DIALOG_H
