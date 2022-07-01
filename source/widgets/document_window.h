//
// Created by Sidney on 13-Jun-18.
//

#ifndef SPIRV_STUDIO_DOCUMENT_WINDOW_H
#define SPIRV_STUDIO_DOCUMENT_WINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <ui_document_window.h>
#include "model/telemetry_container.h"
#include "model/generic_tree_model.h"


class document_window final : public QMainWindow, public Ui::document_window, public generic_tree_model_delegate
{
Q_OBJECT

public:
	document_window();
	~document_window() final;

	bool tree_model_data_did_change(generic_tree_model *model, generic_tree_item *item, int index, const QVariant &data) override;

private slots:
	void open_file();
	void range_changed(int32_t value);

private:
	void update_telemetry();

	telemetry_container m_telemetry;
	std::vector<telemetry_provider_field *> m_enabled_fields;
};

#endif //SPIRV_STUDIO_DOCUMENT_WINDOW_H
