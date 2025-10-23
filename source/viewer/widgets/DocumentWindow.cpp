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

DocumentWindow::DocumentWindow(TelemetryDocument *document)
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

	if(document)
		add_document(document);
}

DocumentWindow::~DocumentWindow()
{
	for(auto &document : m_loaded_documents)
	{
		delete document->document;
		delete document;
	}
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
			clear();
			add_document_by_path(action->data().toString());
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
	telemetry_field_lookup lookup = item->data(0, Qt::UserRole).value<telemetry_field_lookup>();
	const auto iterator = std::find(m_enabled_fields.begin(), m_enabled_fields.end(), lookup);

	const bool was_enabled = iterator != m_enabled_fields.end();
	const bool enabled = item->checkState(0) == Qt::Checked;

	if(was_enabled == enabled)
		return;

	set_field_enabled(lookup, enabled);
}

void DocumentWindow::document_selection_changed(QTreeWidgetItem *item)
{
	if(!item)
		return;

	loaded_document *document = item->data(0, Qt::UserRole).value<loaded_document *>();
	Q_ASSERT(document);

	update_selected_document(document);
}

void DocumentWindow::document_item_changed(QTreeWidgetItem *item)
{
	loaded_document *document = item->data(0, Qt::UserRole).value<loaded_document *>();
	Q_ASSERT(document);

	document->enabled = (item->checkState(0) == Qt::Checked);
	update_statistics_view();

	for(auto &lookup : m_enabled_fields)
	{
		const telemetry_field *field = lookup_field(lookup, document->document);
		if(!field)
			continue;

		if(document->enabled)
			m_chart_view->show_data(field);
		else
			m_chart_view->hide_data(field);
	}
}

QColor DocumentWindow::get_color_for_telemetry_field(const telemetry_field *field, const loaded_document *document) const
{
	if(document->seed.length() == 0)
		return generate_color(QString::fromStdString(field->get_title()), 0.9f, 0.4f);

	return generate_color(QString::fromStdString(field->get_title()) + document->seed, 0.9f, 0.4f);
}

const telemetry_field *DocumentWindow::lookup_field(const telemetry_field_lookup &lookup, TelemetryDocument *document) const
{
	const auto &data = document->get_data();
	if(!data.has_provider(lookup.identifier))
		return nullptr;

	const auto &provider = data.get_provider(lookup.identifier);
	if(!provider.has_field(lookup.field_id))
		return nullptr;

	const telemetry_field &field = provider.get_field(lookup.field_id);
	return &field;
}

void DocumentWindow::set_field_enabled(const telemetry_field_lookup &lookup, bool enable)
{
	QVector<QPair<const telemetry_field *, loaded_document *>> fields;

	for(auto &container : m_loaded_documents)
	{
		const telemetry_field *field = lookup_field(lookup, container->document);
		if(field)
			fields.push_back(qMakePair(field, container));
	}

	if(enable)
	{
		for(auto &[ field, document ] : fields)
		{
			QColor primary_color = get_color_for_telemetry_field(field, document);
			m_chart_view->add_data(field, primary_color, document->start_offset);

			if(!document->enabled)
				m_chart_view->hide_data(field);
		}

		m_enabled_fields.push_back(lookup);
	}
	else
	{
		for(auto &[ field, offset ] : fields)
			m_chart_view->remove_data(field);

		const auto iterator = std::find(m_enabled_fields.begin(), m_enabled_fields.end(), lookup);
		Q_ASSERT(iterator != m_enabled_fields.end());

		m_enabled_fields.erase(iterator);
	}

	update_statistics_view();
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
	{
		add_document_by_path(url.toLocalFile());
	}
}

void DocumentWindow::clear()
{
	setWindowFilePath("");

	m_chart_view->clear();

	m_event_picker->clear();

	m_providers_view->clear();
	m_overview_view->clear();
	m_timeline_tree->clear();

	m_enabled_fields.clear();

	m_documents_tree->clear();

	for(auto &container : m_loaded_documents)
	{
		delete container->document;
		delete container;
	}

	m_loaded_documents.clear();

	update_statistics_view();
}

void DocumentWindow::add_document_by_path(const QString &path, const QString &name)
{
	try
	{
		TelemetryDocument *document = qApp->load_file(path);

		if(!name.isEmpty())
			document->set_name(name);

		add_document(document);
	}
	catch(...)
	{
		statusBar()->showMessage("Failed to load telemetry file " + path);
	}
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
	const bool is_first_document = m_loaded_documents.isEmpty();

	loaded_document *entry = new loaded_document;
	entry->document = document;
	entry->start_offset = 0.0;

	if(!is_first_document)
		entry->seed = document->get_name();

	if(!is_first_document)
	{
		TelemetryDocument *root_document = m_loaded_documents.front()->document;
		auto &root_regions = root_document->get_regions();
		auto &regions = document->get_regions();

		if(root_regions.size() != regions.size())
			throw std::range_error("Telemetry document has different timing regions");

		size_t count = regions.size();
		for(size_t i = 0; i < count; ++ i)
		{
			if(root_regions[i].type != regions[i].type || root_regions[i].name != regions[i].name)
				throw std::range_error("Telemetry document has different timing regions");

			if(entry->start_offset <= 0.0 && regions[i].type == TelemetryRegion::Type::Flying)
				entry->start_offset = root_regions[i].start - regions[i].start;
		}
	}

	m_loaded_documents.push_back(entry);

	{
		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setCheckState(0, Qt::Checked);
		item->setText(0, document->get_name());
		item->setData(0, Qt::UserRole, QVariant::fromValue(entry));

		m_documents_tree->addTopLevelItem(item);

		if(is_first_document)
			item->setSelected(true);
	}

	if(is_first_document)
	{
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

						telemetry_field_lookup lookup;
						lookup.identifier = provider.get_identifier();
						lookup.field_id = field.get_id();

						QTreeWidgetItem *item = new QTreeWidgetItem(provider_item);
						item->setCheckState(0, Qt::CheckState::Unchecked);
						item->setData(0, Qt::UserRole, QVariant::fromValue(lookup));
						item->setBackground(0, get_color_for_telemetry_field(&field, entry));
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

		update_selected_document(entry);

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
	else
	{
		for(auto &lookup : m_enabled_fields)
		{
			const telemetry_field *field = lookup_field(lookup, document);
			if(field)
			{
				QColor color = get_color_for_telemetry_field(field, entry);
				m_chart_view->add_data(field, color, entry->start_offset);
			}
		}

		update_statistics_view();
	}
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
				add_document_by_path(path);
		}

		state.endArray();
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
		TelemetryDocument *doc = document->document;
		if(doc->get_path().isEmpty())
			continue;

		state.setArrayIndex(index ++);
		state.setValue("path", document->document->get_path());
	}

	state.endArray();

	state.setValue("start", m_start_edit->get_value());
	state.setValue("end", m_end_edit->get_value());
	state.setValue("region", m_event_picker->currentIndex());
}

 void DocumentWindow::set_time_range(int32_t start, int32_t end)
{
	m_chart_view->set_range(start, end);
	update_statistics_view();
}

void DocumentWindow::update_selected_document(const loaded_document *document)
{
	m_overview_view->clear();

	const auto &container = document->document->get_data();

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

	for(int i = 0; i < m_providers_view->topLevelItemCount(); ++ i)
	{
		QTreeWidgetItem *item = m_providers_view->topLevelItem(i);

		for(int j = 0; j < item->childCount(); ++ j)
		{
			QTreeWidgetItem *child = item->child(j);
			telemetry_field_lookup lookup = child->data(0, Qt::UserRole).value<telemetry_field_lookup>();

			const telemetry_field *field = lookup_field(lookup, document->document);
			if(!field)
			{
				child->setBackground(0, Qt::NoBrush);
				continue;
			}

			child->setBackground(0, get_color_for_telemetry_field(field, document));
		}
	}
}

void DocumentWindow::update_statistics_view()
{
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

	auto build_bar_set = [&](const telemetry_field &field, loaded_document *document) -> QBarSet * {

		try
		{
			PerformanceCalculator perf(field, m_chart_view->get_start(), m_chart_view->get_end());

			QBarSet *set = new QBarSet(QString::fromStdString(field.get_title()));
			set->setColor(get_color_for_telemetry_field(&field, document));

			switch(field.get_unit())
			{
				case telemetry_unit::time:
				{
					for(auto &perf_set : perf_series)
					{
						if(perf_set.percentile <= 0.0f)
							set->append(perf.calculate_average() * 1000.0);
						else
							set->append(perf.calculate_percentile(perf_set.percentile) * 1000.0);
					}

					break;
				}

				case telemetry_unit::fps:
				case telemetry_unit::value:
				{
					for(auto &perf_set : perf_series)
					{
						if(perf_set.percentile <= 0.0f)
							set->append(perf.calculate_average());
						else
							set->append(perf.calculate_percentile(perf_set.percentile));
					}

					break;
				}

				default:
				{
					delete set;
					return nullptr;
				}
			}


			return set;
		}
		catch(...)
		{
			return nullptr;
		}
	};

	QList<QBarSet *> fps_sets;
	QList<QBarSet *> time_sets;
	QList<QBarSet *> value_sets;

	for(auto &document : m_loaded_documents)
	{
		if(!document->enabled)
			continue;

		for(auto &lookup : m_enabled_fields)
		{
			const telemetry_field *field = lookup_field(lookup, document->document);
			if(!field)
				continue;

			const telemetry_unit unit = field->get_unit();

			if(unit != telemetry_unit::time && unit != telemetry_unit::fps && unit != telemetry_unit::value)
				continue;

			const auto &data = document->document->get_data();
			if(!data.has_provider(field->get_provider_id()))
				continue;

			const auto &provider = data.get_provider(field->get_provider_id());
			if(!provider.has_field(field->get_id()))
				continue;

			const telemetry_field &additional_field = provider.get_field(field->get_id());

			if(QBarSet *set = build_bar_set(additional_field, document))
			{
				switch(unit)
				{
					case telemetry_unit::time:
						time_sets.append(set);
						break;
					case telemetry_unit::value:
						value_sets.append(set);
						break;
					case telemetry_unit::fps:
						fps_sets.append(set);
						break;
				}
			}
		}
	}

	std::reverse(fps_sets.begin(), fps_sets.end());
	std::reverse(time_sets.begin(), time_sets.end());
	std::reverse(value_sets.begin(), value_sets.end());

	auto build_bar_series = [&](const QList<QBarSet *> &sets, int precision, const QString &series_label, const QString &unit_label) {

		if(sets.isEmpty())
			return;

		QHorizontalBarSeries *series = new QHorizontalBarSeries();
		series->append(sets);
		series->setLabelsVisible(true);
		series->setLabelsPosition(QAbstractBarSeries::LabelsInsideEnd);
		series->setLabelsPrecision(precision);
		series->setLabelsFormat(series_label);

		chart->addSeries(series);

		auto y_axis = new QBarCategoryAxis();

		for(auto &perf_set : perf_series)
			y_axis->append(perf_set.name);

		chart->addAxis(y_axis, Qt::AlignLeft);
		series->attachAxis(y_axis);

		auto x_axis = new QValueAxis();
		x_axis->setLabelFormat(unit_label);

		chart->addAxis(x_axis, Qt::AlignBottom);

		series->attachAxis(x_axis);
		x_axis->applyNiceNumbers();

	};

	build_bar_series(time_sets, 3, "@valuems", "%ims");
	build_bar_series(fps_sets, 1, "@valueFPS", "%iFPS");
	build_bar_series(value_sets, 1, "@value", "%i");

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
				add_document(document);
			}
			catch(...)
			{
				clear();
				statusBar()->showMessage("Failed to load telemetry file!");
			}
		}
	}
}

void DocumentWindow::save_file()
{
	for(auto &document : m_loaded_documents)
	{
		if(!document->document->has_data())
			continue;

		QString base_path = m_base_dir;

		if(!m_installations.empty() && base_path.isEmpty())
			base_path = m_installations[m_installation_selector->currentIndex()].get_telemetry_path();

		base_path = base_path % '/' % document->document->get_name();

		QString path = QFileDialog::getSaveFileName(this, tr("Save Telemetry file"), base_path, tr("Telemetry File (*.tlm)"));

		if(!path.isEmpty())
			document->document->save(path);
	}
}

void DocumentWindow::close_file()
{
	if(m_loaded_documents.size() == 1)
	{
		clear();
		return;
	}

	QTreeWidgetItem *item = m_documents_tree->currentItem();
	if(!item)
		return;

	loaded_document *document = item->data(0, Qt::UserRole).value<loaded_document *>();
	Q_ASSERT(document);

	auto iterator = std::find(m_loaded_documents.begin(), m_loaded_documents.end(), document);
	Q_ASSERT(iterator != m_loaded_documents.end());

	if(iterator == m_loaded_documents.begin())
		return;

	for(auto &lookup : m_enabled_fields)
	{
		const telemetry_field *field = lookup_field(lookup, document->document);
		if(!field)
			continue;

		m_chart_view->remove_data(field);
	}

	delete item;

	m_loaded_documents.erase(iterator);

	update_statistics_view();

	delete document->document;
	delete document;

}
void DocumentWindow::close_all_files()
{
	clear();
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
				add_document_by_path(full_result_path, runner.get_name());
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

	auto &regions = m_loaded_documents[0]->document->get_regions();

	set_time_range(regions[index].start, regions[index].end);

	m_start_edit->set_value(regions[index].start);
	m_end_edit->set_value(regions[index].end);
}
