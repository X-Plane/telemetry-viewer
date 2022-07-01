//
// Created by Sidney on 27/07/2020.
//

#ifndef TELEMETRY_STUDIO_CHART_WIDGET_H
#define TELEMETRY_STUDIO_CHART_WIDGET_H

#include <QWebEngineView>

struct telemetry_provider_field;

class chart_widget : public QWebEngineView
{
public:
	chart_widget(QWidget *parent = nullptr);
	~chart_widget();

	void clear();

	void add_data(telemetry_provider_field *field);
	void remove_data(telemetry_provider_field *field);

	void set_range(int32_t start, int32_t end);

private:
	struct chart_data
	{
		chart_data() = default;
		chart_data(chart_data &&) = default;
		chart_data &operator =(chart_data &&) = default;

		telemetry_provider_field *field;
		QString data;
	};

	QString build_html(telemetry_provider_field *field) const;
	void update_data();

	int32_t m_start;
	int32_t m_end;

	std::vector<chart_data> m_data;
};

#endif //TELEMETRY_STUDIO_CHART_WIDGET_H
