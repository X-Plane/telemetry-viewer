//
// Created by Sidney on 13-Jun-18.
//

#include <QString>
#include <thread>
#include <telemetry/known_providers.h>

#include "DocumentWindow.h"
#include "TestRunnerDialog.h"
#include "Application.h"

#include "utilities/Color.h"
#include "utilities/DataDecimator.h"
#include "utilities/Settings.h"
#include "utilities/PerformanceCalculator.h"

DocumentWindow::DocumentWindow() :
	m_document(nullptr)
{
	setupUi(this);

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

	m_splitter->setStretchFactor(0, 3);
	m_splitter->setStretchFactor(1, 1);

	connect(m_timeline_widget, &TimelineWidget::spanFocused, [this](uint64_t id){
		auto model = m_timeline_tree->model();
		auto x = model->match(model->index(0,0, {}), Qt::DisplayRole, QVariant::fromValue(id), 1, Qt::MatchFlag::MatchExactly|Qt::MatchFlag::MatchRecursive);
		if(!x.empty())
		{
			m_timeline_tree->scrollTo(x.front(),QAbstractItemView::ScrollHint::PositionAtTop);
			m_timeline_tree->selectionModel()->select(x.front(), QItemSelectionModel::SelectionFlag::ClearAndSelect|QItemSelectionModel::Rows);
		}
	});

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
	delete m_document;
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

	if(item->checkState(0) == Qt::Checked)
		m_chart_view->add_data(field, item->background(0).color());
	else
		m_chart_view->remove_data(field);
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
	if(urls.size() == 1)
	{
		const QString path = urls[0].toLocalFile();
		set_document_by_path(path);
	}
	else
	{
		for(int i = 0; i < urls.size(); ++ i)
		{
			const QString path = urls[i].toLocalFile();

			if(i == 0)
				set_document_by_path(path);
			else
				qApp->open_file(path);
		}
	}
}



void DocumentWindow::clear()
{
	setWindowFilePath("");

	m_chart_view->clear();

	m_event_picker->clear();
	m_event_ranges.clear();

	m_providers_view->clear();
	m_overview_view->clear();
	m_timeline_tree->clear();

	delete m_document;
	m_document = nullptr;
}

void DocumentWindow::set_document_by_path(const QString &path)
{
	try
	{
		TelemetryDocument *document = qApp->load_file(path);
		set_document(document);
	}
	catch(...)
	{
		set_document(nullptr);
		statusBar()->showMessage("Failed to load telemetry file!");
	}
}

void DocumentWindow::set_document(TelemetryDocument *document)
{
	clear();

	m_document = document;

	if(!m_document)
		return;

	const QString path = m_document->get_path();
	const QFileInfo info(path);

	m_base_dir = info.absolutePath();

	setWindowFilePath(path);
	statusBar()->showMessage("Loaded " + path);

	const auto &container = m_document->get_data();

	m_start_edit->set_range(container.get_start_time(), container.get_end_time());
	m_start_edit->set_value(container.get_start_time());

	m_end_edit->set_range(container.get_start_time(), container.get_end_time());
	m_end_edit->set_value(container.get_end_time());

	{
		// Figure out our event ranges
		{
			event_range everything;

			everything.start = container.get_start_time();
			everything.end = container.get_end_time();
			everything.name = "Everything";

			m_event_ranges.push_back(everything);
			m_event_picker->addItem(everything.name);
		}

		if(container.has_provider(provider_sim_apup::identifier))
		{
			auto &do_world_events = provider_sim_apup::get_field(container, provider_sim_apup::do_world);
			auto &aircraft_events = provider_sim_apup::get_field(container, provider_sim_apup::loaded_aircraft);

			bool is_doing_world = false;
			double start_timestamp = 0.0;
			double end_timestamp = 0.0;

			auto flush_range = [this, &aircraft_events](double start, double end) {

				// We want at least 12 seconds worth of data to add it to the timeline
				if((end - start) > 12.0)
				{
					QString title;

					try
					{
						title = aircraft_events.get_data_point_after_time(start + 5.0).value.get<const char *>();
					}
					catch(...)
					{
						title = "Event";
					}

					event_range range;
					range.start = start + 8.0;
					range.end = end - 3.0;
					range.name = title + QString(" (") + TimePickerWidget::format_time(range.start) + " - " + TimePickerWidget::format_time(range.end) + QString(")");

					m_event_ranges.push_back(range);
					m_event_picker->addItem(range.name);
				}

			};

			for(auto &data : do_world_events.get_data_points())
			{
				if(data.value.get<bool>() && !is_doing_world)
				{
					is_doing_world = true;
					start_timestamp = data.timestamp;
				}

				if(data.value.get<bool>())
					end_timestamp = data.timestamp;

				if(!data.value.get<bool>() && is_doing_world)
				{
					is_doing_world = false;
					flush_range(start_timestamp, end_timestamp);
				}
			}

			if(is_doing_world)
				flush_range(start_timestamp, end_timestamp);
		}

		m_event_picker->setCurrentIndex(m_event_ranges.size() > 1 ? 1 : 0);
		range_changed();
	}

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
					if(field.empty())
						continue;

					const QString title = QString::fromStdString(field.get_title());

					QTreeWidgetItem *item = new QTreeWidgetItem(provider_item);
					item->setCheckState(0, Qt::CheckState::Unchecked);
					item->setData(0, Qt::UserRole, QVariant::fromValue(&field));
					item->setBackground(0, generate_color_for_title(title));
					item->setText(1, title);

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

	{
		auto create_span = [](const telemetry_event &event) -> QTreeWidgetItem * {
			auto create_child_span = [](QTreeWidgetItem *root, const telemetry_event &event, auto &r) -> QTreeWidgetItem *
			{
				QString path;

				for(auto &entry: event.get_entries())
				{
					if(entry.title == "path")
						path = entry.value.get<const char *>();
				}

				QTreeWidgetItem *item = new QTreeWidgetItem(root);
				item->setText(0, QString::number(event.get_id()));
				item->setText(1, QString::number(std::ceil(event.get_duration() * 1000.0f)));
				item->setText(2, path);

				for (auto &child: event.get_children())
					r(item, child, r);

				return item;
			};

			return create_child_span(nullptr, event, create_child_span);
		};


		for(auto &event : container.get_events())
			m_timeline_tree->addTopLevelItem(create_span(event));

		m_timeline_widget->setTimelineSpans(container.get_events());
	}
}




QColor DocumentWindow::generate_color_for_title(const QString &title) const
{
	return generate_color(title, 0.9f, 0.4f);
}

void DocumentWindow::restore_state(QSettings &state)
{
	restoreGeometry(state.value("geometry").toByteArray());

	QString path = state.value("file", "").toString();
	QFileInfo file(path);

	if(file.exists())
	{
		set_document_by_path(file.filePath());

		int32_t start_value = state.value("start", m_start_edit->get_value()).toInt();
		int32_t end_value = state.value("end", m_end_edit->get_value()).toInt();

		set_time_range(start_value, end_value);
	}
}

void DocumentWindow::save_state(QSettings &state) const
{
	QString path = windowFilePath();

	state.setValue("geometry", saveGeometry());

	if(!path.isEmpty())
	{
		state.setValue("file", path);
		state.setValue("start", m_start_edit->get_value());
		state.setValue("end", m_end_edit->get_value());
	}
}

 void DocumentWindow::set_time_range(int32_t start, int32_t end)
{
	m_chart_view->set_range(start, end);

	QChart *chart = new QChart();

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
			auto &field = provider_timing::get_field(m_document->get_data(), field_id);
			PerformanceCalculator perf(field, start, end);

			QBarSet *set = new QBarSet(QString::fromStdString(field.get_title()));
			set->setColor(generate_color_for_title(set->label()));

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

	QString path = QFileDialog::getOpenFileName(this, tr("Open Telemetry file"), base_path, tr("Telemetry File (*.tlm)"));
	QFileInfo info(path);

	if(info.exists())
	{
		touch_telemetry_file(info);
		set_document_by_path(path);
	}
}

void DocumentWindow::save_file()
{
	if(!m_document || !m_document->has_data())
		return;

	QString base_path = m_base_dir;

	if(!m_installations.empty() && base_path.isEmpty())
		base_path = m_installations[m_installation_selector->currentIndex()].get_telemetry_path();

	QString path = QFileDialog::getSaveFileName(this, tr("Save Telemetry file"), base_path, tr("Telemetry File (*.tlm)"));

	if(!path.isEmpty())
		m_document->save(path);
}

void DocumentWindow::touch_telemetry_file(const QFileInfo &file_info)
{
	QString file_path = file_info.filePath();
	QString file_name = file_info.fileName();

	file_path.remove(file_path.length() - file_name.length(), file_name.length());

	m_base_dir = file_path;
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
				set_document_by_path(full_result_path);
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

	set_time_range(m_event_ranges[index].start, m_event_ranges[index].end);

	m_start_edit->set_value(m_event_ranges[index].start);
	m_end_edit->set_value(m_event_ranges[index].end);
}
