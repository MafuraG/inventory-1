#include <QtGui/QMessageBox>
#include <QtSql/QSqlError>

#include "models.h"

#include "filterdialog.h"

FilterDialog::FilterDialog(QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);

	try {
		itemTypeModel = new ItemTypeModel(this);
		placeModel = new PlaceModel(this);
	} catch(IIdObject::InvalidIdException e) {
		QMessageBox::critical(this, tr("Filter dialog error"), tr("Database invalid id exception!"));
	} catch(IDatabaseObject::DBErrorException e) {
		QMessageBox::critical(this, tr("Filter dialog error"), e.query.lastError().databaseText() + "\n" + e.query.lastError().driverText());
	}

	ui.listItemType->setModel(itemTypeModel);
	ui.listPlace->setModel(placeModel);
}

FilterDialog::~FilterDialog() {
}

InventoryViewFilter FilterDialog::filter() const {
	InventoryViewFilter result;
	result.usePlace = ui.groupPlace->isChecked();
	result.useItemType = ui.groupItemType->isChecked();
	result.useActivity = ui.groupActive->isChecked();
	result.placeId = placeModel->idAt(ui.listPlace->currentIndex());
	result.itemTypeId = itemTypeModel->idAt(ui.listItemType->currentIndex());
	result.active = ui.buttonActive->isChecked();
	return result;
}

void FilterDialog::setFilter(const InventoryViewFilter &newFilter) {
	ui.groupPlace->setChecked(newFilter.usePlace);
	ui.groupItemType->setChecked(newFilter.useItemType);
	ui.groupActive->setChecked(newFilter.useActivity);
	if(newFilter.usePlace)
		ui.listPlace->setCurrentIndex(placeModel->indexOf(Place(newFilter.placeId)));
	if(newFilter.useItemType)
		ui.listItemType->setCurrentIndex(itemTypeModel->indexOf(ItemType(newFilter.itemTypeId)));
	if(newFilter.useActivity)
		ui.buttonActive->setChecked(newFilter.active);
}

