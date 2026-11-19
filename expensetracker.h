#pragma once

#include <QList>
#include <QObject>
#include <QString>

struct Expense
{
    qint64 kopecks;
    QString title;
    QString category;
    QString note;
};

QString formatAmount(qint64 kopecks);

class ExpenseTracker : public QObject
{
    Q_OBJECT

public:
    explicit ExpenseTracker(QString historyPath, QObject *parent = nullptr);
    const QList<Expense> &expenses() const;
    qint64 total() const;

public slots:
    void loadHistory();
    void addExpense(const QString &amount, const QString &title,
                    const QString &category = {}, const QString &note = {});
    void showExpenses();
    void showTotal();
    void rejectInput(const QString &message);

signals:
    void expenseAdded(const Expense &expense);
    void expensesReady(const QList<Expense> &expenses);
    void totalReady(qint64 kopecks);
    void errorOccurred(const QString &message);

private:
    bool saveHistory(const QList<Expense> &expenses);
    QString historyPath_;
    QList<Expense> expenses_;
    qint64 total_ = 0;
    bool historyLoaded_ = false;
};
