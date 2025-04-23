//
// Created by Sidney on 22/04/2025.
//

#ifndef CHARTEVENT_H
#define CHARTEVENT_H

#include <QChart>

class telemetry_event;

class ChartEvent : public QObject, public QGraphicsItem
{
public:
	ChartEvent(QChart *chart, const telemetry_event &event, uint32_t row);

	bool intersects_event(const telemetry_event &event) const { return !(m_event.get_end() < event.get_start() || event.get_end() < m_event.get_start()); }

	void set_row(uint32_t row);

	QRectF boundingRect() const override { return m_rect; }
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public Q_SLOTS:
	void update_geometry();

private:
	QRectF compute_bounding_rect() const;

	QChart *m_chart;
	const telemetry_event &m_event;
	uint32_t m_row;

	QString m_title;
	QBrush m_color;

	QPainterPath m_clip_path;
	QRectF m_rect;
};

#endif //CHARTEVENT_H
