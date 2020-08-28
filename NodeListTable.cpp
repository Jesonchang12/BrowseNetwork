#include "NodeListTable.h"
#include "ui_NodeListTable.h"

NodeListTable::NodeListTable(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NodeListTable)
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 不可编辑
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);// 整行选中
//    ui->tableWidget->horizontalHeader()->setStyleSheet( "QHeaderView::section {background-color:white; border:none; border-right:1px solid gray;}");
//    ui->tableWidget->horizontalHeader()->setHighlightSections(false);    // 表头不可选中
    int col = ui->tableWidget->columnCount();
    for (int i = 0; i < col; i++) { // 表头不可选中
        QTableWidgetItem *item = ui->tableWidget->item(0,i);
        item->setFlags(item->flags()&~Qt::ItemIsSelectable);
    }
    ui->pushButton_2->setEnabled(false);
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &NodeListTable::enableOrDisableButton);
    m_searcher = new NetworkSearch(this);
    qRegisterMetaType<NetworkNodeInfo>("NetworkNodeInfo");
    qRegisterMetaType<NetworkNodeInfo>("NetworkNodeInfo&");
    connect(m_searcher, &NetworkSearch::signal_nodeListFound, this, &NodeListTable::append2Table,  Qt::QueuedConnection);
    connect(m_searcher, &NetworkSearch::signal_nodeInfoCatched, this, &NodeListTable::updateTable, Qt::QueuedConnection);
}

NodeListTable::~NodeListTable()
{
    delete ui;
    delete m_searcher;
}

void NodeListTable::append2Table(const QList<NetworkNodeInfo> &nodeList)
{
//    ui->tableWidget->clearContents();
    while (ui->tableWidget->rowCount() > 1) {
        ui->tableWidget->removeRow(1);
    }
    foreach (auto node, nodeList) {
        int rowCount = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(rowCount);
        ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(node.index)));
        ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(node.name));
        ui->tableWidget->setItem(rowCount, 2, new QTableWidgetItem(QString::number(node.moduleID)));
        ui->tableWidget->setItem(rowCount, 3, new QTableWidgetItem(node.ip));
        ui->tableWidget->setItem(rowCount, 4, new QTableWidgetItem(node.mac));
    }
}

void NodeListTable::updateTable(const NetworkNodeInfo &node)
{
    auto row = node.index;
//    qDebug() << "!!!row:" << row;
    if (!ui->tableWidget->item(row, 0)) return;
//    ui->tableWidget->item(row, 0)->setText(QString::number(node.index));
    ui->tableWidget->item(row, 1)->setText(node.name);
    ui->tableWidget->item(row, 2)->setText(QString::number(node.moduleID));
//    ui->tableWidget->item(row, 3)->setText(node.ip);
//    ui->tableWidget->item(row, 4)->setText(node.mac);
}

void NodeListTable::on_pushButton_2_clicked()
{
    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    NetworkNodeInfo info;
    int count = items.count();
    if (count > 4) {
        info.index = items.at(0)->text().toInt();
        info.name = items.at(1)->text();
        info.moduleID = items.at(2)->text().toInt();
        info.ip = items.at(3)->text();
        info.mac = items.at(4)->text();
        emit signal_selectedNode(info);
    }
//    this->close();
}

void NodeListTable::on_pushButton_3_clicked()
{
    this->close();
}

void NodeListTable::on_pushButton_clicked()
{
    m_searcher->disconnect();
    connect(m_searcher, &NetworkSearch::signal_nodeListFound, this, &NodeListTable::append2Table,  Qt::QueuedConnection);
    connect(m_searcher, &NetworkSearch::signal_nodeInfoCatched, this, &NodeListTable::updateTable, Qt::QueuedConnection);
    m_searcher->search4AllNodes();
}

void NodeListTable::enableOrDisableButton()
{
    ui->pushButton_2->setEnabled(ui->tableWidget->selectedItems().count() > 0 && ui->tableWidget->currentRow() > 0);
}
