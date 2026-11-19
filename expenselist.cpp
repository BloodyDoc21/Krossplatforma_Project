#include "expenselist.h"

#include <QDebug>
#include <QMouseEvent>
#include <QRandomGenerator>

ExpenseList::ExpenseList(QWidget *parent) : QListWidget(parent)
{
    setObjectName(QStringLiteral("expenses"));
    qDebug() << "[CREATE]" << objectName();
    connect(this, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *item) {
        if (item)
            emit expenseSelected(item->text());
    });
}

ExpenseList::~ExpenseList()
{
    qDebug() << "[DESTROY]" << objectName();
}

void ExpenseList::selectExpense(int row)
{
    if (row >= 0 && row < count()) {
        if (row == currentRow()) {
            restoreHighlight();
            emit expenseSelected(currentItem()->text());
        } else {
            setCurrentRow(row);
        }
    }
}

void ExpenseList::nextExpense()
{
    if (count() > 0)
        selectExpense((currentRow() + 1) % count());
}

void ExpenseList::randomExpense()
{
    if (count() > 1) {
        // При наличии выбора пропускаем текущую строку, чтобы смена была видна.
        const int row = currentRow();
        const int pick = QRandomGenerator::global()->bounded(count() - (row >= 0));
        selectExpense(pick + (row >= 0 && pick >= row));
    } else if (count() == 1) {
        selectExpense(0);
    }
}

void ExpenseList::blinkHighlight()
{
    if (auto *item = currentItem())
        item->setSelected(!item->isSelected());
    else if (count() > 0)
        selectExpense(0);
}

void ExpenseList::restoreHighlight()
{
    if (auto *item = currentItem())
        item->setSelected(true);
}

void ExpenseList::mousePressEvent(QMouseEvent *event)
{
    const auto button = event->button();
    if (count() == 0 || (button != Qt::LeftButton && button != Qt::RightButton)) {
        QListWidget::mousePressEvent(event);
        return;
    }

    if (button == Qt::LeftButton)
        nextExpense();
    else
        selectExpense(currentRow() <= 0 ? count() - 1 : currentRow() - 1);
    qDebug() << "[MOUSE]" << button << "row:" << currentRow();
    event->accept();
}
