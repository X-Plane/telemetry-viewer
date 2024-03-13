//
// Created by Sidney on 27/07/2020.
//

#include <QStringBuilder>
#include "chart_widget.h"
#include "../model/telemetry_container.h"

static QString get_widget_axis(telemetry_unit unit)
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
		case telemetry_unit::duration:
			return "timeAxis";
	}

	return "valueAxis";
}


chart_widget::chart_widget(QWidget *parent) :
	QWebEngineView(parent),
	m_type(chart_type::line),
	m_start(0),
	m_end(std::numeric_limits<int32_t>::max()),
	m_rebuild_chart(true),
	m_is_loaded(false)
{
	load(QUrl("qrc:/resources/chart.html"));

	QWebEnginePage *current_page = page();

	connect(current_page, &QWebEnginePage::loadFinished, [this] {
		m_is_loaded = true;
		update_data();
	});
}

void chart_widget::show_developer_tools() const
{
	QWebEngineView *dev_tools = new QWebEngineView();

	dev_tools->page()->setInspectedPage(page());
	dev_tools->show();
}


void chart_widget::update_data()
{
	if(!m_is_loaded)
		return;

	QString js;

	if(m_rebuild_chart)
	{
		QString type;
		QString additional_options = "{}";

		switch(m_type)
		{
			case chart_type::line:
				type = "line";
				additional_options = "line_chart_options";
				break;

			case chart_type::boxplot:
				type = "violin";
				break;
		}

		js = js % "instantiate_chart('" % type % "', " % additional_options % ");\n";
		m_rebuild_chart = false;
	}

	if(!m_data.empty())
	{
		js = js % "set_data([ ";

		for(auto &entry : m_data)
			js = js % entry.data;

		js = js % "], [ \"\" ]);\n";
	}
	else
		js = js % "clear_data();\n";

	page()->runJavaScript(js);
}

void chart_widget::set_range(int32_t start, int32_t end)
{
	m_start = start;
	m_end = end;

	for(auto &entry : m_data)
		entry.data = build_html(entry.field);

	update_data();
}

void chart_widget::set_type(chart_type type)
{
	if(m_type == type)
		return;

	m_type = type;
	update_data();
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
	update_data();
}

void chart_widget::clear()
{
	m_data.clear();
	update_data();
}


QString chart_widget::build_html(telemetry_provider_field *field) const
{
	switch(m_type)
	{
		case chart_type::line:
			return build_line_html(field);
		case chart_type::boxplot:
			return build_boxplot_html(field);
	}
}

QString chart_widget::build_line_html(telemetry_provider_field *field) const
{
	QString result = QString("{\n")
					 % "borderWidth: 2,\n"
					 % "showLine: true,\n"
					 % "type: 'line',\n"
					 % "label: '" % field->title % "',\n"
					 % "fill: false,\n"
					 % "borderColor: '" % field->color % "',\n"
					 % "yAxisID: '" % get_widget_axis(field->unit) % "',\n"
					 % "data: [\n";

	enumerate_field(field, [&](const telemetry_data_point &data) {
		QString field_data;

		switch(field->unit)
		{
			case telemetry_unit::duration:
			{
				QPointF point = data.value.toPointF();
				field_data = QString::number(point.y() - point.x());

				break;
			}

			default:
				field_data = QString::number(data.value.toFloat());
				break;
		}

		result = result % "{x:" % QString::number(data.timestamp) % ",y:" % field_data % "},\n";
	});

	result = result % "]\n";
	result = result % "},\n";

	return result;
}

QString chart_widget::build_boxplot_html(telemetry_provider_field *field) const
{
	QString result = QString("{\n")
					 % "type: 'violin',\n"
					 % "borderWidth: 2,\n"
					 % "label: '" % field->title % "',\n"
					 % "borderColor: '" % field->color % "',\n"
					 % "data: [ [\n";

	enumerate_field(field, [&](const telemetry_data_point &data) {

		QString field_data = "[ ";

		switch(field->unit)
		{
			case telemetry_unit::duration:
			{
				QPointF point = data.value.toPointF();
				field_data = QString::number(point.y() - point.x());

				break;
			}

			default:
				field_data = QString::number(data.value.toFloat());
				break;
		}

		result = result % field_data %  ", ";
	});

	result = result % "] ]\n";
	result = result % "},\n";

	return result;
}

void chart_widget::enumerate_field(telemetry_provider_field *field, std::function<void (const telemetry_data_point &)> &&callback) const
{
	for(auto &data : field->data_points)
	{
		if(data.timestamp < m_start)
			continue;
		if(data.timestamp > m_end) // Data is in order!
			break;

		callback(data);
	}
}