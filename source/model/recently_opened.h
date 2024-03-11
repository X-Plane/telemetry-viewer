//
// Created by Sidney on 11/03/2024.
//

#ifndef RECENTLY_OPENED_H
#define RECENTLY_OPENED_H

#include <QObject>

#define MAX_RECENTLY_OPENED 5

class recently_opened : public QObject
{
Q_OBJECT
public:
	recently_opened();
	~recently_opened();

	void add_entry(const QString &path);
	void clear_entries();

	const QList<QString> &get_entries() const { return m_entries; }

private:
	void save();

	QList<QString> m_entries;
	bool m_is_dirty;
};

#endif //RECENTLY_OPENED_H
