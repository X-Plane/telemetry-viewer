//
// Created by Sidney on 13-Jun-18.
//

#ifndef SPIRV_STUDIO_DOCUMENT_WINDOW_H
#define SPIRV_STUDIO_DOCUMENT_WINDOW_H

#include <ui_DocumentWindow.h>
#include <model/TelemetryDocument.h>
#include <model/XplaneInstallation.h>

class TestRunnerDialog;

class DocumentWindow final : public QMainWindow, public Ui::DocumentWindow
{
Q_OBJECT
public:
	DocumentWindow();
	~DocumentWindow() override;

	void set_document(TelemetryDocument *document);
	void set_document_by_path(const QString &path);

	void restore_state(QSettings &state);
	void save_state(QSettings &state) const;

	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dragLeaveEvent(QDragLeaveEvent *event) override;
	void dropEvent(QDropEvent *event) override;

	bool can_accept_mime_data(const QMimeData *mime) const;

protected:
	void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
	[[maybe_unused]] void new_file();
	[[maybe_unused]] void open_file();
	[[maybe_unused]] void save_file();

	[[maybe_unused]] void run_fps_test();

	[[maybe_unused]] void range_changed();
	[[maybe_unused]] void event_range_changed(int index);

	[[maybe_unused]] void populate_recent_items();
	[[maybe_unused]] void clear_recent_items();

	[[maybe_unused]] void provider_item_changed(QTreeWidgetItem *item);

private:
	struct event_range
	{
		int32_t start, end;
		QString name;
	};

	void clear();

	void set_time_range(int32_t start, int32_t end);
	void touch_telemetry_file(const QFileInfo &file_info);

	QColor generate_color_for_title(const QString &title) const;

	QString m_base_dir;

	TelemetryDocument *m_document;
	std::vector<std::unique_ptr<QAction>> m_recent_file_actions;

	QVector<event_range> m_event_ranges;
	QVector<XplaneInstallation> m_installations;
};

#endif //SPIRV_STUDIO_DOCUMENT_WINDOW_H
