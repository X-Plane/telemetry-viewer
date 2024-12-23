//
// Created by Sidney on 31/07/2020.
//

#include <QLineEdit>
#include <QStringBuilder>
#include "TimePickerWidget.h"

TimePickerWidget::TimePickerWidget(QWidget *parent) :
	QAbstractSpinBox(parent),
	m_start(0),
	m_end(1),
	m_value(0)
{
	format_value();

	connect(this, &QAbstractSpinBox::editingFinished, this, &TimePickerWidget::interpret_text);
}

QSize TimePickerWidget::sizeHint() const
{
	return lineEdit()->sizeHint();
}

QString TimePickerWidget::format_time(int32_t time)
{
	int32_t hour = time / 60 / 60;
	int32_t minute = (time / 60) % 60;
	int32_t seconds = time % 60;

	QString value = hour > 0 ? (QString::asprintf("%02i", hour) % ":") : QString();

	return value % QString::asprintf("%02i", minute) % ":" % QString::asprintf("%02i", seconds);
}

void TimePickerWidget::format_value() const
{
	lineEdit()->setText(format_time(m_value));
}

void TimePickerWidget::interpret_text()
{
	int32_t value = 0;
	int32_t shifts = 0;

	const QByteArray data = lineEdit()->text().toUtf8();
	const char *string = data.constData();

	// This "Parses" the input string. If a number is encountered, assume it is seconds and just add it to the accumulated value
	// When encountering anything else, shift the whole number. After 3 shifts (seconds -> minutes -> hours), just give up and ignore whatever happens next
	// as a result, the following are all valid:
	// 3a2 => 3:02 => 182 seconds
	// 3:02 => 0:02 => 182 seconds
	// 1i3:2 => 1:03:02 => 3782 seconds
	// 1i3:2x1  => 1:03:02 => 3782 seconds
	//
	// The values do not have to fall into the half open range [0, 60), since they are tracked in seconds
	// so a value of 162:0 would be interpreted as 162 minutes

	while(*string)
	{
		if(std::isdigit(*string))
		{
			value += std::atoi(string);

			do {
				string ++;
			} while(std::isdigit(*string));
		}
		else
		{
			if(++ shifts >= 3)
				break; // Shifted past the hour mark

			value *= 60;

			do {
				string ++;
			} while(*string && !std::isdigit(*string));
		}
	}

	set_value(value);
}


QAbstractSpinBox::StepEnabled TimePickerWidget::stepEnabled() const
{
	StepEnabled enabled = StepNone;

	if(m_value > m_start)
		enabled |= StepDownEnabled;
	if(m_value < m_end)
		enabled |= StepUpEnabled;

	return enabled;
}

void TimePickerWidget::stepBy(int steps)
{
	m_value += steps;
	m_value = std::min(m_end, std::max(m_start, m_value));

	format_value();
	emit value_changed(m_value);
}


void TimePickerWidget::set_range(int32_t start, int32_t end)
{
	m_start = start;
	m_end = end;

	const int32_t value = std::min(m_end, std::max(m_start, m_value));
	if(value != m_value)
	{
		m_value = value;
		format_value();

		emit value_changed(m_value);
	}
}
void TimePickerWidget::set_value(int32_t value)
{
	value = std::min(m_end, std::max(m_start, value));

	if(value != m_value)
	{
		m_value = value;
		format_value();

		emit value_changed(m_value);
	}
}
