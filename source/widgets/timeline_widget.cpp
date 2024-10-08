//
// Created by devans on 05/08/23.
//

#include "timeline_widget.h"

#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QScrollBar>
#include <cmath>

constexpr double k_seconds_to_units = 1000.0;
constexpr double seconds_to_ms(double sec) { return sec * k_seconds_to_units; }

timeline_span_item::timeline_span_item(const QPalette& p, const telemetry_event_span& span, QGraphicsItem* parent)
	: QGraphicsItem(parent)
	, m_span(span)
{
	QString path;
	QBrush color = p.window();
	for (auto& f : span.fields)
	{
		if (f.first == "path")
		{
			path = f.second.toString();
		}
		if (f.first == "io_result")
		{
			switch (f.second.toUInt()) {
				case 0: color = Qt::green; break;
				case 1: color = Qt::red; break;
			}
		}
	}

	auto rect = new QGraphicsRectItem(0.f, 0.f, seconds_to_ms(span.end - span.begin), 20.f, this);
	rect->setX(seconds_to_ms(span.begin));
	rect->setBrush(color);
	rect->setPen(Qt::NoPen);
	rect->setFlag(QGraphicsItem::ItemIsSelectable);
	rect->setEnabled(true);
	rect->setCursor(Qt::PointingHandCursor);
	rect->setData(0, (quint64 )span.id);

	auto label_item = new QGraphicsSimpleTextItem(path, rect);
	label_item->setBrush(p.text());
	label_item->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	auto f = label_item->font();
	if (!span.child_spans.empty())
		f.setBold(true);
	label_item->setFont(f);
	auto child_count = span.child_spans.size();
	auto label = QString("(%1 ms) %2 %3").arg((span.end - span.begin) * 1000).arg(path).arg(child_count);
	this->setToolTip(label);
	rect->setFlag(QGraphicsItem::ItemClipsChildrenToShape);

	for (auto& child : span.child_spans)
	{
		m_span_groups.push_back(new timeline_span_item(p, child, this));
		connect(m_span_groups.back(), &timeline_span_item::reflowed, [this]() { reflow(); });
	}

	std::sort(m_span_groups.begin(), m_span_groups.end(), [](timeline_span_item* a, timeline_span_item* b) {
		return a->m_span.begin < b->m_span.begin;
	});

	collapse();
}

QRectF timeline_span_item::boundingRect() const
{
	return {seconds_to_ms(m_span.begin), 0.0, seconds_to_ms(m_span.end - m_span.begin), 20.0 };
}

void timeline_span_item::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

}

void timeline_span_item::collapse()
{
	for (auto& g : m_span_groups)
	{
		g->setVisible(false);
	}
	reflow();
}

void timeline_span_item::expand()
{
	for (auto& g : m_span_groups)
	{
		g->setVisible(true);
	}
	reflow();
}

void timeline_span_item::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
	QMenu menu;
	menu.addAction(QString("%1").arg(m_span.id));
	if (!m_span_groups.empty())
	{
		if (m_span_groups.front()->isVisible())
			menu.addAction("Collapse", [this](){ collapse(); });
		else
			menu.addAction("Expand", [this](){ expand(); });
	}
	menu.exec(event->screenPos());
}

void timeline_span_item::reflow()
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

timeline_widget::timeline_widget(QWidget *parent)
		: QGraphicsView(parent)
{
	setTransformationAnchor(AnchorUnderMouse);

	connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &timeline_widget::viewportChange);
	connect(horizontalScrollBar(), &QScrollBar::rangeChanged, this, &timeline_widget::viewportChange);
	connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &timeline_widget::viewportChange);
	connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, &timeline_widget::viewportChange);

	connect(&scene, &QGraphicsScene::selectionChanged, [this](){
		auto items = this->scene.selectedItems();
		if (!items.empty())
			emit spanFocused(items.front()->data(0).toULongLong());
	});
}

void timeline_widget::setTimelineSpans(const QVector<telemetry_event_span> &spans)
{
	double min_time = std::numeric_limits<float>::max();
	double max_time = std::numeric_limits<float>::lowest();
	for (auto& s : spans)
	{
		min_time = std::min(min_time, s.begin);
		max_time = std::max(max_time, s.end);
		m_span_groups.append(new timeline_span_item(scene.palette(), s, nullptr));
		connect(m_span_groups.back(), &timeline_span_item::reflowed, this, &timeline_widget::reflowTimeline);
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

void timeline_widget::reflowTimeline()
{
	qreal h = 0;
	for (auto& g : m_span_groups)
	{
		g->setY(h);
		h += (g->boundingRect() | g->childrenBoundingRect()).height();
	}
	setSceneRect(scene.itemsBoundingRect());
}

void timeline_widget::keyPressEvent(QKeyEvent *event)
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

void timeline_widget::keyReleaseEvent(QKeyEvent *event)
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

void timeline_widget::wheelEvent(QWheelEvent *e)
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

void timeline_widget::drawBackground(QPainter *painter, const QRectF &view)
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

void timeline_widget::changeZoom(float scale)
{
	m_time_scale = scale;
	qreal effective_scale = std::pow(qreal(2), (scale - 250) / qreal(50));
	QTransform matrix;
	matrix.scale(effective_scale, 1.0);
	setTransform(matrix);
}

void timeline_widget::viewportChange()
{
}