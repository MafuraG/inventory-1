#include <QtDebug>
#include <QtCore/QSettings>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QMessageBox>
#include <QtGui/QTextEdit>
#include <QtGui/QFileDialog>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget * parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	tabs = new QTabBar();
	tabIndex.MAIN    = tabs->addTab(tr("Main"));
	tabIndex.PRINT   = tabs->addTab(tr("Print"));
	tabIndex.TYPES   = tabs->addTab(tr("Item types"));
	tabIndex.PLACES  = tabs->addTab(tr("Places"));
	tabIndex.PERSONS = tabs->addTab(tr("Persons"));

	QBoxLayout * box = static_cast<QBoxLayout*>(ui.centralwidget->layout());
	if(box) {
		box->insertWidget(0, tabs);
	}
	connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(setupTab(int)));

	// Settings.
	QSettings settings;
	resize(settings.value("mainwindow/size", size()).toSize());
	move(settings.value("mainwindow/pos", pos()).toPoint());
	if(settings.value("mainwindow/maximized", false).toBool())
		setWindowState(Qt::WindowMaximized);

	QString databaseLocation = settings.value("database/location", "").toString();
	if(databaseLocation.isEmpty()) {
		databaseLocation = QFileDialog::getSaveFileName(this, tr("Database location"),
				QDir(QApplication::applicationDirPath()).absoluteFilePath("inventory.sqlite"),
				tr("SQLite3 database files (*.sqlite);;All files (*.*)")
				); 
	}

	// App logic.
	Inventory::Database::setDatabaseName(databaseLocation);
	if(!Inventory::Database::reopen()) {
		QMessageBox::critical(this, tr("Database"), tr("Cannot open database at path '%1'!").arg(Inventory::Database::databaseName()));
		exit(1);
	}
	inventoryModel = new Inventory::InventoryModel();
	printableModel = new Inventory::PrintableInventoryModel();
	itemTypesModel = new Inventory::ReferenceModel(Inventory::ReferenceModel::ITEM_TYPES);
	placesModel    = new Inventory::ReferenceModel(Inventory::ReferenceModel::PLACES);
	personsModel   = new Inventory::ReferenceModel(Inventory::ReferenceModel::PERSONS);
	connect(inventoryModel, SIGNAL(modelReset()), this, SLOT(resetView()));
	connect(printableModel, SIGNAL(modelReset()), this, SLOT(resetView()));
	connect(itemTypesModel, SIGNAL(modelReset()), this, SLOT(resetView()));
	connect(placesModel,    SIGNAL(modelReset()), this, SLOT(resetView()));
	connect(personsModel,   SIGNAL(modelReset()), this, SLOT(resetView()));

	ui.listItemTypeFilter->setModel(itemTypesModel);
	ui.listPlaceFilter->setModel(placesModel);

	if(tabs->currentIndex() == tabIndex.MAIN) {
		setupTab(tabIndex.MAIN);
	} else {
		tabs->setCurrentIndex(tabIndex.MAIN);
	}
}

MainWindow::~MainWindow()
{
	// Settings.
	QSettings settings;
	settings.setValue("mainwindow/maximized",
			windowState().testFlag(Qt::WindowMaximized));
	if(!windowState().testFlag(Qt::WindowMaximized))
	{
		settings.setValue("mainwindow/size", size());
		settings.setValue("mainwindow/pos", pos());
	}
	settings.setValue("database/location", Inventory::Database::databaseName());

	// Database.
	Inventory::Database::close();
}

void MainWindow::setupTab(int index)
{
	ui.actionShowHistory ->setEnabled(index == tabIndex.MAIN);
	ui.actionAddMultiline->setEnabled(index == tabIndex.TYPES || index == tabIndex.PLACES || index == tabIndex.PERSONS);
	ui.actionAdd         ->setEnabled(index != tabIndex.PRINT);
	ui.actionRemove      ->setEnabled(index != tabIndex.PRINT);
	ui.actionHideFilter  ->setEnabled(index == tabIndex.MAIN || index == tabIndex.PRINT);

	ui.filterBox->setVisible(ui.actionHideFilter->isEnabled() && !ui.actionHideFilter->isChecked());

	if     (index == tabIndex.MAIN)    ui.view->setModel(inventoryModel);
	else if(index == tabIndex.PRINT)   ui.view->setModel(printableModel);
	else if(index == tabIndex.TYPES)   ui.view->setModel(itemTypesModel);
	else if(index == tabIndex.PLACES)  ui.view->setModel(placesModel);
	else if(index == tabIndex.PERSONS) ui.view->setModel(personsModel);
	resetView(true);
}

void MainWindow::resetView(bool update)
{
	ui.view->resizeColumnsToContents();
	ui.view->horizontalHeader()->setStretchLastSection(true);
	if(update && ui.view->model()) {
		Inventory::AbstractUpdatableTableModel * model = qobject_cast<Inventory::AbstractUpdatableTableModel *>(ui.view->model());
		if(model) {
			model->update();
		}
	}
}

void MainWindow::on_actionShowHistory_triggered()
{
	if(ui.view->model() != inventoryModel)
		return;

	int row = ui.view->currentIndex().isValid() ? ui.view->currentIndex().row() : -1;
	if(row < 0)
		return;

	Inventory::Id id = inventoryModel->idAt(row);
	QScopedPointer<Inventory::HistoryModel> model(new Inventory::HistoryModel(id));

	QDialog dialog(this);
		QVBoxLayout * vbox = new QVBoxLayout();
			QTableView * view = new QTableView();
				view->setModel(&(*model));
			vbox->addWidget(view);
			QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Close);
				connect(buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));
				connect(buttons, SIGNAL(rejected()), &dialog, SLOT(reject()));
			vbox->addWidget(buttons);
	dialog.setLayout(vbox);
	dialog.setSizeGripEnabled(true);
	dialog.exec();
}

void MainWindow::on_actionAddMultiline_triggered()
{
	if(tabs->currentIndex() != tabIndex.TYPES && tabs->currentIndex() != tabIndex.PLACES && tabs->currentIndex() != tabIndex.PERSONS)
		return;

	QDialog dialog(this);
		QVBoxLayout * vbox = new QVBoxLayout();
			QTextEdit * edit = new QTextEdit();
			vbox->addWidget(edit);
			QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
				connect(buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));
				connect(buttons, SIGNAL(rejected()), &dialog, SLOT(reject()));
			vbox->addWidget(buttons);
	dialog.setLayout(vbox);
	dialog.setSizeGripEnabled(true);
	if(dialog.exec() == QDialog::Accepted) {
		QStringList lines = edit->toPlainText().split('\n');

		Inventory::ReferenceModel * model = qobject_cast<Inventory::ReferenceModel *>(ui.view->model());
		if(model) {
			bool success = model->addMultiline(lines);
			if(!success) {
				QMessageBox::critical(this, tr("Add lines"), tr("Adding of lines failed!"));
			}
		}
	}
}

void MainWindow::on_actionPrintCSV_triggered()
{
	if(!ui.view->model())
		return;

	QString csvFileName = QFileDialog::getSaveFileName(this, tr("Save to CSV..."), 0, tr("CSV files (*.csv);;All files (*.*)"));
	if(csvFileName.isEmpty())
		return;

	QFile file(csvFileName);
	if(!file.open(QFile::Text | QFile::WriteOnly)) {
		QMessageBox::critical(this, tr("Save to CSV"), tr("Cannot open file '%1' for write!").arg(file.fileName()));
		return;
	}

	QTextStream out(&file);
	QAbstractItemModel * model = ui.view->model();
	for(int row = 0; row < model->rowCount(); ++row) {
		QStringList cells;
		for(int col = 0; col < model->columnCount(); ++col) {
			QString text = model->data(model->index(row, col)).toString();
			text.replace("\"", "\\\"");
			text.append('"').prepend('"');
			cells << text;
		}
		out << cells.join(", ") << endl;
	}
}

void MainWindow::on_actionAdd_triggered()
{
	ui.view->model()->insertRow(ui.view->model()->rowCount());
}

void MainWindow::on_actionRemove_triggered()
{
	if(!ui.view->model())
		return;

	int row = ui.view->currentIndex().isValid() ? ui.view->currentIndex().row() : -1;
	if(row < 0)
		return;

	bool removed = !ui.view->model()->removeRow(row);
	if(removed) {
		if(tabs->currentIndex() == tabIndex.TYPES || tabs->currentIndex() == tabIndex.PLACES || tabs->currentIndex() == tabIndex.PERSONS) {
			QMessageBox::information(this, tr("Remove record"), tr("Cannot remove record. Probably, there are items that are using it."));
		}
	}
}

void MainWindow::on_buttonUseItemTypeFilter_toggled(bool value)
{
	inventoryModel->switchItemTypeFilter(value);
	printableModel->switchItemTypeFilter(value);
}

void MainWindow::on_listItemTypeFilter_currentIndexChanged(int index)
{
	Inventory::Id value = itemTypesModel->idAt(index);
	inventoryModel->setItemTypeFilter(value);
	printableModel->setItemTypeFilter(value);
}

void MainWindow::on_buttonUsePlaceFilter_toggled(bool value)
{
	inventoryModel->switchPlaceFilter(value);
	printableModel->switchPlaceFilter(value);
}

void MainWindow::on_listPlaceFilter_currentIndexChanged(int index)
{
	Inventory::Id value = placesModel->idAt(index);
	inventoryModel->setPlaceFilter(value);
	printableModel->setPlaceFilter(value);
}

void MainWindow::on_buttonUseWrittenOffFilter_toggled(bool value)
{
	inventoryModel->switchWrittenOffFilter(value);
	printableModel->switchWrittenOffFilter(value);
}

void MainWindow::on_listWrittenOffFilter_currentIndexChanged(int index)
{
	bool value = (index == 1);
	inventoryModel->setWrittenOffFilter(value);
	printableModel->setWrittenOffFilter(value);
}

