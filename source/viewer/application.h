//
// Created by Sidney on 13/12/2024.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QSettings>

class document_window;
class telemetry_document;

class application : public QApplication
{
public:
	static int run(int &argc, char **argv);
	static QString get_settings_path();

	void new_file();
	void open_file(const QString &path);

	void close_document(document_window *window);

	telemetry_document *load_file(const QString &path);

	std::vector<std::unique_ptr<QAction>> get_recently_opened_files();
	void clear_recently_opened_files();

protected:
	application(int &argc, char **argv);
	~application() override;

private:
	QSettings m_settings;
	QVector<document_window *> m_document_windows;
	QVector<QString> m_recently_opened;
};

#undef qApp
#define qApp static_cast<application *>(application::instance())

#endif //APPLICATION_H
