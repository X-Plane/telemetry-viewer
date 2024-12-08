//
// Created by Sidney on 13-Jun-18.
//

#include <QApplication>
#include <QFileDialog>
#include <QProcess>
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <thread>

#include "document_window.h"

#include "test_runner_dialog.h"
#include "model/recently_opened.h"
#include "model/telemetry_reader.h"
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
	connect(m_event_picker, qOverload<int>(&QComboBox::currentIndexChanged), this, &document_window::event_range_changed);
	connect(m_mode_selector, qOverload<int>(&QComboBox::currentIndexChanged), this, &document_window::mode_changed);
	connect(m_memory_scaling, qOverload<int>(&QComboBox::currentIndexChanged), this, &document_window::memory_scale_changed);

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

	connect(m_installation_selector, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {

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
			auto &field = provider_timing::get_field(m_telemetry, field_id);
			performance_calculator perf(field, start, end);

			QBarSet *set = new QBarSet(field.title);
			set->setColor(field.color);

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

void document_window::load_file(const QString &path)
{
	m_telemetry = read_telemetry_data(path);

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
	if(m_telemetry.raw_data.isEmpty())
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
			file.write(m_telemetry.raw_data);

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


void document_window::range_changed(int32_t value)
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

void document_window::mode_changed(int index)
{
	m_chart_view->set_type((chart_type)index);
}

void document_window::memory_scale_changed(int index)
{
	m_chart_view->set_memory_scaling((memory_scaling)index);
}

bool document_window::tree_model_data_did_change(generic_tree_model *model, generic_tree_item *item, int index, const QVariant &data)
{
	if(item->is_boolean(index))
	{
		telemetry_provider_field *field = (telemetry_provider_field *)item->get_context();

		if(data.toBool())
		{
			if(!field->enabled)
			{
				field->enabled = true;
				m_chart_view->add_data(field);
			}
		}
		else
		{
			if(field->enabled)
			{
				field->enabled = false;
				m_chart_view->remove_data(field);
			}
		}

		return true;
	}

	return false;
}

void document_window::update_telemetry()
{
	{
		generic_tree_item *root_item = new generic_tree_item({"Statistic", "Value"});

		for(auto &stat : m_telemetry.statistics)
		{
			generic_tree_item *child = root_item->add_child({ stat.title });

			for(auto &entry : stat.entries)
			{
				if(entry.second.metaType().id() == QMetaType::QStringList)
				{
					bool is_first = true;

					for(auto &string : entry.second.toStringList())
					{
						if(is_first)
						{
							child->add_child({ entry.first, string });
							is_first = false;
						}
						else
							child->add_child({ "", string });
					}
				}
				else
					child->add_child({ entry.first, entry.second });
			}
		}

		generic_tree_model *old_model = (generic_tree_model *)m_overview_view->model();
		generic_tree_model *model = new generic_tree_model(root_item);

		m_overview_view->setModel(model);
		m_overview_view->update();

		for(uint32_t i = 0; i < m_telemetry.statistics.size(); i ++)
		{
			const uint32_t index = m_telemetry.statistics.size() - 1 - i;
			m_overview_view->expand(model->index(index, 0, QModelIndex()));
		}

		delete old_model;
	}

	// Clear the old data
	m_event_picker->clear();
	m_event_ranges.clear();

	// Add the default fallback range
	{
		event_range everything;

		everything.start = m_telemetry.start_time;
		everything.end = m_telemetry.end_time;
		everything.name = "Everything";

		m_event_ranges.push_back(everything);
	}

	m_start_edit->set_range(m_telemetry.start_time, m_telemetry.end_time);
	m_start_edit->set_value(m_telemetry.start_time);

	m_end_edit->set_range(m_telemetry.start_time, m_telemetry.end_time);
	m_end_edit->set_value(m_telemetry.end_time);


	// Add all the telmetry providers
	{
		generic_tree_item *root_item = new generic_tree_item({"Provider", "Title"});

		QVector<uint32_t> expanded;

		{
			uint32_t index = 0;

			for(auto &provider: m_telemetry.providers)
			{
				generic_tree_item *child = root_item->add_child({provider.title});

				if(provider.identifier == provider_timing::identifier)
				{
					expanded.push_front(index);

					try
					{
						auto &cpu = provider.find_field(provider_timing::cpu);
						if(!cpu.data_points.empty())
							cpu.enabled = true;

						auto &gpu = provider.find_field(provider_timing::gpu);
						if(!gpu.data_points.empty())
							gpu.enabled = true;
					}
					catch(...)
					{}
				}

				if(provider.identifier == provider_sim_apup::identifier)
				{
					auto &do_world_events = provider.find_field(provider_sim_apup::do_world);
					auto &aircraft_events = provider.find_field(provider_sim_apup::loaded_aircraft);

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
								title = aircraft_events.get_data_point_after_time(start + 5.0).value.toString();
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
						}

					};

					for(auto &data : do_world_events.data_points)
					{
						if(data.value.toBool() && !is_doing_world)
						{
							is_doing_world = true;
							start_timestamp = data.timestamp;
						}

						if(data.value.toBool())
							end_timestamp = data.timestamp;

						if(!data.value.toBool() && is_doing_world)
						{
							is_doing_world = false;
							flush_range(start_timestamp, end_timestamp);
						}
					}

					if(is_doing_world)
						flush_range(start_timestamp, end_timestamp);
				}

				for(auto &entry: provider.fields)
				{
					if(entry.data_points.empty())
						continue;

					child->add_child({entry.enabled,
									  entry.title + " (" + telemetry_unit_to_string(entry.unit) + ")"}, &entry);
				}

				index++;
			}
		}

		generic_tree_model *old_model = (generic_tree_model *)m_providers_view->model();
		generic_tree_model *model = new generic_tree_model(root_item);
		model->set_delegate(this);

		m_providers_view->setModel(model);
		m_providers_view->update();

		for(auto &event : m_event_ranges)
			m_event_picker->addItem(event.name);

		if(m_event_ranges.size() > 1)
		{
			m_event_picker->setCurrentIndex(1);
			range_changed(1);
		}
		else
			set_time_range(m_telemetry.start_time, m_telemetry.end_time);


		for(auto index : expanded)
		{
			m_providers_view->expand(model->index(index, 0, QModelIndex()));

			for(auto &field : m_telemetry.providers[index].fields)
			{
				if(field.enabled)
					m_chart_view->add_data(&field);
			}
		}

		delete old_model;
	}
	{
		generic_tree_item *root_item = new generic_tree_item({"Event", "Duration", "Path"});
		auto add_span = [](generic_tree_item* root, const telemetry_event_span& ev) {
			auto add_span_impl = [](generic_tree_item* root, const telemetry_event_span& ev, auto& r) -> void
			{
				QString path;
				for (auto& f : ev.fields)
				{
					if (f.first == "path")
						path = f.second.toString();
				}

				generic_tree_item *child = root->add_child({ (quint64)ev.id, (ev.end - ev.begin) * 1000.0f, path });

				for (auto& child_ev: ev.child_spans)
					r(child, child_ev, r);
			};

			add_span_impl(root, ev, add_span_impl);
		};
		for (auto& ev : m_telemetry.event_spans)
			add_span(root_item, ev);

		generic_tree_model *old_model = (generic_tree_model*)m_timeline_tree->model();
		generic_tree_model *model = new generic_tree_model(root_item);
		m_timeline_tree->setModel(model);
		m_timeline_tree->update();
		delete old_model;

		m_timeline_widget->setTimelineSpans(m_telemetry.event_spans);
	}
}
