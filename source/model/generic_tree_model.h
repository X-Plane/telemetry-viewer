//
// Created by Sidney on 03/06/2020.
//

#ifndef TELEMETRY_STUDIO_GENERIC_TABLE_MODEL_H
#define TELEMETRY_STUDIO_GENERIC_TABLE_MODEL_H

#include <QAbstractItemModel>
#include "generic_tree_item.h"

class generic_tree_model;

class generic_tree_model_delegate
{
public:
	virtual bool tree_model_data_did_change(generic_tree_model *model, generic_tree_item *item, int index, const QVariant &data) { return false; };
};

class generic_tree_model final : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit generic_tree_model(generic_tree_item *root_item, QObject *parent = nullptr);
	~generic_tree_model() override;

	void set_delegate(generic_tree_model_delegate *delegate);

	int columnCount(const QModelIndex &parent) const override;
	int rowCount(const QModelIndex &parent) const override;

	QVariant data(const QModelIndex &index, int role) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	QModelIndex index(int row, int column, const QModelIndex &parent) const override;
	QModelIndex parent(const QModelIndex &index) const override;

	Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
	generic_tree_item *get_item(const QModelIndex &index) const;

	generic_tree_item *m_root_item;
	generic_tree_model_delegate *m_delegate;
};


#endif //TELEMETRY_STUDIO_GENERIC_TABLE_MODEL_H
