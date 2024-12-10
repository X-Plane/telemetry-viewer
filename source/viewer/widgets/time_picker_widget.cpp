//
// Created by Sidney on 31/07/2020.
//

#include <QLineEdit>
#include <QStringBuilder>
#include "time_picker_widget.h"

time_picker_widget::time_picker_widget(QWidget *parent) :
	QAbstractSpinBox(parent),
	m_start(0),
	m_end(1),
	m_value(0)
{
	format_value();
}

time_picker_widget::~time_picker_widget()
{}

QSize time_picker_widget::sizeHint() const
{
	return lineEdit()->sizeHint();
}

QString time_picker_widget::format_time(int32_t time)
{
	int32_t hour = time / 60 / 60;
	int32_t minute = (time / 60) % 60;
	int32_t seconds = time % 60;

	QString value = hour > 0 ? (QString::asprintf("%02i", hour) % ":") : QString();

	return value % QString::asprintf("%02i", minute) % ":" % QString::asprintf("%02i", seconds);
}

void time_picker_widget::format_value() const
{
	lineEdit()->setText(format_time(m_value));
}

QAbstractSpinBox::StepEnabled time_picker_widget::stepEnabled() const
{
	StepEnabled enabled = StepNone;

	if(m_value > m_start)
		enabled |= StepDownEnabled;
	if(m_value < m_end)
		enabled |= StepUpEnabled;

	return enabled;
}

void time_picker_widget::stepBy(int steps)
{
	m_value += steps;
	m_value = std::min(m_end, std::max(m_start, m_value));

	format_value();
	emit value_changed(m_value);
}


void time_picker_widget::set_range(int32_t start, int32_t end)
{
	m_start = start;
	m_end = end;
}
void time_picker_widget::set_value(int32_t value)
{
	m_value = value;
	format_value();
}
