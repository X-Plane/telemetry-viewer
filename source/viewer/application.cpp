//
// Created by Sidney on 13/12/2024.
//

#include <QPalette>
#include <QStyleFactory>
#include "application.h"
#include "widgets/document_window.h"

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

QString application::get_settings_path()
{
	return applicationDirPath() + "/settings.ini";
}

application::application(int &argc, char **argv) :
	QApplication(argc, argv),
	m_settings(get_settings_path(), QSettings::IniFormat)
{
	{
		const int count = m_settings.beginReadArray("documents");
		if(count == 0)
		{
			document_window *window = new document_window();
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
					document_window *window = new document_window();
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

application::~application()
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

void application::new_file()
{
	document_window *window = new document_window();
	window->show();

	m_document_windows.push_back(window);
}
void application::open_file(const QString &path)
{
	try
	{
		telemetry_document *document = load_file(path);

		document_window *window = new document_window();
		window->show();
		window->set_document(document);

		m_document_windows.push_back(window);
	}
	catch(...)
	{
		// TODO: Show an error pop up here
	}
}
void application::close_document(document_window *window)
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


telemetry_document *application::load_file(const QString &path)
{
	telemetry_document *document = telemetry_document::load_file(path);
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

std::vector<std::unique_ptr<QAction>> application::get_recently_opened_files()
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

void application::clear_recently_opened_files()
{
	m_recently_opened.clear();
}


int application::run(int &argc, char **argv)
{
	setApplicationName("Telemetry Viewer");
	setOrganizationName("Laminar Research");
	setApplicationVersion(QString::number(VERSION_MAJOR) % "." % QString::number(VERSION_MINOR) % "." % QString::number(VERSION_PATCH));

#if !LIN
	QString style_sheet = apply_dark_theme();

	application app(argc, argv);
	app.setStyleSheet(style_sheet);
#else
	QApplication::setStyle(QStyleFactory::create("Fusion"));
	application app(argc, argv);
#endif

	return exec();
}