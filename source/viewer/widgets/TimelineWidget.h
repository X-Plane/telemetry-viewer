#ifndef TELEMETRY_STUDIO_TIMELINE_WIDGET_H
#define TELEMETRY_STUDIO_TIMELINE_WIDGET_H

#include <QGraphicsView>
#include <QGraphicsItemGroup>
#include <QVector>
#include <QKeyEvent>
#include <telemetry/event.h>

class TimelineSpanItem : public QObject, public QGraphicsItem
{
Q_OBJECT
public:
	TimelineSpanItem(const QPalette& p, const telemetry_event& span, QGraphicsItem* parent);

	void collapse();
	void expand();

	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

Q_SIGNALS:
	void reflowed();

protected:
	void reflow();

	telemetry_event m_span;
	std::vector<TimelineSpanItem*> m_span_groups;
};

class TimelineWidget : public QGraphicsView
{
	Q_OBJECT

	QGraphicsScene scene;

public:
	TimelineWidget(QWidget *parent = nullptr);

	void setTimelineSpans(const std::vector<telemetry_event>& spans);

	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;

	void wheelEvent(QWheelEvent*) override;

	void drawBackground(QPainter*, const QRectF&) override;

Q_SIGNALS:
	void spanFocused(uint64_t id);

public Q_SLOTS:
	void changeZoom(float scale);
	void reflowTimeline();

private:
	void viewportChange();

	float m_time_scale = 100.f;
	QVector<TimelineSpanItem*> m_span_groups;
};


#endif //TELEMETRY_STUDIO_TIMELINE_WIDGET_H
