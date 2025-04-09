//
// Created by Sidney on 16/10/24.
//

#ifndef CHART_CALLOUT_H
#define CHART_CALLOUT_H

#include <QChart>
#include <telemetry/data.h>

class telemetry_field;
class ChartWidget;

class ChartCallout : public QGraphicsItem
{
public:
	ChartCallout(QChart *parent);

	void set_anchor(QPointF point);
	void set_data_points(const ChartWidget *widget, const QVector<QPair<const telemetry_field *, telemetry_data_point>> &data_points);

	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
	QChart *m_chart = nullptr;
	QPointF m_anchor;

	QFont m_font;
	QString m_text;
	QRectF m_rect;
	QRectF m_text_rect;
};

#endif //CHART_CALLOUT_H