#pragma once

#include <QtCore/QAbstractItemModel>
#include <QtGui/QItemDelegate>
#include <QtSql/QSqlQueryModel>

#include "inventory.h"

// @todo: - error handling ,exception catching
// @todo: show note icon in name field, and tooltip, and edit note in inputdialog via context menu.
class PlaceModel : public QAbstractItemModel {
	Q_OBJECT
public:
	PlaceModel(QObject *parent = 0); // Throws DBErrorException, InvalidIdException.
	virtual ~PlaceModel();

	void updateList(); // Throws DBErrorException, InvalidIdException.
	int indexOf(const Place &place) const;
	int idAt(int row) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // Tool tip.
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &index) const;
private:
	QList<Place> list;

};

class ItemTypeModel : public QAbstractItemModel {
	Q_OBJECT
public:
	ItemTypeModel(QObject *parent = 0); // Throws DBErrorException, InvalidIdException.
	virtual ~ItemTypeModel();

	void updateList(); // Throws DBErrorException, InvalidIdException.
	int indexOf(const ItemType &itemType) const;
	int idAt(int row) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // Tool tip.
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &index) const;
private:
	QList<ItemType> list;

};

class InventoryModel : public QAbstractItemModel {
	Q_OBJECT
public:
	enum FieldType {UnknownField = -1, ItemTypeField = 0, NameField = 1, InnField = 2, PlaceField = 3, ActiveField = 4, NoteField = 5, FieldCount = 6};

	InventoryModel(QObject *parent = 0);
	virtual ~InventoryModel();

	void updateList(const QList<Item> &itemList);
	int indexOf(const Item &item) const;
	int idAt(int row) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // Tool tip.
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &index) const;
private:
	QList<Item> list;

};

class InventoryDelegate : public QItemDelegate {
	Q_OBJECT
	Q_DISABLE_COPY(InventoryDelegate)
public:
	InventoryDelegate(QObject *parent = 0);
	virtual ~InventoryDelegate();

	virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
	virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


class HistoryModel : public QAbstractItemModel {
	Q_OBJECT
public:
	enum FieldType {UnknownField = -1, TimeField = 0, NameField = 1, OldValueField = 2, NewValueField = 3, FieldCount = 4};

	HistoryModel(const Item &item, QObject *parent = 0); // Throws DBErrorException, InvalidIdException.
	virtual ~HistoryModel();

	void updateList(); // Throws DBErrorException, InvalidIdException.

	virtual Qt::ItemFlags flags(const QModelIndex &index) const; // Read-only.
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const; // Tool tip.
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &index) const;
private:
	Item item;
	QList<History> list;
};

class SortingSqlQueryModel : public QSqlQueryModel {
	Q_OBJECT
	Q_DISABLE_COPY(SortingSqlQueryModel)
public:
	SortingSqlQueryModel(QObject *parent = 0) : QSqlQueryModel(parent) {
		setQuery(Inventory::instance()->getQuery());
	}
	virtual ~SortingSqlQueryModel() {}

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) {
		setQuery(Inventory::instance()->getQuery(column, order));
	}
};

