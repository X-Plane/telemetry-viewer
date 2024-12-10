//
// Created by Sidney on 29/11/2023.
//

#include <QDir>
#include <qclipboard.h>

#include "test_runner_dialog.h"
#include "utilities/settings.h"

test_runner_dialog::test_runner_dialog(xplane_installation *installation) :
	m_installation(installation)
{
	setupUi(this);
	setWindowTitle(QString("Run FPS Test"));

	for(auto &executable : m_installation->executables)
	{
		m_executable->addItem(executable);
	}

	QDir replay_dir(m_installation->replay_path);

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

	m_resolution_preset->addItem("Use current resolution");
	m_resolution_preset->addItem("Fullscreen 1080p");
	m_resolution_preset->addItem("Fullscreen 1440p");
	m_resolution_preset->addItem("Window 1080p");
	m_resolution_preset->addItem("Window 1440p");

	load_settings();

	connect(m_executable, qOverload<int>(&QComboBox::currentIndexChanged), this, &test_runner_dialog::combo_box_selection_changed);
	connect(m_replay_file, qOverload<int>(&QComboBox::currentIndexChanged), this, &test_runner_dialog::combo_box_selection_changed);
	connect(m_weather_preset, qOverload<int>(&QComboBox::currentIndexChanged), this, &test_runner_dialog::combo_box_selection_changed);
	connect(m_settings_preset, qOverload<int>(&QComboBox::currentIndexChanged), this, &test_runner_dialog::combo_box_selection_changed);
	connect(m_resolution_preset, qOverload<int>(&QComboBox::currentIndexChanged), this, &test_runner_dialog::combo_box_selection_changed);

	connect(m_additional_commands, &QLineEdit::textChanged, this, &test_runner_dialog::line_text_changed);

	connect(m_copy_to_clipboard, &QAbstractButton::pressed, [this] {

		QStringList arguments = get_arguments("", true);

		QClipboard *clipboard = QGuiApplication::clipboard();
		clipboard->setText(arguments.join(' '));
	});
}

void test_runner_dialog::load_settings()
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

		for(int i = 0; i < m_installation->executables.size(); ++ i)
		{
			if(m_installation->executables[i] == executable)
			{
				m_executable->setCurrentIndex(i);
				break;
			}
		}
	}
	{
		const QString replay = settings.value("replays", "").toString();

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

	m_additional_commands->setText(settings.value("additional_commands", "").toString());

	settings.endGroup();
}

void test_runner_dialog::save_settings()
{
	QSettings settings = open_settings();

	settings.beginGroup("test_runner");

	settings.setValue("executable", m_installation->executables[m_executable->currentIndex()]);
	settings.setValue("replay", m_replay_files[m_replay_file->currentIndex()]);

	settings.setValue("weather_preset", m_weather_preset->currentIndex());
	settings.setValue("settings_preset", m_settings_preset->currentIndex());
	settings.setValue("resolution_preset", m_resolution_preset->currentIndex());

	settings.setValue("additional_commands", m_additional_commands->text());

	settings.endGroup();
}



void test_runner_dialog::combo_box_selection_changed(int index)
{
	save_settings();
}
void test_runner_dialog::line_text_changed(const QString &text)
{
	save_settings();
}


uint32_t test_runner_dialog::get_fps_test() const
{
	const uint32_t test = m_settings_preset->currentIndex() + 1;
	const uint32_t weather = m_weather_preset->currentIndex() * 10;

	return test + weather;
}

QString test_runner_dialog::get_executable() const
{
	return m_installation->path + "/" + m_installation->executables[m_executable->currentIndex()];
}

QStringList test_runner_dialog::get_arguments(const QString &telemetry_path, bool escape_paths) const
{
	QStringList result;
	result.push_back("--weather_seed=1");
	result.push_back("--time_seed=1");
	result.push_back("--no_prefs");
	result.push_back("--event_trace");
	result.push_back("--fps_test=" + QString::asprintf("%i", get_fps_test()));

	if(escape_paths)
		result.push_back("--load_smo=\"" + xplaneify_path(m_installation->replay_path + "/" + m_replay_files[m_replay_file->currentIndex()]) + "\"");
	else
		result.push_back("--load_smo=" + xplaneify_path(m_installation->replay_path + "/" + m_replay_files[m_replay_file->currentIndex()]));

	if(!telemetry_path.isEmpty())
		result.push_back("--telemetry_path=" + xplaneify_path(telemetry_path) + "");

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
