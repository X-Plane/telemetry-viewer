//
// Created by Sidney on 13-Jun-18.
//

#include <QString>
#include <thread>
#include <telemetry/known_providers.h>

#include "DocumentWindow.h"
#include "TestRunnerDialog.h"
#include "Application.h"
#include "ChartEvent.h"

#include "utilities/Color.h"
#include "utilities/DataDecimator.h"
#include "utilities/Settings.h"
#include "utilities/PerformanceCalculator.h"

static QColor get_color_for_telemetry_field(const telemetry_field *field)
{
	return generate_color(QString::fromStdString(field->get_title()), 0.9f, 0.4f);
}


DocumentWindow::DocumentWindow()
{
	setupUi(this);

	add_toolbar_spacer();

	m_installation_selector = new QComboBox();
	add_toolbar_widget(m_installation_selector, "Installation");


	m_action_exit->setShortcut(QKeySequence::Quit);
	connect(m_action_exit, &QAction::triggered, qApp, &QApplication::quit);

	connect(m_mode_selector, &QComboBox::currentIndexChanged, [this](int index) {
		m_chart_view->set_type((ChartWidget::Type)index);
	});
	connect(m_memory_scaling, &QComboBox::currentIndexChanged, [this](int index) {
		m_chart_view->set_memory_scaling((ChartWidget::MemoryScaling)index);
	});

	m_mode_selector->setCurrentIndex((int)m_chart_view->get_type());
	m_memory_scaling->setCurrentIndex((int)m_chart_view->get_memory_scaling());

	m_splitter->setStretchFactor(0, 1);
	m_splitter->setStretchFactor(1, 3);
	m_splitter->setStretchFactor(2, 1);

	m_splitter_vertical->setStretchFactor(0, 2);
	m_splitter_vertical->setStretchFactor(1, 2);
	m_splitter_vertical->setStretchFactor(2, 1);

	m_installations = qApp->get_installations();

	QSettings settings = open_settings();
	const QString selected_install = settings.value("installation", "").toString();

	for(auto &install : m_installations)
	{
		m_installation_selector->addItem(install.get_path());

		if(selected_install == install.get_path())
			m_installation_selector->setCurrentIndex(m_installation_selector->count() - 1);
	}

	connect(m_installation_selector, &QComboBox::currentIndexChanged, [this](int index) {

		QSettings settings = open_settings();
		settings.setValue("installation", m_installations[index].get_path());

	});

	statusBar()->showMessage("Ready");
}

DocumentWindow::~DocumentWindow()
{
	for(auto &document : m_loaded_documents)
		delete document.document;
}

void DocumentWindow::closeEvent(QCloseEvent *event)
{
	qApp->close_document(this);
}

void DocumentWindow::populate_recent_items()
{
	m_menu_recents->clear();
	m_recent_file_actions = std::move(qApp->get_recently_opened_files());

	for(auto &action : m_recent_file_actions)
	{
		connect(action.get(), &QAction::triggered, [this, action = action.get()] {
			set_document_by_path(action->data().toString());
		});

		m_menu_recents->addAction(action.get());
	}

	if(!m_menu_recents->isEmpty())
		m_menu_recents->addSeparator();

	m_menu_recents->addAction(m_action_clear_recents);
}

void DocumentWindow::clear_recent_items()
{
	qApp->clear_recently_opened_files();
}

void DocumentWindow::provider_item_changed(QTreeWidgetItem *item)
{
	const telemetry_field *field = item->data(0, Qt::UserRole).value<const telemetry_field *>();
	Q_ASSERT(field);

	set_field_enabled(field, item->checkState(0) == Qt::Checked);
}

void DocumentWindow::set_field_enabled(const telemetry_field *field, bool enable)
{
	QVector<QPair<const telemetry_field *, int32_t>> fields;

	for(auto &container : m_loaded_documents)
	{
		const auto &data = container.document->get_data();
		if(!data.has_provider(field->get_provider_id()))
			continue;

		const auto &provider = data.get_provider(field->get_provider_id());
		if(!provider.has_field(field->get_id()))
			continue;

		const telemetry_field &additional_field = provider.get_field(field->get_id());
		fields.push_back(qMakePair(&additional_field, container.start_offset));
	}

	if(enable)
	{
		const QColor primary_color = get_color_for_telemetry_field(field);

		QColor secondary_color = primary_color;
		secondary_color.setAlpha(128);


		m_chart_view->add_data(field, primary_color, 0);

		for(auto &[ additional, offset ] : fields)
			m_chart_view->add_data(additional, secondary_color, offset);

		m_enabled_fields.push_back(field);
	}
	else
	{
		m_chart_view->remove_data(field);

		for(auto &additional : fields)
			m_chart_view->remove_data(additional.first);

		const auto iterator = std::find(m_enabled_fields.begin(), m_enabled_fields.end(), field);
		Q_ASSERT(iterator != m_enabled_fields.end());

		m_enabled_fields.erase(iterator);
	}
}

bool DocumentWindow::can_accept_mime_data(const QMimeData *mime) const
{
	return mime->hasUrls() && !mime->urls().isEmpty();
}

void DocumentWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if(can_accept_mime_data(event->mimeData()))
	{
		event->acceptProposedAction();
		return;
	}

	QMainWindow::dragEnterEvent(event);
}
void DocumentWindow::dragMoveEvent(QDragMoveEvent *event)
{
	if(can_accept_mime_data(event->mimeData()))
	{
		event->acceptProposedAction();
		return;
	}

	QMainWindow::dragMoveEvent(event);
}
void DocumentWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}
void DocumentWindow::dropEvent(QDropEvent *event)
{
	const QMimeData *mime = event->mimeData();

	if(!can_accept_mime_data(mime))
	{
		QMainWindow::dropEvent(event);
		return;
	}

	const QList<QUrl> urls = mime->urls();

	if(qApp->keyboardModifiers() & Qt::ControlModifier)
	{
		for(auto &url : urls)
		{
			try
			{
				TelemetryDocument *document = qApp->load_file(url.toLocalFile());
				add_document(document);
			}
			catch(std::exception &e)
			{
				statusBar()->showMessage("Failed to load telemetry file. Error: " + QString(e.what()));
			}
		}

		return;
	}

	QStringList paths;

	for(auto &url : urls)
		paths.append(url.toLocalFile());

	set_documents_by_paths(paths);
}



void DocumentWindow::clear()
{
	setWindowFilePath("");

	m_chart_view->clear();

	m_event_picker->clear();

	m_providers_view->clear();
	m_overview_view->clear();

	m_documents_tree->clear();

	for(auto &container : m_loaded_documents)
		delete container.document;

	m_loaded_documents.clear();
}

void DocumentWindow::set_document_by_path(const QString &path, const QString &name)
{
	try
	{
		TelemetryDocument *document = qApp->load_file(path);

		if(!name.isEmpty())
			document->set_name(name);

		set_document(document);
	}
	catch(...)
	{
		set_document(nullptr);
		statusBar()->showMessage("Failed to load telemetry file!");
	}
}

void DocumentWindow::set_documents_by_paths(const QStringList &paths)
{
	clear();

	for(auto &path : paths)
	{
		try
		{
			TelemetryDocument *document = qApp->load_file(path);

			if(m_loaded_documents.isEmpty())
				set_document(document);
			else
				add_document(document);
		}
		catch(...)
		{
			statusBar()->showMessage("Failed to load telemetry file!");
		}
	}
}

void DocumentWindow::set_document(TelemetryDocument *document)
{
	clear();

	if(!document)
		return;

	add_document(document);

	const QString path = document->get_path();
	const QFileInfo info(path);

	QString file_path = info.filePath();
	QString file_name = info.fileName();

	file_path.remove(file_path.length() - file_name.length(), file_name.length());

	setWindowFilePath(path);
	statusBar()->showMessage("Loaded " + path);

	const auto &container = document->get_data();

	m_start_edit->set_range(container.get_start_time(), container.get_end_time());
	m_start_edit->set_value(container.get_start_time());

	m_end_edit->set_range(container.get_start_time(), container.get_end_time());
	m_end_edit->set_value(container.get_end_time());

	auto &regions = document->get_regions();
	int selected_region = 0;
	int index = 0;

	for(auto &region : regions)
	{
		QString title = region.name + QString(" (") + TimePickerWidget::format_time(region.start) + " - " + TimePickerWidget::format_time(region.end) + QString(")");
		m_event_picker->addItem(title);

		if(region.type == TelemetryRegion::Type::Flying && selected_region == 0)
			selected_region = index;

		index ++;
	}

	m_event_picker->setCurrentIndex(selected_region);
	event_range_changed(selected_region);

	// Statistics view
	{
		QList<QTreeWidgetItem *> items;

		for(auto &stat : container.get_statistics())
		{
			QTreeWidgetItem *root_item = new QTreeWidgetItem();
			root_item->setText(0, QString::fromStdString(stat.get_title()));

			for(auto &entry : stat.get_entries())
			{
				QTreeWidgetItem *stat_item = new QTreeWidgetItem(root_item);
				stat_item->setText(0, QString::fromStdString(entry.title));

				switch(entry.value.type)
				{
					case telemetry_type::uint8:
					case telemetry_type::uint16:
					case telemetry_type::uint32:
					case telemetry_type::uint64:
						stat_item->setText(1, QString::number(entry.value.get<uint64_t>()));
						break;

					case telemetry_type::int32:
					case telemetry_type::int64:
						stat_item->setText(1, QString::number(entry.value.get<int64_t>()));
						break;

					case telemetry_type::f32:
					case telemetry_type::f64:
						stat_item->setText(1, QString::number(entry.value.get<double>()));
						break;

					case telemetry_type::string:
						stat_item->setText(1, QString::fromStdString(entry.value.string));
						stat_item->setToolTip(1, QString::fromStdString(entry.value.string));
						break;

					default:
						stat_item->setText(1, "Unsupported type");
				}


			}

			items.push_back(root_item);
		}

		m_overview_view->addTopLevelItems(items);

		for(auto &item : items)
			item->setExpanded(true);
	}

	// Add all the telemetry providers
	{
		QList<QTreeWidgetItem *> top_level_items;
		QList<QTreeWidgetItem *> expanded_items;
		QList<QTreeWidgetItem *> enabled_items;

		{
			for(auto &provider: container.get_providers())
			{
				QTreeWidgetItem *provider_item = new QTreeWidgetItem();
				provider_item->setText(0, QString::fromStdString(provider.get_title()));
				provider_item->setData(0, Qt::UserRole, QVariant::fromValue(&provider));

				top_level_items.push_back(provider_item);

				if(provider.get_identifier() == provider_timing::identifier)
					expanded_items.push_back(provider_item);

				for(auto &field: provider.get_fields())
				{
					const telemetry_type type = field.get_type();
					const bool can_chart = !(type == telemetry_type::vec2 || type == telemetry_type::dvec2);

					if(field.empty() || !can_chart)
						continue;

					QTreeWidgetItem *item = new QTreeWidgetItem(provider_item);
					item->setCheckState(0, Qt::CheckState::Unchecked);
					item->setData(0, Qt::UserRole, QVariant::fromValue(&field));
					item->setBackground(0, get_color_for_telemetry_field(&field));
					item->setText(1, QString::fromStdString(field.get_title()));

					switch(field.get_unit())
					{
						case telemetry_unit::value:
							item->setToolTip(1, "Raw value");
							break;
						case telemetry_unit::fps:
							item->setToolTip(1, "FPS");
							break;
						case telemetry_unit::time:
							item->setToolTip(1, "Time");
							break;
						case telemetry_unit::memory:
							item->setToolTip(1, "Memory");
							break;
						case telemetry_unit::duration:
							item->setToolTip(1, "Duration");
							break;
					}

					if(provider.get_identifier() == provider_timing::identifier)
					{
						if(field.get_id() == provider_timing::cpu || field.get_id() == provider_timing::gpu)
							enabled_items.push_back(item);
					}
				}
			}
		}

		m_providers_view->addTopLevelItems(top_level_items);

		for(auto &item : expanded_items)
			item->setExpanded(true);
		for(auto &item : enabled_items)
			item->setCheckState(0, Qt::CheckState::Checked);
	}

	for(auto &event : container.get_events())
		m_chart_view->add_event(event);
}

QAction *DocumentWindow::add_toolbar_widget(QWidget *widget, const QString &text) const
{
	QWidget *container = new QWidget();

	QVBoxLayout *layout = new QVBoxLayout(container);
	layout->setContentsMargins(2, 2, 2, 2);
	layout->setSpacing(2);
	layout->setAlignment(Qt::AlignCenter);

	QLabel *label = new QLabel(text, container);
	label->setAlignment(Qt::AlignCenter);

	layout->addWidget(widget);
	layout->addWidget(label);

	return toolBar->addWidget(container);
}

QAction *DocumentWindow::add_toolbar_spacer() const
{
	QWidget *spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	return toolBar->addWidget(spacer);
}

void DocumentWindow::add_document(TelemetryDocument *document)
{
	loaded_document entry;
	entry.document = document;
	entry.start_offset = 0.0;

	if(!m_loaded_documents.empty())
	{
		TelemetryDocument *root_document = m_loaded_documents.front().document;
		auto &root_regions = root_document->get_regions();
		auto &regions = document->get_regions();

		if(root_regions.size() != regions.size())
			throw std::range_error("Telemetry document has different timing regions");

		size_t count = regions.size();
		for(size_t i = 0; i < count; ++ i)
		{
			if(root_regions[i].type != regions[i].type || root_regions[i].name != regions[i].name)
				throw std::range_error("Telemetry document has different timing regions");

			if(entry.start_offset <= 0.0 && regions[i].type == TelemetryRegion::Type::Flying)
				entry.start_offset = root_regions[i].start - regions[i].start;
		}
	}

	m_loaded_documents.push_back(entry);

	{
		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setText(0, document->get_name());

		m_documents_tree->addTopLevelItem(item);
	}

	for(auto &field : m_enabled_fields)
	{
		const auto &data = entry.document->get_data();
		if(!data.has_provider(field->get_provider_id()))
			continue;

		const auto &provider = data.get_provider(field->get_provider_id());
		if(!provider.has_field(field->get_id()))
			continue;

		const telemetry_field &additional_field = provider.get_field(field->get_id());

		QColor color = get_color_for_telemetry_field(field);
		color.setAlpha(128);

		m_chart_view->add_data(&additional_field, color, entry.start_offset);
	}
}

DocumentWindow::loaded_document &DocumentWindow::get_selected_document()
{
	return m_loaded_documents.front();
}
const DocumentWindow::loaded_document &DocumentWindow::get_selected_document() const
{
	return m_loaded_documents.front();
}

void DocumentWindow::restore_state(QSettings &state)
{
	clear();

	restoreGeometry(state.value("geometry").toByteArray());

	{
		const int count = state.beginReadArray("files");
		QStringList paths;

		for(int i = 0; i < count; ++ i)
		{
			state.setArrayIndex(i);

			QString path = state.value("path").toString();
			QFileInfo file(path);

			if(file.exists())
				paths.append(path);
		}

		state.endArray();

		set_documents_by_paths(paths);
	}


	int32_t start_value = state.value("start", m_start_edit->get_value()).toInt();
	int32_t end_value = state.value("end", m_end_edit->get_value()).toInt();
	int32_t index = state.value("region", m_event_picker->currentIndex()).toInt();

	m_start_edit->set_value(start_value);
	m_end_edit->set_value(end_value);
	m_event_picker->setCurrentIndex(index);
}

void DocumentWindow::save_state(QSettings &state) const
{
	state.setValue("geometry", saveGeometry());

	state.beginWriteArray("files");

	int index = 0;

	for(auto &document : m_loaded_documents)
	{
		TelemetryDocument *doc = document.document;
		if(doc->get_path().isEmpty())
			continue;

		state.setArrayIndex(index ++);
		state.setValue("path", document.document->get_path());
	}

	state.endArray();

	state.setValue("start", m_start_edit->get_value());
	state.setValue("end", m_end_edit->get_value());
	state.setValue("region", m_event_picker->currentIndex());
}

 void DocumentWindow::set_time_range(int32_t start, int32_t end)
{
	m_chart_view->set_range(start, end);

	QChart *chart = new QChart();

	if(!m_loaded_documents.empty())
	{
		struct perf_set
		{
			QString name;
			float percentile;
		};

		const std::array perf_series = {
			perf_set{ "P1", 0.01f },
			perf_set{ "P5", 0.05f },
			perf_set{ "Average", -1.0f },
			perf_set{ "P95", 0.95f },
			perf_set{ "P99", 0.99f }
		};

		auto build_bar_set = [&](provider_timing::field_id field_id) -> QBarSet * {

			try
			{
				auto &field = provider_timing::get_field(m_loaded_documents.at(0).document->get_data(), field_id);
				PerformanceCalculator perf(field, start, end);

				QBarSet *set = new QBarSet(QString::fromStdString(field.get_title()));
				set->setColor(get_color_for_telemetry_field(&field));

				for(auto &perf_set : perf_series)
				{
					if(perf_set.percentile <= 0.0f)
						set->append(perf.calculate_average() * 1000.0);
					else
						set->append(perf.calculate_percentile(perf_set.percentile) * 1000.0);
				}

				return set;
			}
			catch(...)
			{
				return nullptr;
			}
		};

		QList<QBarSet *> bar_sets;

		if(QBarSet *cpu = build_bar_set(provider_timing::cpu))
			bar_sets.append(cpu);
		if(QBarSet *gpu = build_bar_set(provider_timing::gpu))
			bar_sets.append(gpu);


		if(!bar_sets.isEmpty())
		{
			QHorizontalBarSeries *series = new QHorizontalBarSeries();
			series->append(bar_sets);
			series->setLabelsVisible(true);
			series->setLabelsPosition(QAbstractBarSeries::LabelsInsideEnd);
			series->setLabelsPrecision(3);
			series->setLabelsFormat("@valuems");

			chart->addSeries(series);

			auto y_axis = new QBarCategoryAxis();

			for(auto &perf_set : perf_series)
				y_axis->append(perf_set.name);

			chart->addAxis(y_axis, Qt::AlignLeft);
			series->attachAxis(y_axis);

			auto x_axis = new QValueAxis();
			x_axis->setLabelFormat("%ims");

			chart->addAxis(x_axis, Qt::AlignBottom);

			series->attachAxis(x_axis);
			x_axis->applyNiceNumbers();
		}
	}

	m_statistics_view->setChart(chart);
}

void DocumentWindow::new_file()
{
	qApp->new_file();
}

void DocumentWindow::open_file()
{
	QString base_path = m_base_dir;

	if(!m_installations.empty() && base_path.isEmpty())
		base_path = m_installations[m_installation_selector->currentIndex()].get_telemetry_path();

	QStringList paths = QFileDialog::getOpenFileNames(this, tr("Open Telemetry file"), base_path, tr("Telemetry File (*.tlm)"));

	clear();

	for(auto &path : paths)
	{
		QFileInfo info(path);

		if(info.exists())
		{
			try
			{
				TelemetryDocument *document = qApp->load_file(path);

				if(m_loaded_documents.empty())
					set_document(document);
				else
					add_document(document);
			}
			catch(...)
			{
				set_document(nullptr);
				statusBar()->showMessage("Failed to load telemetry file!");
			}
		}
	}
}

void DocumentWindow::save_file()
{
	for(auto &document : m_loaded_documents)
	{
		if(!document.document->has_data())
			continue;

		QString base_path = m_base_dir;

		if(!m_installations.empty() && base_path.isEmpty())
			base_path = m_installations[m_installation_selector->currentIndex()].get_telemetry_path();

		base_path = base_path % '/' % document.document->get_name();

		QString path = QFileDialog::getSaveFileName(this, tr("Save Telemetry file"), base_path, tr("Telemetry File (*.tlm)"));

		if(!path.isEmpty())
			document.document->save(path);
	}
}

void DocumentWindow::run_fps_test()
{
	XplaneInstallation *installation = &m_installations[m_installation_selector->currentIndex()];
	TestRunnerDialog runner(installation);

	if(runner.exec())
	{
		statusBar()->showMessage("Waiting for the FPS test to finish");

		QString result_path = QDir::tempPath() + "/xplane_telemetry";
		QString full_result_path = result_path + ".tlm"; // X-Plane is overly helpful by putting the .tlm extension in for us

		// Remove any leftover old telemetry file
		{
			QFile file;
			file.remove(full_result_path);
		}

		QProcess process;
		process.start(runner.get_executable(), runner.get_arguments(result_path, false));

		if(!process.waitForStarted())
		{
			statusBar()->showMessage("Failed to start FPS test!");
			return;
		}

		while(!process.waitForFinished())
			std::this_thread::sleep_for(std::chrono::seconds(1));

		if(process.exitStatus() == QProcess::NormalExit)
		{
			QFileInfo info(full_result_path);

			if(info.isFile())
				set_document_by_path(full_result_path, runner.get_name());
		}
	}
}


void DocumentWindow::range_changed()
{
	set_time_range(m_start_edit->get_value(), m_end_edit->get_value());
}
void DocumentWindow::event_range_changed(int index)
{
	if(index == -1)
		return;

	auto &regions = get_selected_document().document->get_regions();

	set_time_range(regions[index].start, regions[index].end);

	m_start_edit->set_value(regions[index].start);
	m_end_edit->set_value(regions[index].end);
}
