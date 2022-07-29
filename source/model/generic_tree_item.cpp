//
// Created by Sidney on 03/06/2020.
//

#include "generic_tree_item.h"

generic_tree_item::generic_tree_item(const QVector<QVariant> &data, void *context, generic_tree_item *parent) :
	m_parent(parent),
	m_data(data),
	m_context(context)
{}

generic_tree_item::~generic_tree_item()
{
	qDeleteAll(m_children);
}

void generic_tree_item::set_data(const QVariant &data, int index)
{
	if(index < 0 || index >= m_data.size())
		return;

	m_data[index] = data;
}

QVariant generic_tree_item::get_data(int index) const
{
	if(index < 0 || index >= m_data.size())
		return QVariant();

	return m_data[index];
}

generic_tree_item *generic_tree_item::insert_child(int position, const QVector<QVariant> &data, void *context)
{
	if(position < 0 || position > m_children.size())
		return nullptr;

	generic_tree_item *item = new generic_tree_item(data, context, this);
	m_children.insert(position, item);

	return item;
}
generic_tree_item *generic_tree_item::add_child(const QVector<QVariant> &data, void *context)
{
	generic_tree_item *item = new generic_tree_item(data, context, this);
	m_children.push_back(item);

	return item;
}

int generic_tree_item::get_index_in_parent() const
{
	if(m_parent)
		return m_parent->m_children.indexOf(const_cast<generic_tree_item *>(this));

	return 0;
}

bool generic_tree_item::is_boolean(int index) const
{
	if(index < 0 || index >= m_data.size())
		return false;

	const QVariant &variant = m_data[index];
	return (variant.type() == QVariant::Bool);
}
