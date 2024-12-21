//
// Created by Sidney on 13-Jun-18.
//

#ifndef SPIRV_STUDIO_DOCUMENT_WINDOW_H
#define SPIRV_STUDIO_DOCUMENT_WINDOW_H

#include <ui_document_window.h>
#include <model/telemetry_document.h>
#include <model/xplane_installation.h>

class test_runner_dialog;

class document_window final : public QMainWindow, public Ui::document_window
{
Q_OBJECT
public:
	document_window();
	~document_window() override;

	void set_document(telemetry_document *document);
	void set_document_by_path(const QString &path);

	void add_document(telemetry_document *document);

	void restore_state(QSettings &state);
	void save_state(QSettings &state) const;

	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dragLeaveEvent(QDragLeaveEvent *event) override;
	void dropEvent(QDropEvent *event) override;

	bool can_accept_mime_data(const QMimeData *mime) const;

protected:
	void closeEvent(QCloseEvent *event) override;

private slots:
	void open_file();
	void save_file();

	void run_fps_test();

	void range_changed();
	void event_range_changed(int index);

private:
	struct event_range
	{
		int32_t start, end;
		QString name;
	};

	struct additional_document
	{
		telemetry_document *document;
		int32_t start_offset;
		QVector<event_range> event_ranges;
	};

	void clear();
	void set_field_enabled(const telemetry_field *field, bool enable);

	void set_time_range(int32_t start, int32_t end);
	void touch_telemetry_file(const QFileInfo &file_info);

	QColor generate_color_for_title(const QString &title) const;

	telemetry_document *m_document;
	QVector<additional_document> m_additional_documents;
	QVector<const telemetry_field *> m_enabled_fields;

	QString m_base_dir;
	std::vector<std::unique_ptr<QAction>> m_recent_file_actions;

	QVector<event_range> m_event_ranges;
	QVector<xplane_installation> m_installations;
};

#endif //SPIRV_STUDIO_DOCUMENT_WINDOW_H
