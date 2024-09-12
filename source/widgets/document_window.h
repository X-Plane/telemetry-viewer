//
// Created by Sidney on 13-Jun-18.
//

#ifndef SPIRV_STUDIO_DOCUMENT_WINDOW_H
#define SPIRV_STUDIO_DOCUMENT_WINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QSettings>
#include <QFileInfo>
#include <ui_document_window.h>

#include "utilities/xplane_installations.h"
#include "model/telemetry_container.h"
#include "model/generic_tree_model.h"

class test_runner_dialog;

class document_window final : public QMainWindow, public Ui::document_window, public generic_tree_model_delegate
{
Q_OBJECT
public:
	document_window();
	document_window(QSettings &state);
	~document_window() override;

	static void restore_state();
	static void store_state();

	void load_file(const QString &path);
	bool tree_model_data_did_change(generic_tree_model *model, generic_tree_item *item, int index, const QVariant &data) override;

protected:
	void closeEvent(QCloseEvent *event) override;

private slots:
	void new_file();
	void open_file();
	void save_file();
	void clear_recent_files();

	void run_fps_test();

	void range_changed(int32_t value);
	void event_range_changed(int index);
	void mode_changed(int index);

private:
	struct event_range
	{
		int32_t start, end;
		QString name;
	};

	void save_state(QSettings &state);
	void set_time_range(int32_t start, int32_t end);

	void update_telemetry();
	void touch_telemetry_file(const QFileInfo &file_info);

	QString m_base_dir;
	QVector<QAction *> m_recent_file_actions;

	telemetry_container m_telemetry;
	std::vector<event_range> m_event_ranges;

	QVector<xplane_installation> m_installations;

	document_window *m_next_window;
};

#endif //SPIRV_STUDIO_DOCUMENT_WINDOW_H
