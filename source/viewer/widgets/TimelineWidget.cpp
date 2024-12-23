//
// Created by devans on 05/08/23.
//

#include "TimelineWidget.h"

#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QScrollBar>
#include <cmath>

constexpr double k_seconds_to_units = 1000.0;
constexpr double seconds_to_ms(double sec) { return sec * k_seconds_to_units; }

TimelineSpanItem::TimelineSpanItem(const QPalette& p, const telemetry_event& span, QGraphicsItem* parent)
	: QGraphicsItem(parent)
	, m_span(span)
{
	QString path;
	QBrush color = p.window();
	for (auto& f : span.get_entries())
	{
		if (f.title == "path")
		{
			path = f.value.get<const char *>();
		}
		if (f.title == "io_result")
		{
			switch (f.value.get<uint32_t>()) {
				case 0: color = Qt::green; break;
				case 1: color = Qt::red; break;
			}
		}
	}

	auto child_count = span.get_children().size();

	auto rect = new QGraphicsRectItem(0.f, 0.f, seconds_to_ms(span.get_duration()), 20.f, this);
	rect->setX(seconds_to_ms(span.get_start()));
	rect->setBrush(color);
	rect->setPen(Qt::NoPen);
	rect->setFlag(QGraphicsItem::ItemIsSelectable);
	rect->setEnabled(true);
	rect->setCursor(Qt::PointingHandCursor);
	rect->setData(0, QVariant::fromValue(span.get_id()));

	auto label_item = new QGraphicsSimpleTextItem(path, rect);
	label_item->setBrush(p.text());
	label_item->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	auto f = label_item->font();
	if (child_count > 0)
		f.setBold(true);
	label_item->setFont(f);

	auto label = QString("(%1 ms) %2 %3").arg(span.get_duration() * 1000).arg(path).arg(child_count);
	this->setToolTip(label);
	rect->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

	for (auto& child : span.get_children())
	{
		m_span_groups.push_back(new TimelineSpanItem(p, child, this));
		connect(m_span_groups.back(), &TimelineSpanItem::reflowed, [this]() { reflow(); });
	}

	std::sort(m_span_groups.begin(), m_span_groups.end(), [](TimelineSpanItem* a, TimelineSpanItem* b) {
		return a->m_span.get_start() < b->m_span.get_start();
	});

	collapse();
}

QRectF TimelineSpanItem::boundingRect() const
{
	return {seconds_to_ms(m_span.get_start()), 0.0, seconds_to_ms(m_span.get_duration()), 20.0 };
}

void TimelineSpanItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

}

void TimelineSpanItem::collapse()
{
	for (auto& g : m_span_groups)
	{
		g->setVisible(false);
	}
	reflow();
}

void TimelineSpanItem::expand()
{
	for (auto& g : m_span_groups)
	{
		g->setVisible(true);
	}
	reflow();
}

void TimelineSpanItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
	QMenu menu;
	menu.addAction(QString("%1").arg(m_span.get_id()));
	if (!m_span_groups.empty())
	{
		if (m_span_groups.front()->isVisible())
			menu.addAction("Collapse", [this](){ collapse(); });
		else
			menu.addAction("Expand", [this](){ expand(); });
	}
	menu.exec(event->screenPos());
}

void TimelineSpanItem::reflow()
{
	qreal h = 20.f;
	for (auto& g : m_span_groups)
	{
		if (g->isVisible())
		{
			g->setY(h);
			h += (g->boundingRect() | g->childrenBoundingRect()).height();
		}
		else
		{
			g->setY(0);
		}
	}
	emit reflowed();
}

TimelineWidget::TimelineWidget(QWidget *parent)
		: QGraphicsView(parent)
{
	setTransformationAnchor(AnchorUnderMouse);

	connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &TimelineWidget::viewportChange);
	connect(horizontalScrollBar(), &QScrollBar::rangeChanged, this, &TimelineWidget::viewportChange);
	connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TimelineWidget::viewportChange);
	connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, &TimelineWidget::viewportChange);

	connect(&scene, &QGraphicsScene::selectionChanged, [this](){
		auto items = this->scene.selectedItems();
		if (!items.empty())
			emit spanFocused(items.front()->data(0).toULongLong());
	});
}

void TimelineWidget::setTimelineSpans(const std::vector<telemetry_event> &spans)
{
	double min_time = std::numeric_limits<float>::max();
	double max_time = std::numeric_limits<float>::lowest();
	for (auto& s : spans)
	{
		min_time = std::min(min_time, s.get_duration());
		max_time = std::max(max_time, s.get_duration());
		m_span_groups.append(new TimelineSpanItem(scene.palette(), s, nullptr));
		connect(m_span_groups.back(), &TimelineSpanItem::reflowed, this, &TimelineWidget::reflowTimeline);
		scene.addItem(m_span_groups.back());
	}

	auto timelineBounds = scene.itemsBoundingRect();

	setScene(&scene);
	show();

	reflowTimeline();

	//fitInView(timelineBounds, Qt::AspectRatioMode::KeepAspectRatio);
	changeZoom(1.0);
	ensureVisible(timelineBounds);
}

void TimelineWidget::reflowTimeline()
{
	qreal h = 0;
	for (auto& g : m_span_groups)
	{
		g->setY(h);
		h += (g->boundingRect() | g->childrenBoundingRect()).height();
	}
	setSceneRect(scene.itemsBoundingRect());
}

void TimelineWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Shift:
			setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
			break;
		default:
			QGraphicsView::keyPressEvent(event);
			break;
	}
}

void TimelineWidget::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Shift:
			setDragMode(QGraphicsView::DragMode::NoDrag);
			break;
		default:
			QGraphicsView::keyPressEvent(event);
			break;
	}
}

void TimelineWidget::wheelEvent(QWheelEvent *e)
{
	if ((e->modifiers() & Qt::KeyboardModifier::ShiftModifier) != 0)
	{
		changeZoom(std::clamp(m_time_scale + e->angleDelta().y() * 0.1f, 1.0f, 500.0f));
		e->accept();
	}
	else
	{
		QGraphicsView::wheelEvent(e);
	}
}

void TimelineWidget::drawBackground(QPainter *painter, const QRectF &view)
{
	auto view_size = width();
	auto scene_view_left = mapToScene(0, 0).x();
	auto scene_view_right = mapToScene(view_size, 0).x();
	double interval = 500 * 2.0;
	double timeline_min = std::floor(scene_view_left / interval) * interval;
	double timeline_max = std::ceil(scene_view_right / interval) * interval;

	int i_steps = std::max(1, int((timeline_max - timeline_min)  / interval));

	for (int i = 0; i <= i_steps; ++i)
	{
		qreal l = timeline_min + interval * i;
		qreal m = l + interval * 0.5;
		qreal r = l + interval;

		painter->setBrush(palette().light());
		painter->drawRect(QRectF{l, view.top()-1, m-l, view.height()+1});
		painter->setBrush(palette().dark());
		painter->drawRect(QRectF{m, view.top()-1, r-m, view.height()+1});
	}
}

void TimelineWidget::changeZoom(float scale)
{
	m_time_scale = scale;
	qreal effective_scale = std::pow(qreal(2), (scale - 250) / qreal(50));
	QTransform matrix;
	matrix.scale(effective_scale, 1.0);
	setTransform(matrix);
}

void TimelineWidget::viewportChange()
{
}