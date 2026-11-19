#pragma once

#include <QListWidget>

class ExpenseList : public QListWidget
{
    Q_OBJECT

public:
    explicit ExpenseList(QWidget *parent = nullptr);
    ~ExpenseList() override;

    void nextExpense();
    void randomExpense();
    void blinkHighlight();
    void restoreHighlight();
    void selectExpense(int row);

signals:
    void expenseSelected(const QString &text);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};
