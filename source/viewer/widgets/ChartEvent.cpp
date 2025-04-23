//
// Created by Sidney on 22/04/2025.
//

#include <QCursor>
#include <QPainter>
#include <QApplication>
#include <telemetry/event.h>
#include "ChartEvent.h"

ChartEvent::ChartEvent(QChart *chart, const telemetry_event &event, uint32_t row) :
	QGraphicsItem(chart),
	m_chart(chart),
	m_event(event),
	m_row(row)
{
	m_color = QApplication::palette().color(QPalette::Window);

	for(auto &f : event.get_entries())
	{
		if(f.title == "path")
		{
			m_title = f.value.get<const char *>();
			setToolTip(m_title);
		}
		if(f.title == "io_result")
		{
			switch (f.value.get<uint32_t>()) {
				case 0: m_color = Qt::green; break;
				case 1: m_color = Qt::red; break;
			}
		}
	}

	setZValue(2);

	connect(m_chart, &QChart::plotAreaChanged, this, &ChartEvent::update_geometry);
}

void ChartEvent::set_row(uint32_t row)
{
	m_row = row;
	update_geometry();
}

void ChartEvent::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();
	painter->setClipPath(m_clip_path, Qt::IntersectClip);

	QPainterPath path;
	path.addRect(m_rect);

	QPen pen;
	pen.setColor(QApplication::palette().color(QPalette::Midlight));
	pen.setStyle(Qt::DotLine);

	painter->setBrush(m_color);
	painter->setPen(pen);

	painter->drawPath(path);

	painter->setPen(QApplication::palette().color(QPalette::Text));
	painter->drawText(m_rect, m_title);

	painter->restore();
}

void ChartEvent::update_geometry()
{
	m_rect = compute_bounding_rect();

	m_clip_path = QPainterPath();
	m_clip_path.addRect(m_chart->plotArea());

	prepareGeometryChange();
}

QRectF ChartEvent::compute_bounding_rect() const
{
	QPointF start_point(m_event.get_start(), 0);
	QPointF end_point(m_event.get_end(), 0);

	QPointF scene_start_point = m_chart->mapToPosition(start_point);
	QPointF scene_end_point = m_chart->mapToPosition(end_point);

	const qreal top = m_chart->plotArea().top();
	const qreal height = 20.0f;

	QRectF rect;
	rect.setLeft(scene_start_point.x());
	rect.setRight(scene_end_point.x());
	rect.setTop(top + m_row * height);
	rect.setBottom(rect.top() + 20.0f);

	return rect;
}
