//
// Created by Sidney on 13/12/2024.
//

#include <QPalette>
#include <QStyleFactory>
#include "Application.h"
#include "widgets/DocumentWindow.h"

#define MAX_RECENTLY_OPENED 5

QString apply_dark_theme()
{
	// https://gist.github.com/QuantumCD/6245215
	QPalette palette;
	palette.setColor(QPalette::Window, QColor(53,53,53));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(25,25,25));
	palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, Qt::white);
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(53,53,53));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::red);
	palette.setColor(QPalette::Link, QColor(42, 130, 218));

	palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
	palette.setColor(QPalette::HighlightedText, Qt::black);

	QApplication::setPalette(palette);
	QApplication::setStyle(QStyleFactory::create("Fusion"));

	return "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }";
}

QString Application::get_settings_path()
{
	return applicationDirPath() + "/settings.ini";
}

Application::Application(int &argc, char **argv) :
	QApplication(argc, argv),
	m_settings(get_settings_path(), QSettings::IniFormat)
{
	{
		const int count = m_settings.beginReadArray("documents");
		if(count == 0)
		{
			DocumentWindow *window = new DocumentWindow();
			window->show();

			m_document_windows.push_back(window);
		}
		else
		{
			for(int i = 0; i < count; ++ i)
			{
				m_settings.setArrayIndex(i);

				try
				{
					DocumentWindow *window = new DocumentWindow();
					window->restore_state(m_settings);
					window->show();

					m_document_windows.push_back(window);
				}
				catch(...)
				{}
			}
		}

		m_settings.endArray();
	}

	{
		const int count = m_settings.beginReadArray("recently_opened");

		for(int i = 0; i < count; ++ i)
		{
			m_settings.setArrayIndex(i);

			QString path = m_settings.value("path").toString();
			QFileInfo file(path);

			if(file.exists())
				m_recently_opened.push_back(file.canonicalPath() + "/" + file.fileName());
		}

		m_settings.endArray();
	}
}

Application::~Application()
{
	{
		m_settings.beginWriteArray("documents");

		int index = 0;

		for(auto &document : m_document_windows)
		{
			m_settings.setArrayIndex(index ++);
			document->save_state(m_settings);
		}

		m_settings.endArray();
	}

	{
		m_settings.beginWriteArray("recently_opened");

		int index = 0;

		for(auto &file : m_recently_opened)
		{
			m_settings.setArrayIndex(index ++);
			m_settings.setValue("path", file);
		}

		m_settings.endArray();
	}
}

void Application::new_file()
{
	DocumentWindow *window = new DocumentWindow();
	window->show();

	m_document_windows.push_back(window);
}
void Application::open_file(const QString &path)
{
	try
	{
		TelemetryDocument *document = load_file(path);

		DocumentWindow *window = new DocumentWindow();
		window->show();
		window->set_document(document);

		m_document_windows.push_back(window);
	}
	catch(...)
	{
		// TODO: Show an error pop up here
	}
}
void Application::close_document(DocumentWindow *window)
{
	// Last window
	if(m_document_windows.size() == 1)
	{
		qApp->quit();
		return;
	}

	auto iterator = std::find(m_document_windows.begin(), m_document_windows.end(), window);
	if(iterator != m_document_windows.end())
		m_document_windows.erase(iterator);

	delete window;
}


TelemetryDocument *Application::load_file(const QString &path)
{
	TelemetryDocument *document = TelemetryDocument::load_file(path);
	if(document)
	{
		QFileInfo file(path);
		QString real_path = file.canonicalPath() + "/" + file.fileName();

		bool has_file = false;

		for(auto &file : m_recently_opened)
		{
			if(file == real_path)
			{
				has_file = true;
				break;
			}
		}

		if(!has_file)
		{
			if(m_recently_opened.size() >= MAX_RECENTLY_OPENED)
				m_recently_opened.pop_back();

			m_recently_opened.push_front(real_path);
		}
	}

	return document;
}

std::vector<std::unique_ptr<QAction>> Application::get_recently_opened_files()
{
	std::vector<std::unique_ptr<QAction>> result;

	for(auto &path : m_recently_opened)
	{
		QFileInfo file(path);
		if(file.exists())
		{
			auto open_action = std::make_unique<QAction>(file.fileName());
			open_action->setData(path);

			result.push_back(std::move(open_action));
		}
	}

	return result;
}

void Application::clear_recently_opened_files()
{
	m_recently_opened.clear();
}


QVector<XplaneInstallation> Application::get_installations() const
{
	const QString install_file_name("x-plane_install_12.txt");

	QString installer_path;

#if WIN || APL
	installer_path = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/" + install_file_name;
#else
	installer_path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.x-plane/" + install_file_name;
#endif

	QFileInfo info(installer_path);

	if(!info.exists() || !info.isFile())
	{
		qDebug() << "No X-Plane install file found in " << installer_path;
		return {};
	}

	QFile file(info.filePath());

	if(!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "Failed to open " << installer_path;
		return {};
	}

	QVector<XplaneInstallation> installations;

	const size_t length = file.bytesAvailable();

	char *data = new char[length];
	file.read(data, length);
	file.close();

	char *start_token = data;
	char *token = start_token;

	while(token < data + length)
	{
		if(!isprint(*token))
		{
			if(token > start_token)
			{
				*token = '\0';

				QString install_path = QString(start_token);
				QFileInfo path_info(install_path);

				if(path_info.exists() && path_info.isDir())
				{
					XplaneInstallation install(install_path);
					installations.push_back(std::move(install));
				}
			}

			start_token = token + 1;
		}

		token ++;
	}

	delete[] data;

	return installations;
}



int Application::run(int &argc, char **argv)
{
	setApplicationName("Telemetry Viewer");
	setOrganizationName("Laminar Research");
	setApplicationVersion(QString::number(VERSION_MAJOR) % "." % QString::number(VERSION_MINOR) % "." % QString::number(VERSION_PATCH));

#if !LIN
	QString style_sheet = apply_dark_theme();

	Application app(argc, argv);
	app.setStyleSheet(style_sheet);
#else
	QApplication::setStyle(QStyleFactory::create("Fusion"));
	Application app(argc, argv);
#endif

	return exec();
}
