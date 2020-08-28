#ifndef NODELISTTABLE_H
#define NODELISTTABLE_H

#include <QMainWindow>
#include <QList>
#include "NetworkSearch.h"

namespace Ui {
class NodeListTable;
}

class NodeListTable : public QMainWindow
{
    Q_OBJECT

public:
    explicit NodeListTable(QWidget *parent = 0);
    ~NodeListTable();

signals:
    void signal_selectedNode(const NetworkNodeInfo &info);

private slots:
    void append2Table(const QList<NetworkNodeInfo> &nodeList);
    void updateTable(const NetworkNodeInfo &node);

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void enableOrDisableButton();

private:
    Ui::NodeListTable *ui;
    NetworkSearch *m_searcher;
};

#endif // NODELISTTABLE_H
