#include <QApplication>
#include <QStyleFactory>
#include "widgets/document_window.h"

void apply_dark_theme(QApplication &app)
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

	app.setPalette(palette);
	app.setStyle(QStyleFactory::create("Fusion"));
	app.setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QCoreApplication::setApplicationName("Telemetry Viewer");
	QCoreApplication::setOrganizationName("Laminar Research");
	QCoreApplication::setApplicationVersion("0.4");

	QApplication app(argc, argv);
	apply_dark_theme(app);

	document_window::restore_state();

	const int result = QApplication::exec();

	document_window::store_state();

	return result;
}