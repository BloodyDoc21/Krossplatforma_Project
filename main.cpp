#include "expensetracker.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream input(stdin);
    QTextStream output(stdout);
    ExpenseTracker tracker(QStringLiteral("history.txt"));

    QObject::connect(&tracker, &ExpenseTracker::expenseAdded, &app,
                     [&output](const Expense &expense) {
        output << "Добавлено: " << expense.title << " — " << formatAmount(expense.kopecks) << Qt::endl;
    });
    QObject::connect(&tracker, &ExpenseTracker::expenseAdded, &tracker, &ExpenseTracker::showTotal);
    QObject::connect(&tracker, &ExpenseTracker::expensesReady, &app,
                     [&output](const QList<Expense> &expenses) {
        if (expenses.isEmpty())
            output << "Расходов пока нет." << Qt::endl;
        for (const auto &expense : expenses)
            output << expense.title << " — " << formatAmount(expense.kopecks) << Qt::endl;
    });
    QObject::connect(&tracker, &ExpenseTracker::totalReady, &app, [&output](qint64 total) {
        output << "Итого: " << formatAmount(total) << Qt::endl;
    });
    QObject::connect(&tracker, &ExpenseTracker::errorOccurred, &app, [&output](const QString &error) {
        output << "Ошибка: " << error << Qt::endl;
    });

    tracker.loadHistory();
    output << "Команды: add <сумма> <название>, list, total, exit" << Qt::endl;
    const QRegularExpression addCommand(QStringLiteral(R"(^add\s+(\S+)\s+(.+)$)"));
    for (QString line; input.readLineInto(&line);) {
        line = line.trimmed();
        if (line == QStringLiteral("exit"))
            break;
        if (line == QStringLiteral("list")) {
            tracker.showExpenses();
        } else if (line == QStringLiteral("total")) {
            tracker.showTotal();
        } else {
            const auto match = addCommand.match(line);
            if (match.hasMatch())
                tracker.addExpense(match.captured(1), match.captured(2));
            else
                tracker.rejectInput(QStringLiteral("Неизвестная команда или неверный формат."));
        }
    }
    return 0;
}
