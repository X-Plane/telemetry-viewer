//
// Created by Sidney on 13-Jun-18.
//

#include <QApplication>
#include <QFileDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include "document_window.h"
#include "model/telemetry_reader.h"

QT_CHARTS_USE_NAMESPACE

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

	m_splitter->setStretchFactor(0, 3);
	m_splitter->setStretchFactor(1, 1);
}

document_window::~document_window()
{
}

void document_window::open_file()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Open Telemetry file"), "", tr("Telemetry File (*.tlm)"));
	if(!path.isEmpty())
	{
		m_telemetry = read_telemetry_data(path);

		m_chart_view->clear();
		m_chart_view->set_range(m_telemetry.start_time, m_telemetry.end_time);

		update_telemetry();
		setWindowFilePath(path);
	}
}

void document_window::range_changed(int32_t value)
{
	m_chart_view->set_range(m_start_edit->get_value(), m_end_edit->get_value());
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
					expanded.push_front(index);

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

	m_enabled_fields.clear();

	m_start_edit->set_range(m_telemetry.start_time, m_telemetry.end_time);
	m_start_edit->set_value(m_telemetry.start_time);

	m_end_edit->set_range(m_telemetry.start_time, m_telemetry.end_time);
	m_end_edit->set_value(m_telemetry.end_time);
}
