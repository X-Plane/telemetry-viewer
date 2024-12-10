//
// Created by Sidney on 27/07/2020.
//

#ifndef TELEMETRY_STUDIO_CHART_WIDGET_H
#define TELEMETRY_STUDIO_CHART_WIDGET_H

#include <QtCharts/QtCharts>
#include <QChartView>
#include <telemetry/provider.h>

enum class chart_type
{
	line,
	boxplot
};

enum class memory_scaling
{
	bytes,
	kilobytes,
	megabytes,
	gigabytes,
};

class chart_widget : public QChartView
{
Q_OBJECT
public:
	chart_widget(QWidget *parent = nullptr);
	~chart_widget() override;

	void add_data(telemetry_field *field, QColor color);
	void remove_data(telemetry_field *field);

	void show_data(telemetry_field *field);
	void hide_data(telemetry_field *field);

	void clear();

	void set_memory_scaling(memory_scaling scaling);
	memory_scaling get_memory_scaling() const { return m_memory_scaling; }

	void set_type(chart_type type);
	chart_type get_type() const { return m_type; }

	void set_range(int32_t start, int32_t end);

private:
	struct chart_axis
	{
		telemetry_unit unit;
		QValueAxis *line_axis;
		QValueAxis *box_axis;
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

		void detach() const;
		void hide();
		void show();
		void update_box_set(int32_t start, int32_t end, double scale_factor) const;

		telemetry_field *field;
		chart_axis *axis = nullptr;

		bool is_hidden = false;

		QColor color;
		QLineSeries *line_series = nullptr;
		QBoxPlotSeries *box_series = nullptr;
		QBoxSet *box_set = nullptr;

		telemetry_data_point min_value;
		telemetry_data_point max_value;
	};

	void build_chart_axis(telemetry_unit unit);

	double scale_memory(double bytes) const;

	chart_axis *get_chart_axis_for_field(telemetry_field *field) const;
	chart_data &get_data_for_field(telemetry_field *field);

	QLineSeries *create_line_series(telemetry_field *field) const;

	void rescale_axes();

	chart_type m_type;
	memory_scaling m_memory_scaling;

	int32_t m_start;
	int32_t m_end;

	std::vector<chart_data> m_data;

	QChart *m_line_chart;
	QChart *m_boxplot_chart;

	QValueAxis *m_timeline_axis;
	QBarCategoryAxis *m_category_axis;
	QVector<chart_axis *> m_axes;
};

#endif //TELEMETRY_STUDIO_CHART_WIDGET_H
