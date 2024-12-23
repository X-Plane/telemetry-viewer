//
// Created by Sidney on 13/12/2024.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QSettings>
#include <QAction>

#include <model/XplaneInstallation.h>

class DocumentWindow;
class TelemetryDocument;

class Application : public QApplication
{
public:
	static int run(int &argc, char **argv);
	static QString get_settings_path();

	void new_file();
	void open_file(const QString &path);

	void close_document(DocumentWindow *window);

	TelemetryDocument *load_file(const QString &path);

	std::vector<std::unique_ptr<QAction>> get_recently_opened_files();
	void clear_recently_opened_files();

	QVector<XplaneInstallation> get_installations() const;

protected:
	Application(int &argc, char **argv);
	~Application() override;

private:
	QSettings m_settings;
	QVector<DocumentWindow *> m_document_windows;
	QVector<QString> m_recently_opened;
};

#undef qApp
#define qApp static_cast<Application *>(Application::instance())

#endif //APPLICATION_H
