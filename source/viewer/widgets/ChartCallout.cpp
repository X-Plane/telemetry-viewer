//
// Created by Sidney on 16/10/24.
//

#include <cinttypes>
#include <QApplication>
#include <QPainter>
#include <QFontMetrics>
#include <QStringBuilder>
#include <telemetry/provider.h>
#include "ChartCallout.h"
#include "ChartWidget.h"

ChartCallout::ChartCallout(QChart *chart) :
	QGraphicsItem(chart),
	m_chart(chart)
{
	setZValue(5);
}

QRectF ChartCallout::boundingRect() const
{
	QPointF anchor = mapFromParent(m_chart->mapToPosition(m_anchor));

	QRectF rect;
	rect.setLeft(qMin(m_rect.left(), anchor.x()));
	rect.setRight(qMax(m_rect.right(), anchor.x()));
	rect.setTop(qMin(m_rect.top(), anchor.y()));
	rect.setBottom(qMax(m_rect.bottom(), anchor.y()));

	return rect;
}

void ChartCallout::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	QPainterPath path;
	path.addRoundedRect(m_rect, 5, 5);

	painter->setBrush(QApplication::palette().color(QPalette::Base));
	painter->drawPath(path);

	painter->drawText(m_text_rect, m_text);
}

void ChartCallout::set_anchor(QPointF point)
{
	m_anchor = point;

	setPos(point + QPoint(10, -20));
	prepareGeometryChange();
}

void ChartCallout::set_data_points(const ChartWidget *widget, double time, const QVector<QPair<const telemetry_field *, telemetry_data_point>> &data_points)
{
	const uint32_t minutes = time / 60.0;
	const uint32_t seconds = uint32_t(time) % 60;
	const uint32_t milliseconds = (time - uint32_t(time)) * 1000.0;

	QString text = QString::asprintf("Timestamp: %02d:%02d:%03d\n", (int)minutes, (int)seconds, (int)milliseconds);

	auto format_time = [](double value) {

		if(value < 0.001)
			return QString::asprintf("%0.0fus", value * 1000000.0);

		if(value < 1.0)
			return QString::asprintf("%0.2fms", value * 1000.0);

		return QString::asprintf("%0.2fs", value);
	};

	for(auto &[ field, point ] : data_points)
	{
		text = text % field->get_title().c_str() % ": ";

		switch(field->get_unit())
		{
			case telemetry_unit::value:
			{
				switch(field->get_type())
				{
					case telemetry_type::boolean:
					{
						if(point.value.b)
							text = text % "true";
						else
							text = text % "false";

						break;
					}

					case telemetry_type::string:
						text = text % point.value.string.c_str();
						break;

					case telemetry_type::uint8:
					case telemetry_type::uint16:
					case telemetry_type::uint32:
					case telemetry_type::uint64:
						text = text % QString::asprintf("%" PRIu64, point.value.get<uint64_t>());
						break;

					case telemetry_type::int32:
					case telemetry_type::int64:
						text = text % QString::asprintf("%" PRIi64, point.value.get<int64_t>());
						break;

					default:
					{
						if(point.value.can_convert_to<double>())
							text = text % QString::asprintf("%0.3f", point.value.get<double>());
						else
							text = text % "NAN";
					}
				}

				break;
			}
			case telemetry_unit::time:
			{
				if(point.value.can_convert_to<double>())
					text = text % format_time(point.value.get<double>());
				else
					text = text % "NAN";

				break;
			}
			case telemetry_unit::duration:
			{
				const double value = point.value.vec2[1] - point.value.vec2[0];
				text = text % format_time(value);
				break;
			}
			case telemetry_unit::fps:
			{
				if(point.value.can_convert_to<double>())
					text = text % QString::asprintf("%0.2fFPS", point.value.get<double>());
				else
					text = text % "NAN";
				break;
			}
			case telemetry_unit::memory:
			{
				if(point.value.can_convert_to<double>())
				{
					const double value = widget->scale_memory(point.value.get<double>());

					switch(widget->get_memory_scaling())
					{
						using enum ChartWidget::MemoryScaling;

						case Bytes:
							text = text % QString::asprintf("%.0fb", value);
							break;
						case Kilobytes:
							text = text % QString::asprintf("%.2fKb", value);
							break;
						case Megabytes:
							text = text % QString::asprintf("%.2fMb", value);
							break;
						case Gigabytes:
							text = text % QString::asprintf("%.2fGb", value);
							break;
					}
				}
				else
					text = text % "NAN";

				break;
			}
		}

		text = text % "\n";
	}

	text = text.left(text.length() - 1);

	QFontMetrics metrics(m_font);
	m_text_rect = metrics.boundingRect(QRect(0, 0, 150, 150), Qt::AlignLeft, text);
	m_text_rect.translate(5, 5);

	m_rect = m_text_rect.adjusted(-5, -5, 5, 5);
	m_text = text;

	prepareGeometryChange();
}
