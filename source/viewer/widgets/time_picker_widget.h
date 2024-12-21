//
// Created by Sidney on 31/07/2020.
//

#ifndef TELEMETRY_STUDIO_TIME_PICKER_WIDGET_H
#define TELEMETRY_STUDIO_TIME_PICKER_WIDGET_H

#include <QAbstractSpinBox>

class time_picker_widget : public QAbstractSpinBox
{
Q_OBJECT

public:
	time_picker_widget(QWidget *parent = nullptr);

	QSize sizeHint() const override;

	void stepBy(int steps) override;
	StepEnabled stepEnabled() const override;

	void set_range(int32_t start, int32_t end);
	void set_value(int32_t value);

	int32_t get_value() const { return m_value; }

	static QString format_time(int32_t time);

Q_SIGNALS:
	void value_changed(int32_t value);

private:
	void format_value() const;
	void interpret_text();

	int32_t m_start;
	int32_t m_end;
	int32_t m_value;
};

#endif //TELEMETRY_STUDIO_TIME_PICKER_WIDGET_H
