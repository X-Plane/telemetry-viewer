//
// Created by Sidney on 27/07/2020.
//

#ifndef TELEMETRY_STUDIO_CHART_WIDGET_H
#define TELEMETRY_STUDIO_CHART_WIDGET_H

#include <QtCharts/QtCharts>
#include <QChartView>
#include <telemetry/provider.h>

class ChartCallout;

class ChartWidget : public QChartView
{
Q_OBJECT
public:
	enum class Type : uint8_t
	{
		Line,
		LineRunningAverage,
		Boxplot
	};

	enum class MemoryScaling : uint8_t
	{
		Bytes,
		Kilobytes,
		Megabytes,
		Gigabytes,
	};

	ChartWidget(QWidget *parent = nullptr);
	~ChartWidget() override;

	void add_data(const telemetry_field *field, QColor color);
	void remove_data(const telemetry_field *field);

	void show_data(const telemetry_field *field);
	void hide_data(const telemetry_field *field);

	void clear();

	void set_memory_scaling(MemoryScaling scaling);
	MemoryScaling get_memory_scaling() const { return m_memory_scaling; }

	double scale_memory(double bytes) const;

	void set_type(Type type);
	Type get_type() const { return m_type; }

	void set_range(int32_t start, int32_t end);

protected:
	void mouseMoveEvent(QMouseEvent *event) override;
	void leaveEvent(QEvent *event) override;

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

		const telemetry_field *field;
		chart_axis *axis = nullptr;

		bool is_hidden = false;

		QColor color;
		QLineSeries *line_series = nullptr;
		QBoxPlotSeries *box_series = nullptr;
		QBoxSet *box_set = nullptr;

		telemetry_data_point min_value;
		telemetry_data_point max_value;
	};

	void update_tooltip(const QPointF &point) const;
	void update_crosshair(const QPointF &point, const QRectF &plot_area) const;

	void build_chart_axis(telemetry_unit unit);

	chart_axis *get_chart_axis_for_field(const telemetry_field *field) const;
	chart_data &get_data_for_field(const telemetry_field *field);

	QLineSeries *create_line_series(const telemetry_field *field) const;
	void fill_line_series(QLineSeries *series, const telemetry_field *field) const;

	void rescale_axes();

	Type m_type;
	MemoryScaling m_memory_scaling;

	int32_t m_start;
	int32_t m_end;

	std::vector<chart_data> m_data;

	QChart *m_line_chart;
	QChart *m_boxplot_chart;

	QValueAxis *m_timeline_axis;
	QBarCategoryAxis *m_category_axis;
	QVector<chart_axis *> m_axes;

	ChartCallout *m_tooltip;
	QGraphicsLineItem *m_crosshairX;
	QGraphicsLineItem *m_crosshairY;
};

#endif //TELEMETRY_STUDIO_CHART_WIDGET_H
