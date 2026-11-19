#include "expensewindow.h"

#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
template <typename Widget>
class TrackedWidget : public Widget
{
public:
    TrackedWidget(const QString &name, QWidget *parent) : Widget(parent)
    {
        this->setObjectName(name);
        qDebug() << "[CREATE]" << name << "parent:" << parent->objectName();
    }

    ~TrackedWidget() override
    {
        qDebug() << "[DESTROY]" << this->objectName();
    }
};
}

ExpenseWindow::ExpenseWindow(QWidget *parent)
    : QWidget(parent),
      tracker_(new ExpenseTracker(QStringLiteral("history.txt"), this)),
      rootLayout_(new QVBoxLayout(this))
{
    setObjectName(QStringLiteral("ExpenseWindow"));
    setWindowTitle(tr("Трекер расходов"));
    resize(560, 480);

    setupEntryForm();

    list_ = new TrackedWidget<QListWidget>(QStringLiteral("expenses"), this);
    rootLayout_->addWidget(list_, 1);
    setupSummaryPanel();

    status_ = new TrackedWidget<QLabel>(QStringLiteral("status"), this);
    status_->setWordWrap(true);
    rootLayout_->addWidget(status_);

    connect(tracker_, &ExpenseTracker::expenseAdded, this, &ExpenseWindow::refreshExpenses);
    connect(tracker_, &ExpenseTracker::expenseAdded, this, [this](const Expense &expense) {
        title_->clear();
        amount_->clear();
        title_->setFocus();
        status_->setText(tr("Записано: %1").arg(expense.title));
    });
    connect(tracker_, &ExpenseTracker::errorOccurred, status_, &QLabel::setText);

    tracker_->loadHistory();
    refreshExpenses();
}

ExpenseWindow::~ExpenseWindow()
{
    qDebug() << "[DESTROY]" << objectName();
}

void ExpenseWindow::setupEntryForm()
{
    auto *entry = new TrackedWidget<QGroupBox>(QStringLiteral("entry"), this);
    entry->setTitle(tr("Новый расход"));
    auto *row = new QHBoxLayout(entry);
    title_ = new TrackedWidget<QLineEdit>(QStringLiteral("expenseTitle"), entry);
    title_->setPlaceholderText(tr("Название"));
    amount_ = new TrackedWidget<QLineEdit>(QStringLiteral("expenseAmount"), entry);
    amount_->setPlaceholderText(tr("Сумма, ₽"));
    auto *submit = new TrackedWidget<QPushButton>(QStringLiteral("addExpense"), entry);
    submit->setText(tr("Добавить"));
    row->addWidget(title_);
    row->addWidget(amount_);
    row->addWidget(submit);
    rootLayout_->addWidget(entry);

    connect(submit, &QPushButton::clicked, this, &ExpenseWindow::submitExpense);
    connect(amount_, &QLineEdit::returnPressed, this, &ExpenseWindow::submitExpense);
}

void ExpenseWindow::setupSummaryPanel()
{
    auto *panel = new TrackedWidget<QGroupBox>(QStringLiteral("summaryPanel"), this);
    panel->setTitle(tr("Сводка расходов"));
    summary_ = panel;
    summaryLayout_ = new QVBoxLayout(panel);
    countLabel_ = new TrackedWidget<QLabel>(QStringLiteral("expenseCount"), panel);
    totalLabel_ = new TrackedWidget<QLabel>(QStringLiteral("expenseTotal"), panel);
    summaryLayout_->addWidget(countLabel_);
    summaryLayout_->addWidget(totalLabel_);
    rootLayout_->addWidget(panel);

    auto *controls = new QHBoxLayout;
    rootLayout_->addLayout(controls);
    moveButton_ = new TrackedWidget<QPushButton>(QStringLiteral("moveTotal"), this);
    moveButton_->setText(tr("Переместить итог"));
    removeButton_ = new TrackedWidget<QPushButton>(QStringLiteral("removeSummary"), this);
    removeButton_->setText(tr("Удалить сводку"));
    auto *detach = new TrackedWidget<QPushButton>(QStringLiteral("detachDemo"), this);
    detach->setText(tr("Демо setParent(nullptr)"));
    controls->addWidget(moveButton_);
    controls->addWidget(removeButton_);
    controls->addWidget(detach);
    controls->addStretch();

    connect(moveButton_, &QPushButton::clicked, this, &ExpenseWindow::moveTotal);
    connect(removeButton_, &QPushButton::clicked, this, &ExpenseWindow::removeSummary);
    connect(detach, &QPushButton::clicked, this, &ExpenseWindow::demonstrateDetach);
}

void ExpenseWindow::submitExpense()
{
    tracker_->addExpense(amount_->text(), title_->text());
}

void ExpenseWindow::refreshExpenses()
{
    list_->clear();
    for (const auto &expense : tracker_->expenses())
        list_->addItem(tr("%1 — %2").arg(expense.title, formatAmount(expense.kopecks)));
    if (totalLabel_)
        totalLabel_->setText(tr("Итого: %1").arg(formatAmount(tracker_->total())));
    if (countLabel_)
        countLabel_->setText(tr("Записей: %1").arg(tracker_->expenses().size()));
}

void ExpenseWindow::moveTotal()
{
    if (!summary_ || !totalLabel_)
        return;

    const bool insidePanel = totalLabel_->parentWidget() == summary_;
    auto *source = insidePanel ? summaryLayout_.data() : rootLayout_;
    auto *target = insidePanel ? rootLayout_ : summaryLayout_.data();
    source->removeWidget(totalLabel_);
    totalLabel_->setParent(insidePanel ? this : summary_.data());
    if (insidePanel)
        target->insertWidget(target->indexOf(summary_) + 1, totalLabel_);
    else
        target->addWidget(totalLabel_);
    totalLabel_->show();
    qDebug() << "[SET_PARENT]" << totalLabel_->objectName()
             << "parent:" << totalLabel_->parentWidget()->objectName();
}

void ExpenseWindow::removeSummary()
{
    if (!summary_)
        return;
    moveButton_->setEnabled(false);
    removeButton_->setEnabled(false);
    qDebug() << "[DELETE_LATER]" << summary_->objectName();
    summary_->deleteLater();
}

void ExpenseWindow::demonstrateDetach()
{
    auto *owner = new TrackedWidget<QWidget>(QStringLiteral("temporarySummary"), this);
    new TrackedWidget<QLabel>(QStringLiteral("ownedPreview"), owner);
    auto *detached = new TrackedWidget<QLabel>(QStringLiteral("detachedPreview"), owner);

    qDebug() << "[DEMO] children before detach:" << owner->children().size();
    detached->setParent(nullptr);
    qDebug() << "[DEMO] children after detach:" << owner->children().size();
    owner->deleteLater();
    detached->deleteLater();
}
