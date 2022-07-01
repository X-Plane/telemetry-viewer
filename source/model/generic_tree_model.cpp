//
// Created by Sidney on 03/06/2020.
//

#include "generic_tree_model.h"

generic_tree_model::generic_tree_model(generic_tree_item *root_item, QObject *parent) :
	QAbstractItemModel(parent),
	m_root_item(root_item),
	m_delegate(nullptr)
{}
generic_tree_model::~generic_tree_model()
{}

void generic_tree_model::set_delegate(generic_tree_model_delegate *delegate)
{
	m_delegate = delegate;
}

int generic_tree_model::columnCount(const QModelIndex &parent) const
{
	return m_root_item->get_column_count();
}
int generic_tree_model::rowCount(const QModelIndex &parent) const
{
	const generic_tree_item *item = get_item(parent);
	return item ? item->get_child_count() : 0;
}

QVariant generic_tree_model::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();

	generic_tree_item *item = get_item(index);

	if(item->is_boolean(index.column()))
	{
		if(role == Qt::CheckStateRole || role == Qt::EditRole)
		{
			const QVariant &data = item->get_data(index.column());
			return (data.toBool() ? Qt::Checked : Qt::Unchecked);
		}

		return QVariant();
	}

	if(role == Qt::DisplayRole || role == Qt::EditRole)
		return item->get_data(index.column());

	return QVariant();
}

QVariant generic_tree_model::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return m_root_item->get_data(section);

	return QVariant();
}

QModelIndex generic_tree_model::index(int row, int column, const QModelIndex &parent) const
{
	if(parent.isValid() && parent.column() != 0)
		return QModelIndex();

	generic_tree_item *parent_item = get_item(parent);
	if(!parent_item)
		return QModelIndex();

	generic_tree_item *child = parent_item->get_child(row);
	if(child)
		return createIndex(row, column, child);

	return QModelIndex();
}

QModelIndex generic_tree_model::parent(const QModelIndex &index) const
{
	if(!index.isValid())
		return QModelIndex();

	generic_tree_item *child = get_item(index);
	generic_tree_item *parent_item = child ? child->get_parent() : nullptr;

	if(parent_item == m_root_item || !parent_item)
		return QModelIndex();

	return createIndex(parent_item->get_index_in_parent(), 0, parent_item);
}

Qt::ItemFlags generic_tree_model::flags(const QModelIndex &index) const
{
	if(!index.isValid())
		return Qt::NoItemFlags;

	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	generic_tree_item *item = get_item(index);

	if(item->is_boolean(index.column()))
		flags |= Qt::ItemIsUserCheckable;

	return flags;
}

bool generic_tree_model::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!index.isValid())
		return false;

	QVariant new_value = value;

	generic_tree_item *item = get_item(index);

	if(role == Qt::CheckStateRole && item->is_boolean(index.column()))
	{
		const bool bool_value = value.toBool();
		new_value = QVariant(bool_value);
	}

	if(m_delegate && m_delegate->tree_model_data_did_change(this, item, index.column(), new_value))
	{
		item->set_data(new_value, index.column());
		return true;
	}

	return false;
}


generic_tree_item *generic_tree_model::get_item(const QModelIndex &index) const
{
	if(index.isValid())
	{
		generic_tree_item *item = static_cast<generic_tree_item *>(index.internalPointer());
		if (item)
			return item;
	}

	return m_root_item;
}

