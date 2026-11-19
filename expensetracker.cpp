#include "expensetracker.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QRegularExpression>
#include <QSaveFile>

#include <limits>
#include <utility>

QString formatAmount(qint64 kopecks)
{
    return QStringLiteral("%1,%2 ₽").arg(kopecks / 100)
        .arg(kopecks % 100, 2, 10, QLatin1Char('0'));
}

ExpenseTracker::ExpenseTracker(QString historyPath, QObject *parent)
    : QObject(parent), historyPath_(std::move(historyPath))
{
}

const QList<Expense> &ExpenseTracker::expenses() const
{
    return expenses_;
}

qint64 ExpenseTracker::total() const
{
    return total_;
}

void ExpenseTracker::loadHistory()
{
    historyLoaded_ = false;
    QFile file(historyPath_);
    if (!file.exists()) {
        expenses_.clear();
        total_ = 0;
        historyLoaded_ = true;
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Не удалось открыть историю: %1").arg(file.errorString()));
        return;
    }

    const auto document = QJsonDocument::fromJson(file.readAll());
    if (!document.isArray()) {
        emit errorOccurred(tr("История расходов повреждена."));
        return;
    }

    QList<Expense> loaded;
    qint64 total = 0;
    for (const auto entry : document.array()) {
        const auto object = entry.toObject();
        const auto kopecks = object.value("kopecks").toInteger();
        const auto title = object.value("title").toString();
        if (kopecks <= 0 || title.trimmed().isEmpty()
            || kopecks > std::numeric_limits<qint64>::max() - total) {
            emit errorOccurred(tr("История содержит некорректный расход."));
            return;
        }
        loaded.append({kopecks, title, object.value("category").toString(),
                       object.value("note").toString()});
        total += kopecks;
    }
    expenses_ = std::move(loaded);
    total_ = total;
    historyLoaded_ = true;
}

void ExpenseTracker::addExpense(const QString &amount, const QString &title,
                                const QString &category, const QString &note)
{
    // Храним деньги в целых копейках, чтобы итог не зависел от округления double.
    static const QRegularExpression amountPattern(
        QStringLiteral(R"(^([0-9]+)(?:[.,]([0-9]{1,2}))?$)"));
    const auto match = amountPattern.match(amount.trimmed());
    bool valid = false;
    const qint64 rubles = match.captured(1).toLongLong(&valid);
    const qint64 cents = match.captured(2).leftJustified(2, QLatin1Char('0')).toInt();
    QLockFile lock(historyPath_ + QStringLiteral(".lock"));
    if (!lock.tryLock()) {
        emit errorOccurred(tr("История занята или недоступна для записи."));
        return;
    }
    // Повторно читаем историю под блокировкой на случай другой копии приложения.
    loadHistory();
    if (!historyLoaded_)
        return;

    const auto remaining = std::numeric_limits<qint64>::max() - total_;
    if (!valid || (rubles == 0 && cents == 0) || cents > remaining
        || rubles > (remaining - cents) / 100) {
        emit errorOccurred(tr("Введите допустимую положительную сумму с точностью до копеек."));
        return;
    }
    if (title.trimmed().isEmpty()) {
        emit errorOccurred(tr("Введите название расхода."));
        return;
    }

    const Expense expense{rubles * 100 + cents, title.trimmed(),
                          category.trimmed(), note.trimmed()};
    auto updated = expenses_;
    updated.append(expense);
    if (!saveHistory(updated))
        return;

    expenses_ = std::move(updated);
    total_ += expense.kopecks;
    lock.unlock();
    emit expenseAdded(expense);
}

bool ExpenseTracker::saveHistory(const QList<Expense> &expenses)
{
    QJsonArray entries;
    for (const auto &expense : expenses) {
        entries.append(QJsonObject{{"kopecks", expense.kopecks}, {"title", expense.title},
                                   {"category", expense.category}, {"note", expense.note}});
    }

    QSaveFile file(historyPath_);
    if (file.open(QIODevice::WriteOnly)) {
        const auto data = QJsonDocument(entries).toJson();
        if (file.write(data) == data.size() && file.commit())
            return true;
    }
    emit errorOccurred(tr("Не удалось записать историю: %1").arg(file.errorString()));
    return false;
}

void ExpenseTracker::showExpenses()
{
    emit expensesReady(expenses_);
}

void ExpenseTracker::showTotal()
{
    emit totalReady(total_);
}

void ExpenseTracker::rejectInput(const QString &message)
{
    emit errorOccurred(message);
}
