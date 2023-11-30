//
// Created by Sidney on 13-Jun-18.
//

#include <QApplication>
#include <QFileDialog>
#include <QProcess>
#include "document_window.h"

#include <thread>

#include "test_runner_dialog.h"
#include "model/telemetry_reader.h"
#include "utilities/xplane_installations.h"

document_window::document_window()
{
	setupUi(this);
	setWindowTitle(QString("Telemetry Viewer"));

	m_action_open->setShortcut(QKeySequence::Open);
	m_action_open->setStatusTip("Open a telemetry file");

	connect(m_action_open, SIGNAL(triggered()), this, SLOT(open_file()));

	m_action_exit->setShortcut(QKeySequence::Quit);

	connect(m_action_exit, SIGNAL(triggered()), qApp, SLOT(quit()));

	statusBar()->showMessage("Ready");

	connect(m_start_edit, &time_picker_widget::value_changed, this, &document_window::range_changed);
	connect(m_end_edit, &time_picker_widget::value_changed, this, &document_window::range_changed);
	connect(m_event_picker, qOverload<int>(&QComboBox::currentIndexChanged), this, &document_window::event_range_changed);

	m_splitter->setStretchFactor(0, 3);
	m_splitter->setStretchFactor(1, 1);

	connect(m_timeline_widget, &timeline_widget::spanFocused, [this](uint64_t id){
		auto model = m_timeline_tree->model();
		auto x = model->match(model->index(0,0, {}), Qt::DisplayRole, (quint64) id, 1, Qt::MatchFlag::MatchExactly|Qt::MatchFlag::MatchRecursive);
		if (!x.empty())
		{
			m_timeline_tree->scrollTo(x.front(),QAbstractItemView::ScrollHint::PositionAtTop);
			m_timeline_tree->selectionModel()->select(x.front(), QItemSelectionModel::SelectionFlag::ClearAndSelect|QItemSelectionModel::Rows);
		}
	});

	connect(m_run_tests, SIGNAL(pressed()), this, SLOT(run_fps_test()));

	m_installations = get_xplane_installations();

	for(auto &install : m_installations)
		m_installation_selector->addItem(install.path);
}

void document_window::load_file(const QString &path)
{
	m_telemetry = read_telemetry_data(path);

	m_chart_view->clear();
	m_chart_view->set_range(m_telemetry.start_time, m_telemetry.end_time);

	update_telemetry();
	setWindowFilePath(path);
}

void document_window::open_file()
{
	QString base_path = "";

	if(!m_installations.empty())
		base_path = m_installations[m_installation_selector->currentIndex()].telemetry_path;

	QString path = QFileDialog::getOpenFileName(this, tr("Open Telemetry file"), base_path, tr("Telemetry File (*.tlm)"));
	if(!path.isEmpty())
	{
		load_file(path);
	}
}

void document_window::run_fps_test()
{
	test_runner_dialog dialog(&m_installations[m_installation_selector->currentIndex()]);
	const int result = dialog.exec();

	if(result)
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
		process.start(dialog.get_executable(), dialog.get_arguments(result_path));

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
	m_chart_view->set_range(m_start_edit->get_value(), m_end_edit->get_value());
}

void document_window::event_range_changed(int index)
{
	if(index == -1)
		return;

	m_chart_view->set_range(m_event_ranges[index].start, m_event_ranges[index].end);

	m_start_edit->set_value(m_event_ranges[index].start);
	m_end_edit->set_value(m_event_ranges[index].end);
}


bool document_window::tree_model_data_did_change(generic_tree_model *model, generic_tree_item *item, int index, const QVariant &data)
{
	if(item->is_boolean(index))
	{
		telemetry_provider_field *field = (telemetry_provider_field *)item->get_context();
		auto iterator = std::find(m_enabled_fields.begin(), m_enabled_fields.end(), field);

		if(data.toBool())
		{
			if(iterator == m_enabled_fields.end())
			{
				m_enabled_fields.push_back(field);
				m_chart_view->add_data(field);
			}
		}
		else
		{
			if(iterator != m_enabled_fields.end())
			{
				m_enabled_fields.erase(iterator);
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
				child->add_child({ entry.first, entry.second });
			}
		}

		generic_tree_model *old_model = (generic_tree_model *)m_statistics_view->model();
		generic_tree_model *model = new generic_tree_model(root_item);

		m_statistics_view->setModel(model);
		m_statistics_view->update();

		for(uint32_t i = 0; i < m_telemetry.statistics.size(); i ++)
		{
			const uint32_t index = m_telemetry.statistics.size() - 1 - i;
			m_statistics_view->expand(model->index(index, 0, QModelIndex()));
		}

		delete old_model;
	}

	// Clear the old data
	m_event_picker->clear();
	m_event_ranges.clear();

	m_enabled_fields.clear();

	// Add the default fallback range
	{
		event_range everything;

		everything.start = m_telemetry.start_time;
		everything.end = m_telemetry.end_time;
		everything.name = "Everything";

		m_event_ranges.push_back(everything);
	}

	// Add all the telmetry providers
	{
		generic_tree_item *root_item = new generic_tree_item({"Provider", "Title"});

		QVector<uint32_t> expanded;

		{
			uint32_t index = 0;

			for(auto &provider: m_telemetry.providers)
			{
				generic_tree_item *child = root_item->add_child({provider.title});

				for(auto &entry: provider.fields)
				{
					child->add_child({entry.enabled,
									  entry.title + " (" + telemetry_unit_to_string(entry.unit) + ")"}, &entry);
				}

				if(provider.identifier == "com.laminarresearch.test_main_class")
				{
					expanded.push_front(index);
				}

				if(provider.identifier == "com.laminarresarch.sim_apup")
				{
					auto &do_world_events = provider.find_field(0);
					auto &aircraft_events = provider.find_field(2);

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
								title = aircraft_events.get_data_point_after_time(start + 5.0).second.toString();
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
						if(data.second.toBool() && !is_doing_world)
						{
							is_doing_world = true;
							start_timestamp = data.first;
						}

						if(data.second.toBool())
							end_timestamp = data.first;

						if(!data.second.toBool() && is_doing_world)
						{
							is_doing_world = false;
							flush_range(start_timestamp, end_timestamp);
						}
					}

					if(is_doing_world)
						flush_range(start_timestamp, end_timestamp);
				}

				index++;
			}
		}

		generic_tree_model *old_model = (generic_tree_model *)m_providers_view->model();
		generic_tree_model *model = new generic_tree_model(root_item);
		model->set_delegate(this);

		m_providers_view->setModel(model);
		m_providers_view->update();

		for(auto index : expanded)
			m_providers_view->expand(model->index(index, 0, QModelIndex()));

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

	for(auto &event : m_event_ranges)
		m_event_picker->addItem(event.name);

	m_start_edit->set_range(m_telemetry.start_time, m_telemetry.end_time);
	m_start_edit->set_value(m_telemetry.start_time);

	m_end_edit->set_range(m_telemetry.start_time, m_telemetry.end_time);
	m_end_edit->set_value(m_telemetry.end_time);
}
