//
// Created by Sidney on 29/11/2023.
//

#include <QDir>
#include <QClipboard>
#include <model/XplaneInstallation.h>
#include <utilities/Settings.h>

#include "TestRunnerDialog.h"

TestRunnerDialog::TestRunnerDialog(XplaneInstallation *installation) :
	m_installation(installation)
{
	setupUi(this);
	setWindowTitle(QString("Run FPS Test"));

	m_executables = installation->get_executables();

	for(auto &executable : m_executables)
		m_executable->addItem(executable);

	QDir replay_dir(m_installation->get_replay_path());

	for(auto &file : replay_dir.entryInfoList())
	{
		if(file.isFile() && file.fileName().endsWith("fps"))
		{
			m_replay_files.push_back(file.fileName());
			m_replay_file->addItem(file.fileName());
		}
	}

	m_weather_preset->addItem("Clear");
	m_weather_preset->addItem("VFR Few Clouds");
	m_weather_preset->addItem("VFR Scattered");
	m_weather_preset->addItem("VFR Broken");
	m_weather_preset->addItem("Marginal VFR Overcast");
	m_weather_preset->addItem("IFR Non-Precision");
	m_weather_preset->addItem("IFR Precision");
	m_weather_preset->addItem("Convective");
	m_weather_preset->addItem("Large-Cell Thunder-Storms");

	m_settings_preset->addItem("1");
	m_settings_preset->addItem("2");
	m_settings_preset->addItem("3");
	m_settings_preset->addItem("4");
	m_settings_preset->addItem("5");

	m_tod_box->addItem("Day");
	m_tod_box->addItem("Night");

	m_resolution_preset->addItem("Use current resolution");
	m_resolution_preset->addItem("Fullscreen 1080p");
	m_resolution_preset->addItem("Fullscreen 1440p");
	m_resolution_preset->addItem("Window 1080p");
	m_resolution_preset->addItem("Window 1440p");

	load_settings();

	connect(m_executable, qOverload<int>(&QComboBox::currentIndexChanged), this, &TestRunnerDialog::combo_box_selection_changed);
	connect(m_replay_file, qOverload<int>(&QComboBox::currentIndexChanged), this, &TestRunnerDialog::combo_box_selection_changed);
	connect(m_weather_preset, qOverload<int>(&QComboBox::currentIndexChanged), this, &TestRunnerDialog::combo_box_selection_changed);
	connect(m_settings_preset, qOverload<int>(&QComboBox::currentIndexChanged), this, &TestRunnerDialog::combo_box_selection_changed);
	connect(m_resolution_preset, qOverload<int>(&QComboBox::currentIndexChanged), this, &TestRunnerDialog::combo_box_selection_changed);

	connect(m_additional_commands, &QLineEdit::textChanged, this, &TestRunnerDialog::line_text_changed);

	connect(m_copy_to_clipboard, &QAbstractButton::pressed, [this] {

		QStringList arguments = get_arguments("", true);

		QClipboard *clipboard = QGuiApplication::clipboard();
		clipboard->setText(arguments.join(' '));
	});
}

void TestRunnerDialog::load_settings()
{
	QSettings settings = open_settings();

	settings.beginGroup("test_runner");

	if(settings.value("version", 1).toInt() != 1)
	{
		settings.endGroup();
		return;
	}


	{
		const QString executable = settings.value("executable", "").toString();

		for(int i = 0; i < m_executables.size(); ++ i)
		{
			if(m_executables[i] == executable)
			{
				m_executable->setCurrentIndex(i);
				break;
			}
		}
	}
	{
		const QString replay = settings.value("replay", "").toString();

		for(int i = 0; i < m_replay_files.size(); ++ i)
		{
			if(m_replay_files[i] == replay)
			{
				m_replay_file->setCurrentIndex(i);
				break;
			}
		}
	}

	{
		const int weather_preset = settings.value("weather_preset", 0).toInt();
		m_weather_preset->setCurrentIndex(weather_preset);
	}
	{
		const int settings_preset = settings.value("settings_preset", 0).toInt();
		m_settings_preset->setCurrentIndex(settings_preset);
	}
	{
		const int resolution_preset = settings.value("resolution_preset", 0).toInt();
		m_resolution_preset->setCurrentIndex(resolution_preset);
	}
	{
		const int tod_preset = settings.value("tod", 0).toInt();
		m_tod_box->setCurrentIndex(tod_preset);
	}

	m_additional_commands->setText(settings.value("additional_commands", "").toString());

	settings.endGroup();
}

void TestRunnerDialog::save_settings()
{
	QSettings settings = open_settings();

	settings.beginGroup("test_runner");

	settings.setValue("executable", m_executables[m_executable->currentIndex()]);
	settings.setValue("replay", m_replay_files[m_replay_file->currentIndex()]);

	settings.setValue("weather_preset", m_weather_preset->currentIndex());
	settings.setValue("settings_preset", m_settings_preset->currentIndex());
	settings.setValue("resolution_preset", m_resolution_preset->currentIndex());
	settings.setValue("tod", m_tod_box->currentIndex());

	settings.setValue("additional_commands", m_additional_commands->text());

	settings.endGroup();
}



void TestRunnerDialog::combo_box_selection_changed(int index)
{
	save_settings();
}
void TestRunnerDialog::line_text_changed(const QString &text)
{
	save_settings();
}


uint32_t TestRunnerDialog::get_fps_test() const
{
	const uint32_t test = m_settings_preset->currentIndex() + 1;
	const uint32_t weather = m_weather_preset->currentIndex() * 10;
	const uint32_t tod = m_tod_box->currentIndex() == 1 ? 200 : 0;

	return test + weather + tod;
}

QString TestRunnerDialog::get_executable() const
{
	return m_installation->get_path() + "/" + m_executables[m_executable->currentIndex()];
}

QStringList TestRunnerDialog::get_arguments(const QString &telemetry_path, bool escape_paths) const
{
	QStringList result;
	result.push_back("--weather_seed=1");
	result.push_back("--time_seed=1");
	result.push_back("--no_prefs");
	result.push_back("--event_trace");
	result.push_back("--fps_test=" + QString::asprintf("%i", get_fps_test()));

	auto make_full_path_happy_for_xplane = [](const QString &path) {
#if WIN
		// X-Plane stupidly will not accept / as a separator after the drive name
		if(path.length() > 3)
		{
			if(path[1] == ':' && path[2] == '/')
			{
				QString result = path;
				result[2] = '\\';

				return result;
			}
		}
#endif

		return path;
	};

	QString replay_path = make_full_path_happy_for_xplane(m_installation->get_replay_path() + "/" + m_replay_files[m_replay_file->currentIndex()]);

	if(escape_paths)
		result.push_back("--load_smo=\"" + replay_path + "\"");
	else
		result.push_back("--load_smo=" + replay_path);

	if(!telemetry_path.isEmpty())
		result.push_back("--telemetry_path=" + make_full_path_happy_for_xplane(telemetry_path) + "");

	switch(m_resolution_preset->currentIndex())
	{
		case 0:
		default:
			break;

		case 1:
			result.push_back("--full=1920x1080");
			break;
		case 2:
			result.push_back("--full=2560x1440");
			break;

		case 3:
			result.push_back("--window=20x40x1940x1120");
			break;
		case 4:
			result.push_back("--window=20x40x2580x1480");
			break;
	}

	if(!m_additional_commands->text().isEmpty())
		result.push_back(m_additional_commands->text());

	return result;
}

QString TestRunnerDialog::get_name() const
{
	QString replay = m_replay_files[m_replay_file->currentIndex()];
	QString name = "FPS Test " % QString::number(get_fps_test()) % " - " % replay % ".tlm";

	return name;
}
