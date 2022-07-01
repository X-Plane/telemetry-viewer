//
// Created by Sidney on 03/06/2020.
//

#ifndef TELEMETRY_STUDIO_GENERIC_TABLE_ITEM_H
#define TELEMETRY_STUDIO_GENERIC_TABLE_ITEM_H

#include <QVariant>
#include <QVector>

class generic_tree_item
{
public:
	generic_tree_item(const QVector<QVariant> &data, void *context = nullptr, generic_tree_item *parent = nullptr);
	~generic_tree_item();

	generic_tree_item *get_parent() const { return m_parent; }
	int get_child_count() const { return m_children.size(); }
	int get_column_count() const { return m_data.size(); }

	void *get_context() const { return m_context; }

	QVariant get_data(int index) const;
	generic_tree_item *get_child(int index) const { return m_children[index]; }

	generic_tree_item *insert_child(int position, const QVector<QVariant> &data, void *context = nullptr);
	generic_tree_item *add_child(const QVector<QVariant> &data, void *context = nullptr);

	void set_data(const QVariant &data, int index);

	int get_index_in_parent() const;

	bool is_boolean(int index) const;

private:
	generic_tree_item *m_parent;
	QVector<generic_tree_item *> m_children;
	QVector<QVariant> m_data;
	void *m_context;
};


#endif //TELEMETRY_STUDIO_GENERIC_TABLE_ITEM_H
