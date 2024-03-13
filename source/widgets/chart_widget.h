//
// Created by Sidney on 27/07/2020.
//

#ifndef TELEMETRY_STUDIO_CHART_WIDGET_H
#define TELEMETRY_STUDIO_CHART_WIDGET_H

#include <QWebEngineView>

struct telemetry_data_point;
struct telemetry_provider_field;

enum class chart_type
{
	line,
	boxplot
};

class chart_widget : public QWebEngineView
{
Q_OBJECT
public:
	chart_widget(QWidget *parent = nullptr);

	void show_developer_tools() const;

	void clear();

	void add_data(telemetry_provider_field *field);
	void remove_data(telemetry_provider_field *field);

	void set_range(int32_t start, int32_t end);
	void set_type(chart_type type);

public slots:
	void update_data();

private:
	struct chart_data
	{
		chart_data() = default;
		chart_data(chart_data &&) = default;
		chart_data &operator =(chart_data &&) = default;

		telemetry_provider_field *field;
		QString data;
	};

	void enumerate_field(telemetry_provider_field *field, std::function<void (const telemetry_data_point &)> &&callback) const;

	QString build_html(telemetry_provider_field *field) const;

	QString build_line_html(telemetry_provider_field *field) const;
	QString build_boxplot_html(telemetry_provider_field *field) const;

	chart_type m_type;

	int32_t m_start;
	int32_t m_end;

	std::vector<chart_data> m_data;
	bool m_rebuild_chart;
	bool m_is_loaded;
};

#endif //TELEMETRY_STUDIO_CHART_WIDGET_H
