//
// Created by Sidney on 13-Jun-18.
//

#include <QString>
#include <thread>
#include <telemetry/parser.h>

#include "document_window.h"

#include "test_runner_dialog.h"
#include "model/recently_opened.h"
#include "utilities/color.h"
#include "utilities/data_decimator.h"
#include "utilities/xplane_installations.h"
#include "utilities/settings.h"
#include "utilities/performance_calculator.h"
#include "utilities/providers.h"

static document_window *s_first_document = nullptr;

document_window::document_window() :
	m_next_window(s_first_document)
{
	s_first_document = this;

	setupUi(this);
	setWindowTitle(QString("Telemetry Viewer"));

	m_action_new->setShortcut(QKeySequence::New);
	m_action_new->setStatusTip("Create a new window");
	connect(m_action_new, &QAction::triggered, this, &document_window::new_file);

	m_action_open->setShortcut(QKeySequence::Open);
	m_action_open->setStatusTip("Open a telemetry file");
	connect(m_action_open, &QAction::triggered, this, &document_window::open_file);

	m_action_save->setShortcut(QKeySequence::Save);
	m_action_save->setStatusTip("Save the currently loaded telemetry file");
	connect(m_action_save, &QAction::triggered, this, &document_window::save_file);

	m_action_close->setShortcut(QKeySequence::Close);
	m_action_close->setStatusTip("Close the telemetry file");
	connect(m_action_close, &QAction::triggered, this, &document_window::close);

	m_action_exit->setShortcut(QKeySequence::Quit);
	connect(m_action_exit, &QAction::triggered, qApp, &QApplication::quit);

	connect(m_start_edit, &time_picker_widget::value_changed, this, &document_window::range_changed);
	connect(m_end_edit, &time_picker_widget::value_changed, this, &document_window::range_changed);
	connect(m_event_picker, &QComboBox::currentIndexChanged, this, &document_window::event_range_changed);
	connect(m_mode_selector, &QComboBox::currentIndexChanged, [this](int index) {
		m_chart_view->set_type((chart_type)index);
	});
	connect(m_memory_scaling, &QComboBox::currentIndexChanged, [this](int index) {
		m_chart_view->set_memory_scaling((memory_scaling)index);
	});

	connect(m_providers_view, &QTreeWidget::itemChanged, [this](QTreeWidgetItem *item) {

		if(item->checkState(0) == Qt::Checked)
			m_chart_view->add_data(item->data(0, Qt::UserRole).value<telemetry_field *>(), item->background(0).color());
		else
			m_chart_view->remove_data(item->data(0, Qt::UserRole).value<telemetry_field *>());

	});

	m_mode_selector->setCurrentIndex((int)m_chart_view->get_type());
	m_memory_scaling->setCurrentIndex((int)m_chart_view->get_memory_scaling());

	m_splitter->setStretchFactor(0, 3);
	m_splitter->setStretchFactor(1, 1);

	connect(m_timeline_widget, &timeline_widget::spanFocused, [this](uint64_t id){
		auto model = m_timeline_tree->model();
		auto x = model->match(model->index(0,0, {}), Qt::DisplayRole, QVariant::fromValue(id), 1, Qt::MatchFlag::MatchExactly|Qt::MatchFlag::MatchRecursive);
		if(!x.empty())
		{
			m_timeline_tree->scrollTo(x.front(),QAbstractItemView::ScrollHint::PositionAtTop);
			m_timeline_tree->selectionModel()->select(x.front(), QItemSelectionModel::SelectionFlag::ClearAndSelect|QItemSelectionModel::Rows);
		}
	});

	connect(m_run_tests, &QPushButton::pressed, this, &document_window::run_fps_test);

	m_installations = get_xplane_installations();

	QSettings settings = open_settings();
	const QString selected_install = settings.value("installation", "").toString();

	for(auto &install : m_installations)
	{
		m_installation_selector->addItem(install.path);

		if(selected_install == install.path)
			m_installation_selector->setCurrentIndex(m_installation_selector->count() - 1);
	}

	connect(m_installation_selector, &QComboBox::currentIndexChanged, [this](int index) {

		QSettings settings = open_settings();
		settings.setValue("installation", m_installations[index].path);

	});

	connect(m_menu_recents, &QMenu::aboutToShow, [this]() {

		m_menu_recents->clear();

		for(auto &action : m_recent_file_actions)
			delete action;

		m_recent_file_actions.clear();


		recently_opened opened;

		for(auto &recent : opened.get_entries())
		{
			QAction *open_action = new QAction(recent);
			open_action->setData(recent);

			m_menu_recents->addAction(open_action);

			connect(open_action, &QAction::triggered, [this, open_action] {
				load_file(open_action->data().toString());
			});

			m_recent_file_actions.push_back(open_action);
		}

		if(!m_menu_recents->isEmpty())
			m_menu_recents->addSeparator();

		m_menu_recents->addAction(m_action_clear_recents);

	});

	connect(m_action_clear_recents, &QAction::triggered, this, &document_window::clear_recent_files);

	statusBar()->showMessage("Ready");
}
document_window::document_window(QSettings &state) :
	document_window()
{
	restoreGeometry(state.value("geometry").toByteArray());

	QString path = state.value("file", "").toString();
	QFileInfo file(path);

	if(file.exists())
	{
		load_file(file.filePath());

		int32_t start_value = state.value("start", m_start_edit->get_value()).toInt();
		int32_t end_value = state.value("end", m_end_edit->get_value()).toInt();

		set_time_range(start_value, end_value);
	}
}

document_window::~document_window()
{
	for(auto &action : m_recent_file_actions)
		delete action;

	if(this == s_first_document)
		s_first_document = m_next_window;
	else
	{
		document_window *temp = s_first_document;
		while(temp)
		{
			if(temp->m_next_window == this)
			{
				temp->m_next_window = m_next_window;
				break;
			}
		}
	}
}

QColor document_window::generate_color_for_title(const QString &title) const
{
	return generate_color(title, 0.9f, 0.4f);
}

void document_window::closeEvent(QCloseEvent *event)
{
	delete this;
}

void document_window::restore_state()
{
	QSettings settings = open_settings();

	const int count = settings.beginReadArray("documents");
	if(count == 0)
	{
		document_window *window = new document_window();
		window->show();

		settings.endArray();
		return;
	}

	for(int i = 0; i < count; ++ i)
	{
		settings.setArrayIndex(i);

		document_window *window = new document_window(settings);
		window->show();
	}

	settings.endArray();
}

void document_window::store_state()
{
	QSettings settings = open_settings();

	settings.beginWriteArray("documents");

	int i = 0;
	document_window *temp = s_first_document;

	while(temp)
	{
		settings.setArrayIndex(i ++);

		temp->save_state(settings);
		temp = temp->m_next_window;
	}

	settings.endArray();
}

void document_window::save_state(QSettings &state)
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

void document_window::clear_recent_files()
{
	recently_opened opened;
	opened.clear_entries();
}

void document_window::set_time_range(int32_t start, int32_t end)
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
			auto &field = provider_timing::get_field(m_document.container, field_id);
			performance_calculator perf(field, start, end);

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

telemetry_file read_telemetry_data(const QString &path)
{
	QFile file(path);

	if(!file.open(QIODevice::ReadOnly))
		return {};

	const size_t length = file.bytesAvailable();

	telemetry_file result;
	result.data.resize(length);

	file.read((char *)result.data.data(), length);
	file.close();


	telemetry_parser_options options;
	options.data_point_processor = [](const telemetry_container &container, const telemetry_provider &provider, const telemetry_field &field, const std::vector<telemetry_data_point> &data_points) {

		std::vector<telemetry_data_point> result = decimate_data(data_points, 1000);

		// Expand the first and last point to the very end of the telemetry range
		if(!result.empty())
		{
			auto first = result.front();
			if(first.timestamp > container.get_start_time())
			{
				first.timestamp = container.get_start_time();
				result.insert(result.begin(), first);
			}

			auto last = result.back();
			if(last.timestamp < container.get_end_time())
			{
				last.timestamp = container.get_end_time();
				result.push_back(last);
			}
		}

		return result;

	};

	try
	{
		result.container = parse_telemetry_data(result.data.data(), result.data.size(), options);
		return result;
	}
	catch(std::exception &e)
	{
		qDebug() << "Caught exception while parsing file " << e.what();
		return {};
	}
}

void document_window::load_file(const QString &path)
{
	m_document = read_telemetry_data(path);
	m_chart_view->clear();

	update_telemetry();
	setWindowFilePath(path);

	recently_opened opened;
	opened.add_entry(path);
}

void document_window::new_file()
{
	document_window *window = new document_window();
	window->show();
}

void document_window::open_file()
{
	QString base_path = m_base_dir;

	if(!m_installations.empty() && base_path.isEmpty())
		base_path = m_installations[m_installation_selector->currentIndex()].telemetry_path;

	QString path = QFileDialog::getOpenFileName(this, tr("Open Telemetry file"), base_path, tr("Telemetry File (*.tlm)"));
	QFileInfo info(path);

	if(info.exists())
	{
		touch_telemetry_file(info);
		load_file(path);
	}
}

void document_window::save_file()
{
	if(m_document.data.empty())
		return;

	QString base_path = m_base_dir;

	if(!m_installations.empty() && base_path.isEmpty())
		base_path = m_installations[m_installation_selector->currentIndex()].telemetry_path;

	QString path = QFileDialog::getSaveFileName(this, tr("Save Telemetry file"), base_path, tr("Telemetry File (*.tlm)"));

	if(!path.isEmpty())
	{
		QFile file(path);

		if(file.open(QIODevice::WriteOnly))
		{
			file.write((const char *)m_document.data.data(), m_document.data.size());

			QFileInfo info(file);
			touch_telemetry_file(info);
		}
	}

}

void document_window::touch_telemetry_file(const QFileInfo &file_info)
{
	QString file_path = file_info.filePath();
	QString file_name = file_info.fileName();

	file_path.remove(file_path.length() - file_name.length(), file_name.length());

	m_base_dir = file_path;
}

void document_window::run_fps_test()
{
	xplane_installation *installation = &m_installations[m_installation_selector->currentIndex()];
	test_runner_dialog runner(installation);

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
			{
				load_file(full_result_path);
				statusBar()->showMessage("Ready!");

				return;
			}
		}

		statusBar()->showMessage("Failed to load telemetry file!");
	}
}


void document_window::range_changed()
{
	set_time_range(m_start_edit->get_value(), m_end_edit->get_value());
}
void document_window::event_range_changed(int index)
{
	if(index == -1)
		return;

	set_time_range(m_event_ranges[index].start, m_event_ranges[index].end);

	m_start_edit->set_value(m_event_ranges[index].start);
	m_end_edit->set_value(m_event_ranges[index].end);
}

void document_window::update_telemetry()
{
	m_start_edit->set_range(m_document.container.get_start_time(), m_document.container.get_end_time());
	m_start_edit->set_value(m_document.container.get_start_time());

	m_end_edit->set_range(m_document.container.get_start_time(), m_document.container.get_end_time());
	m_end_edit->set_value(m_document.container.get_end_time());

	{
		// Figure out our event ranges
		m_event_picker->clear();
		m_event_ranges.clear();

		{
			event_range everything;

			everything.start = m_document.container.get_start_time();
			everything.end = m_document.container.get_end_time();
			everything.name = "Everything";

			m_event_ranges.push_back(everything);
			m_event_picker->addItem(everything.name);
		}

		if(m_document.container.has_provider(provider_sim_apup::identifier))
		{
			auto &do_world_events = provider_sim_apup::get_field(m_document.container, provider_sim_apup::do_world);
			auto &aircraft_events = provider_sim_apup::get_field(m_document.container, provider_sim_apup::loaded_aircraft);

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
					range.name = title + QString(" (") + time_picker_widget::format_time(range.start) + " - " + time_picker_widget::format_time(range.end) + QString(")");

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

		if(m_event_ranges.size() > 1)
		{
			m_event_picker->setCurrentIndex(1);
			range_changed();
		}
		else
			set_time_range(m_document.container.get_start_time(), m_document.container.get_end_time());
	}

	// Statistics view
	{
		QList<QTreeWidgetItem *> items;

		for(auto &stat : m_document.container.get_statistics())
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

		m_overview_view->clear();
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
			for(auto &provider: m_document.container.get_providers())
			{
				QTreeWidgetItem *provider_item = new QTreeWidgetItem();
				provider_item->setText(0, QString::fromStdString(provider.get_title()));
				provider_item->setData(0, Qt::UserRole, QVariant::fromValue(&provider));

				top_level_items.push_back(provider_item);

				if(provider.get_identifier() == provider_timing::identifier)
					expanded_items.push_back(provider_item);

				for(auto &entry: provider.get_fields())
				{
					if(entry.empty())
						continue;

					QTreeWidgetItem *field = new QTreeWidgetItem(provider_item);
					field->setCheckState(0, Qt::CheckState::Unchecked);
					field->setText(1, QString::fromStdString(entry.get_title()));
					field->setBackground(0, generate_color_for_title(field->text(1)));
					field->setData(0, Qt::UserRole, QVariant::fromValue(&entry));

					if(provider.get_identifier() == provider_timing::identifier)
					{
						if(entry.get_id() == provider_timing::cpu || entry.get_id() == provider_timing::gpu)
							enabled_items.push_back(field);
					}
				}
			}
		}

		m_providers_view->clear();
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
				item->setText(1, QString::number(ceilf(event.get_duration() * 1000.0f)));
				item->setText(2, path);

				for (auto &child: event.get_children())
					r(item, child, r);

				return item;
			};

			return create_child_span(nullptr, event, create_child_span);
		};

		m_timeline_tree->clear();

		for(auto &event : m_document.container.get_events())
			m_timeline_tree->addTopLevelItem(create_span(event));

		m_timeline_widget->setTimelineSpans(m_document.container.get_events());
	}
}