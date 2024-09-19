//
// Created by Sidney on 27/07/2020.
//

#ifndef TELEMETRY_STUDIO_CHART_WIDGET_H
#define TELEMETRY_STUDIO_CHART_WIDGET_H

#include <QtCharts/QtCharts>
#include <QChartView>
#include "model/telemetry_container.h"

enum class telemetry_unit : uint8_t;
struct telemetry_provider_field;

enum class chart_type
{
	line,
	boxplot
};

class chart_widget : public QChartView
{
Q_OBJECT
public:
	chart_widget(QWidget *parent = nullptr);
	~chart_widget() override;

	void clear();

	void add_data(telemetry_provider_field *field);
	void remove_data(telemetry_provider_field *field);

	void set_range(int32_t start, int32_t end);
	void set_type(chart_type type);

private:
	struct chart_axis
	{
		telemetry_unit unit;
		QAbstractAxis *axis;
		bool range_locked;
		bool visible;
		Qt::Alignment alignment;

		double minimum;
		double maximum;
	};

	struct chart_data
	{
		chart_data() = default;
		chart_data(chart_data &&) = default;
		chart_data &operator =(chart_data &&) = default;

		telemetry_provider_field *field;
		bool is_hidden = false;

		QLineSeries *line_series = nullptr;
		QBoxSet *box_set = nullptr;
		chart_axis *axis = nullptr;

		telemetry_data_point min_value;
		telemetry_data_point max_value;
	};

	void build_chart_axis(telemetry_unit unit);

	chart_axis *get_chart_axis_for_field(telemetry_provider_field *field) const;
	chart_data &get_data_for_field(telemetry_provider_field *field);

	QLineSeries *build_line_series(telemetry_provider_field *field) const;

	void update_ranges();

	chart_type m_type;

	int32_t m_start;
	int32_t m_end;

	std::vector<chart_data> m_data;

	QValueAxis *m_x_axis;
	QVector<chart_axis *> m_axes;
};

#endif //TELEMETRY_STUDIO_CHART_WIDGET_H
