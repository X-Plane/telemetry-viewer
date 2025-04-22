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

	void add_document(TelemetryDocument *document);

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
	struct loaded_document
	{
		TelemetryDocument *document;
		double start_offset;
	};

	void clear();
	void set_field_enabled(const telemetry_field *field, bool enable);

	void set_time_range(int32_t start, int32_t end);
	void touch_telemetry_file(const QFileInfo &file_info);

	loaded_document &get_selected_document();
	const loaded_document &get_selected_document() const;

	QAction *add_toolbar_widget(QWidget *widget, const QString &text) const;
	QAction *add_toolbar_spacer() const;

	QString m_base_dir;

	QVector<loaded_document> m_loaded_documents;
	QVector<const telemetry_field *> m_enabled_fields;

	std::vector<std::unique_ptr<QAction>> m_recent_file_actions;

	QComboBox *m_installation_selector;
	QVector<XplaneInstallation> m_installations;
};

#endif //SPIRV_STUDIO_DOCUMENT_WINDOW_H
