//
// Created by Sidney on 11/03/2024.
//

#include <QFileInfo>

#include "recently_opened.h"
#include "utilities/settings.h"

recently_opened::recently_opened() :
	m_is_dirty(false)
{
	QSettings settings = open_settings();

	const int count = settings.beginReadArray("recently_opened");

	for(int i = 0; i < count; ++ i)
	{
		settings.setArrayIndex(i);

		QString path = settings.value("path").toString();
		QFileInfo file(path);

		if(file.exists())
			m_entries.push_back(file.canonicalPath() + "/" + file.fileName());
	}

	settings.endArray();
}
recently_opened::~recently_opened()
{
	if(m_is_dirty)
		save();
}


void recently_opened::add_entry(const QString &path)
{
	QFileInfo file(path);

	if(!file.exists())
		return;

	QString real_path = file.canonicalPath() + "/" + file.fileName();

	for(int i = 0; i < m_entries.size(); ++ i)
	{
		auto entry = m_entries[i];

		if(entry == real_path)
		{
			if(i != 0)
			{
				m_entries.erase(m_entries.begin() + i);
				m_entries.push_front(entry);

				m_is_dirty = true;
			}

			return;
		}
	}

	if(m_entries.size() >= MAX_RECENTLY_OPENED)
		m_entries.pop_back();

	m_entries.push_front(real_path);
	m_is_dirty = true;
}

void recently_opened::clear_entries()
{
	m_entries.clear();
	m_is_dirty = true;
}

void recently_opened::save()
{
	QSettings settings = open_settings();

	settings.beginWriteArray("recently_opened");

	for(int i = 0; i < m_entries.size(); ++ i)
	{
		settings.setArrayIndex(i);
		settings.setValue("path", m_entries[i]);
	}

	settings.endArray();

	m_is_dirty = false;
}
