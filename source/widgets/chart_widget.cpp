//
// Created by Sidney on 27/07/2020.
//

#include <sstream>
#include <QStringBuilder>
#include "chart_widget.h"
#include "../model/telemetry_container.h"

static inline QString get_widget_axis(telemetry_unit unit)
{
	switch(unit)
	{
		case telemetry_unit::value:
			return "valueAxis";
		case telemetry_unit::memory:
			return "memoryAxis";
		case telemetry_unit::fps:
			return "fpsAxis";
		case telemetry_unit::time:
			return "timeAxis";
		case telemetry_unit::duration:
			return "timeAxis";
	}

	return "valueAxis";
}


chart_widget::chart_widget(QWidget *parent) :
	QWebEngineView(parent),
	m_start(0),
	m_end(std::numeric_limits<int32_t>::max())
{
	load(QUrl("qrc:/resources/chart.html"));

#if 0
	QWebEngineView *dev_tools = new QWebEngineView();
	dev_tools->page()->setInspectedPage(page());
	dev_tools->show();
#endif
}
chart_widget::~chart_widget()
{}


void chart_widget::set_range(int32_t start, int32_t end)
{
	m_start = start;
	m_end = end;

	for(auto &entry : m_data)
	{
		entry.data = build_html(entry.field);
	}

	update_data();
}

void chart_widget::clear()
{
	m_data.clear();
	page()->runJavaScript("clear_data();");
}


void chart_widget::update_data()
{
	QString js = "set_data([ ";

	for(auto &entry : m_data)
	{
		js = js % entry.data;
	}

	js = js % "]);";

	page()->runJavaScript(js);
}

void chart_widget::add_data(telemetry_provider_field *field)
{
	chart_data data;
	data.field = field;
	data.data = build_html(field);

	m_data.push_back(std::move(data));
	update_data();
}
void chart_widget::remove_data(telemetry_provider_field *field)
{
	auto iterator = std::find_if(m_data.begin(), m_data.end(), [&](const chart_data &data) {
		return (field == data.field);
	});

	m_data.erase(iterator);

	if(m_data.empty())
		page()->runJavaScript("clear_data();");
	else
		update_data();
}


QString chart_widget::build_html(telemetry_provider_field *field) const
{
	QString result = QString("{\n")
					 % "type: 'line',\n"
					 % "borderWidth: 4,\n"
					 % "showLine: true,\n"
					// % "pointRadius: 0,\n"
					 % "label: '" % field->title % "',\n"
					 % "fill: false,\n"
					 % "borderColor: '" % field->color % "',\n"
					 % "yAxisID: '" % get_widget_axis(field->unit) % "',\n"
					 % "data: [\n";

	for(auto &data : field->data_points)
	{
		if(data.first < m_start)
			continue;
		if(data.first > m_end) // Data is in order!
			break;

		QString field_data;

		switch(field->unit)
		{
			case telemetry_unit::duration:
			{
				QPointF point = data.second.toPointF();
				field_data = QString::number(point.y() - point.x());

				break;
			}

			default:
				field_data = QString::number(data.second.toFloat());

				qInfo() << data.second.toFloat();
				break;
		}

		result = result % "{x:" % QString::number(data.first) % ",y:" % field_data % "},\n";
	}

	result = result % "]\n";
	result = result % "},\n";

	return result;
}