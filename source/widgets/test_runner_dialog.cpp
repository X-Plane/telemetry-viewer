//
// Created by Sidney on 29/11/2023.
//

#include <QDir>
#include <QTextStream>

#include "test_runner_dialog.h"

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

QStringList test_runner_dialog::get_arguments(const QString &telemetry_path) const
{
	QStringList result;
	result.push_back("--weather_seed=1");
	result.push_back("--time_seed=1");
	result.push_back("--fps_test=" + QString::asprintf("%i", get_fps_test()));
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
			result.push_back("--window=1920x1080");
			break;
		case 4:
			result.push_back("--window=2560x1440");
			break;
	}

	if(!m_additional_commands->text().isEmpty())
		result.push_back(m_additional_commands->text());

	return result;
}
