#ifndef TELEMETRY_STUDIO_TIMELINE_WIDGET_H
#define TELEMETRY_STUDIO_TIMELINE_WIDGET_H

#include <QGraphicsView>
#include <QGraphicsItemGroup>
#include <QVector>
#include <QKeyEvent>

#include "../model/telemetry_container.h"

class timeline_span_item : public QObject, public QGraphicsItem
{
Q_OBJECT
public:
	timeline_span_item(const QPalette& p, const telemetry_event_span& span, QGraphicsItem* parent);

	void collapse();
	void expand();

	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

signals:
	void reflowed();

protected:
	void reflow();

	telemetry_event_span m_span;
	std::vector<timeline_span_item*> m_span_groups;
};

class timeline_widget : public QGraphicsView
{
	Q_OBJECT

	QGraphicsScene scene;

public:
	timeline_widget(QWidget *parent = nullptr);

	void setTimelineSpans(const QVector<telemetry_event_span>& spans);

	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;

	void wheelEvent(QWheelEvent*) override;

	void drawBackground(QPainter*, const QRectF&) override;

signals:
	void spanFocused(uint64_t id);

public slots:
	void changeZoom(float scale);
	void reflowTimeline();

private:
	void viewportChange();

	float m_time_scale = 100.f;
	QVector<timeline_span_item*> m_span_groups;
};


#endif //TELEMETRY_STUDIO_TIMELINE_WIDGET_H
