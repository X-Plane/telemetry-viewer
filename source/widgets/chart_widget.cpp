//
// Created by Sidney on 27/07/2020.
//

#include <QStringBuilder>
#include "chart_widget.h"
#include "model/telemetry_container.h"
#include "utilities/color.h"

chart_widget::chart_widget(QWidget *parent) :
	QChartView(parent),
	m_type(chart_type::line),
	m_memory_scaling(memory_scaling::megabytes),
	m_start(0),
	m_end(std::numeric_limits<int32_t>::max())
{
	m_x_axis = new QValueAxis();
	m_x_axis->setTitleText("Timeline");
	m_x_axis->setTickType(QValueAxis::TicksDynamic);


	build_chart_axis(telemetry_unit::time);
	build_chart_axis(telemetry_unit::value);
	build_chart_axis(telemetry_unit::fps);
	build_chart_axis(telemetry_unit::memory);

	QChart *chart = new QChart();
	setChart(chart);

	chart->addAxis(m_x_axis, Qt::AlignBottom);

	for(auto &axis : m_axes)
		chart->addAxis(axis->axis, axis->alignment);
}
chart_widget::~chart_widget()
{
	for(auto &axis : m_axes)
		delete axis;
}




void chart_widget::set_range(int32_t start, int32_t end)
{
	if(m_start == start && m_end == end)
		return;

	m_start = start;
	m_end = end;

	const uint32_t interval = abs(end - start);

	if(interval >= 5 * 60)
		m_x_axis->setTickInterval(60);
	else if(interval >= 50)
		m_x_axis->setTickInterval(10);
	else
		m_x_axis->setTickInterval(1);

	m_x_axis->setRange(start, end);

	for(auto &data : m_data)
	{
		auto [ min_value, max_value ] = data.field->get_min_max_data_point_in_range(m_start, m_end);

		data.min_value = min_value;
		data.max_value = max_value;
	}

	rescale_axes();
}

void chart_widget::set_type(chart_type type)
{
	if(m_type == type)
		return;

	m_type = type;
}

void chart_widget::set_memory_scaling(memory_scaling scaling)
{
	if(m_memory_scaling == scaling)
		return;

	m_memory_scaling = scaling;

	bool changed_anything = false;

	for(auto &data : m_data)
	{
		if(data.axis->unit != telemetry_unit::memory)
			continue;

		if(data.line_series)
		{
			for(auto &axis : data.line_series->attachedAxes())
				data.line_series->detachAxis(axis);

			chart()->removeSeries(data.line_series);
			data.line_series = nullptr;

			build_line_series(data);

			changed_anything = true;
		}
	}

	if(changed_anything)
		rescale_axes();
}



void chart_widget::add_data(telemetry_provider_field *field)
{
	chart_data &data = get_data_for_field(field);

	if(!data.line_series)
	{
		data.axis = get_chart_axis_for_field(field);
		build_line_series(data);

		auto [ min_value, max_value ] = data.field->get_min_max_data_point_in_range(m_start, m_end);

		data.min_value = min_value;
		data.max_value = max_value;
	}

	if(data.is_hidden)
	{
		data.line_series->show();
		data.is_hidden = false;
	}

	rescale_axes();
}
void chart_widget::remove_data(telemetry_provider_field *field)
{
	chart_data &data = get_data_for_field(field);

	if(data.line_series)
	{
		if(!data.is_hidden)
		{
			data.line_series->hide();
			data.is_hidden = true;
		}

		rescale_axes();
	}
}

void chart_widget::clear()
{
	for(auto &data : m_data)
	{
		for(auto &axis : data.line_series->attachedAxes())
			data.line_series->detachAxis(axis);
	}

	chart()->removeAllSeries();
	m_data.clear();
}



void chart_widget::build_chart_axis(telemetry_unit unit)
{
	chart_axis *axis = new chart_axis();
	axis->unit = unit;
	axis->range_locked = false;
	axis->visible = false;
	axis->alignment = Qt::AlignLeft;

	switch(unit)
	{
		case telemetry_unit::value:
			axis->axis = new QValueAxis();
			axis->axis->setTitleText("Value");
			break;
		case telemetry_unit::memory:
			axis->axis = new QValueAxis();
			axis->axis->setTitleText("Memory");
			axis->alignment = Qt::AlignRight;
			break;
		case telemetry_unit::fps:
			axis->axis = new QValueAxis();
			axis->axis->setTitleText("FPS");
			axis->alignment = Qt::AlignRight;
			break;
		case telemetry_unit::time:
			axis->axis = new QValueAxis();
			axis->axis->setTitleText("Time");
			break;

		default:
			axis->axis = nullptr;
	}

	m_axes.append(axis);
}

chart_widget::chart_axis *chart_widget::get_chart_axis_for_field(telemetry_provider_field *field) const
{
	telemetry_unit unit = field->unit;
	if(unit == telemetry_unit::duration)
		unit = telemetry_unit::time;

	chart_axis *fallback = nullptr;

	for(auto &axis : m_axes)
	{
		if(axis->unit == unit)
			return axis;

		if(axis->unit == telemetry_unit::value)
			fallback = axis;
	}

	return fallback;
}

chart_widget::chart_data &chart_widget::get_data_for_field(telemetry_provider_field *field)
{
	auto iterator = std::find_if(m_data.begin(), m_data.end(), [&](const chart_data &data) {
		return (field == data.field);
	});

	if(iterator != m_data.end())
		return *iterator;

	chart_data &data = m_data.emplace_back();
	data.field = field;

	return data;
}


double chart_widget::scale_memory(double bytes) const
{
	switch(m_memory_scaling)
	{
		case memory_scaling::bytes:
			return bytes;
		case memory_scaling::kilobytes:
			return bytes / 1024.0;
		case memory_scaling::megabytes:
			return bytes / 1024.0 / 1024.0;
		case memory_scaling::gigabytes:
			return bytes / 1024.0 / 1024.0 / 1024.0;

	}

	return bytes;
}



void chart_widget::rescale_axes()
{
	for(auto &axis : m_axes)
	{
		axis->minimum = std::numeric_limits<double>::max();
		axis->maximum = 0.0;

		axis->visible = false;
	}

	for(auto &data : m_data)
	{
		if(!data.line_series || data.is_hidden)
			continue;

		qDebug() << data.field->title << " min/max: " << data.min_value.value.toDouble() << ", " <<  data.max_value.value.toDouble();

		data.axis->visible = true;

		data.axis->minimum = std::min(data.axis->minimum, data.min_value.value.toDouble());
		data.axis->maximum = std::max(data.axis->maximum, data.max_value.value.toDouble());
	}

	auto round_up_to_nearest = [](double value, double N) {
		return std::ceil(value / N) * N;
	};

	auto round_down_to_nearest = [](double value, double N) {
		return std::floor(value / N) * N;
	};

	for(auto &axis : m_axes)
	{
		if(axis->axis->isVisible() != axis->visible)
			axis->axis->setVisible(axis->visible);

		if(!axis->visible)
			continue;

		double min_value = 0.0;
		double max_value = 0.0;

		switch(axis->unit)
		{
			case telemetry_unit::value:
				min_value = round_down_to_nearest(axis->minimum, 1.0);
				max_value = round_up_to_nearest(axis->maximum + 1.0, 1.0);
				break;
			case telemetry_unit::fps:
				min_value = axis->minimum;
				max_value = round_up_to_nearest(axis->maximum, 5.0);
				break;
			case telemetry_unit::time:
				min_value = axis->minimum;
				max_value = std::min(round_up_to_nearest(axis->maximum, 0.01), 0.1);
				break;
			case telemetry_unit::memory:
				min_value = scale_memory(axis->minimum);
				max_value = scale_memory(axis->maximum);
				break;

			default:
				min_value = axis->minimum;
				max_value = axis->maximum;
				break;
		}

		min_value = std::min(round_down_to_nearest(min_value, 1.0), 0.0);

		qDebug() << axis->axis->titleText() << "->set_range(" << min_value << ", " <<  max_value << ")";

		axis->axis->setRange(min_value, max_value);
	}
}

void chart_widget::build_line_series(chart_data &data) const
{
	const auto &field = data.field;

	QLineSeries *series = new QLineSeries();
	series->setColor(field->color);
	series->setName(field->title);

	QPen pen = series->pen();
	pen.setWidth(2);
	series->setPen(pen);

	for(auto &data : field->data_points)
	{
		qreal field_data;

		switch(field->unit)
		{
			case telemetry_unit::duration:
			{
				QPointF point = data.value.toPointF();
				field_data = point.y() - point.x();

				break;
			}

			case telemetry_unit::memory:
			{
				field_data = scale_memory(data.value.toDouble());
				break;
			}

			default:
				field_data = data.value.toFloat();
				break;
		}

		series->append(data.timestamp, field_data);
	}

	chart()->addSeries(series);

	series->attachAxis(m_x_axis);
	series->attachAxis(data.axis->axis);

	data.line_series = series;

	if(data.is_hidden)
		series->hide();
}
