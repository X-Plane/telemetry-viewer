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
	DocumentWindow(TelemetryDocument *document = nullptr);
	~DocumentWindow() override;

	void add_document(TelemetryDocument *document);
	void add_document_by_path(const QString &path, const QString &name = "");

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

	[[maybe_unused]] void document_selection_changed(QTreeWidgetItem *item);
	[[maybe_unused]] void document_item_changed(QTreeWidgetItem *item);

private:
	struct loaded_document
	{
		TelemetryDocument *document;
		double start_offset;
		bool enabled = true;
		QString seed;
	};

	struct telemetry_field_lookup
	{
		std::string identifier;
		uint8_t field_id;

		auto operator<=>(const telemetry_field_lookup &) const = default;
	};

	void clear();
	void set_field_enabled(const telemetry_field_lookup &lookup, bool enable);
	const telemetry_field *lookup_field(const telemetry_field_lookup &lookup, TelemetryDocument *document) const;

	QColor get_color_for_telemetry_field(const telemetry_field *field, const loaded_document *document) const;

	void set_time_range(int32_t start, int32_t end);

	void update_selected_document(const loaded_document *document);
	void update_statistics_view();

	QAction *add_toolbar_widget(QWidget *widget, const QString &text) const;
	QAction *add_toolbar_spacer() const;

	QString m_base_dir;

	QVector<loaded_document *> m_loaded_documents;
	QVector<telemetry_field_lookup> m_enabled_fields;

	std::vector<std::unique_ptr<QAction>> m_recent_file_actions;

	QComboBox *m_installation_selector;
	QVector<XplaneInstallation> m_installations;
};

#endif //SPIRV_STUDIO_DOCUMENT_WINDOW_H
