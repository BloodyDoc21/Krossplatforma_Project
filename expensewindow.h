#pragma once

#include "expensetracker.h"

#include <QPointer>
#include <QWidget>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QVBoxLayout;

class ExpenseWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ExpenseWindow(QWidget *parent = nullptr);
    ~ExpenseWindow() override;

private slots:
    void submitExpense();
    void refreshExpenses();
    void moveTotal();
    void removeSummary();
    void demonstrateDetach();

private:
    void setupEntryForm();
    void setupSummaryPanel();

    ExpenseTracker *tracker_;
    QVBoxLayout *rootLayout_;
    QLineEdit *title_;
    QLineEdit *amount_;
    QListWidget *list_;
    QLabel *status_;
    QPointer<QWidget> summary_;
    QPointer<QVBoxLayout> summaryLayout_;
    QPointer<QLabel> totalLabel_;
    QPointer<QLabel> countLabel_;
    QPushButton *moveButton_;
    QPushButton *removeButton_;
};
