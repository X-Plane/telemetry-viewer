#include <QApplication>
#include "widgets/document_window.h"

QString apply_dark_theme()
{
	// https://gist.github.com/QuantumCD/6245215
	QPalette palette;
	palette.setColor(QPalette::Window, QColor(53,53,53));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(25,25,25));
	palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, Qt::white);
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(53,53,53));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::red);
	palette.setColor(QPalette::Link, QColor(42, 130, 218));

	palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
	palette.setColor(QPalette::HighlightedText, Qt::black);

	QApplication::setPalette(palette);
	QApplication::setStyle(QStyleFactory::create("Fusion"));

	return "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }";
}

int main(int argc, char *argv[])
{
	QCoreApplication::setApplicationName("Telemetry Viewer");
	QCoreApplication::setOrganizationName("Laminar Research");
	QCoreApplication::setApplicationVersion(QString::number(VERSION_MAJOR) % "." % QString::number(VERSION_MINOR) % "." % QString::number(VERSION_PATCH));

#if !LIN
	QString style_sheet = apply_dark_theme();

	QApplication app(argc, argv);
	app.setStyleSheet(style_sheet);
#else
	QApplication::setStyle(QStyleFactory::create("Fusion"));
	QApplication app(argc, argv);
#endif

	document_window::restore_state();

	const int result = QApplication::exec();

	document_window::store_state();

	return result;
}