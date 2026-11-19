#include "expensewindow.h"
#include "expenselist.h"

#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
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
      rootLayout_(new QVBoxLayout(this)),
      timer_(new QTimer(this))
{
    setObjectName(QStringLiteral("ExpenseWindow"));
    setWindowTitle(tr("Трекер расходов"));
    resize(690, 540);

    setupEntryForm();

    list_ = new ExpenseList(this);
    rootLayout_->addWidget(list_, 1);
    setupSummaryPanel();

    status_ = new TrackedWidget<QLabel>(QStringLiteral("status"), this);
    status_->setWordWrap(true);
    rootLayout_->addWidget(status_);
    setupReview();

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
    const int previous = list_->currentRow();
    list_->clear();
    for (const auto &expense : tracker_->expenses())
        list_->addItem(tr("%1 — %2").arg(expense.title, formatAmount(expense.kopecks)));
    list_->selectExpense(previous);
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

void ExpenseWindow::setupReview()
{
    auto *timerRow = new QHBoxLayout;
    const auto addButton = [this, timerRow](const QString &name, const QString &text) {
        auto *button = new TrackedWidget<QPushButton>(name, this);
        button->setText(text);
        timerRow->addWidget(button);
        return button;
    };

    timerButton_ = addButton(QStringLiteral("toggleTimer"), tr("Старт"));
    auto *slower = addButton(QStringLiteral("slower"), tr("Медленнее"));
    auto *faster = addButton(QStringLiteral("faster"), tr("Быстрее"));
    auto *mode = addButton(QStringLiteral("cycleMode"), tr("Режим"));
    intervalLabel_ = new TrackedWidget<QLabel>(QStringLiteral("timerInterval"), this);
    modeLabel_ = new TrackedWidget<QLabel>(QStringLiteral("timerMode"), this);
    timerRow->addWidget(intervalLabel_);
    timerRow->addWidget(modeLabel_);
    timerRow->addStretch();
    rootLayout_->addLayout(timerRow);

    auto *shortcutControls = new QHBoxLayout;
    auto *addShortcut = new QPushButton(tr("+ Быстрый выбор"), this);
    addShortcut->setObjectName(QStringLiteral("addShortcut"));
    auto *clear = new QPushButton(tr("Очистить кнопки"), this);
    clear->setObjectName(QStringLiteral("clearShortcuts"));
    auto *route = new QPushButton(tr("Сменить обработчик выбора"), this);
    route->setObjectName(QStringLiteral("switchSelectionHandler"));
    shortcutControls->addWidget(addShortcut);
    shortcutControls->addWidget(clear);
    shortcutControls->addWidget(route);
    shortcutControls->addStretch();
    rootLayout_->addLayout(shortcutControls);

    auto *scroll = new QScrollArea(this);
    auto *bar = new QWidget(scroll);
    bar->setObjectName(QStringLiteral("shortcutBar"));
    shortcutsLayout_ = new QHBoxLayout(bar);
    shortcutsLayout_->setContentsMargins(0, 0, 0, 0);
    shortcutsLayout_->addStretch();
    scroll->setWidget(bar);
    scroll->setWidgetResizable(true);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFixedHeight(addShortcut->sizeHint().height()
                           + 2 * scroll->frameWidth()
                           + scroll->horizontalScrollBar()->sizeHint().height());
    rootLayout_->addWidget(scroll);

    timer_->setInterval(600);
    connect(timerButton_, &QPushButton::clicked, this, [this] {
        if (timer_->isActive()) {
            timer_->stop();
            list_->restoreHighlight();
        } else {
            timer_->start();
        }
        timerButton_->setText(timer_->isActive() ? tr("Стоп") : tr("Старт"));
        qDebug() << "[TIMER] active:" << timer_->isActive();
    });
    const auto changeInterval = [this](int step) {
        timer_->setInterval(qBound(200, timer_->interval() + step, 5000));
        updateIntervalLabel();
        qDebug() << "[TIMER] interval:" << timer_->interval();
    };
    connect(slower, &QPushButton::clicked, this, [changeInterval] { changeInterval(200); });
    connect(faster, &QPushButton::clicked, this, [changeInterval] { changeInterval(-200); });
    connect(mode, &QPushButton::clicked, this, &ExpenseWindow::cycleReviewMode);
    connect(addShortcut, &QPushButton::clicked, this, &ExpenseWindow::addShortcut);
    connect(clear, &QPushButton::clicked, this, &ExpenseWindow::clearShortcuts);
    connect(route, &QPushButton::clicked, this, [this] {
        selectionInTitle_ = !selectionInTitle_;
        connectSelectionHandler();
    });

    updateIntervalLabel();
    connectTimerHandler();
    connectSelectionHandler();
}

void ExpenseWindow::updateIntervalLabel()
{
    intervalLabel_->setText(tr("%1 мс").arg(timer_->interval()));
}

void ExpenseWindow::cycleReviewMode()
{
    list_->restoreHighlight();
    switch (reviewMode_) {
    case ReviewMode::Cycle: reviewMode_ = ReviewMode::Random; break;
    case ReviewMode::Random: reviewMode_ = ReviewMode::Blink; break;
    case ReviewMode::Blink: reviewMode_ = ReviewMode::Cycle; break;
    }
    connectTimerHandler();
}

void ExpenseWindow::connectTimerHandler()
{
    // У таймера один активный обработчик, даже при смене режима на ходу.
    disconnect(timerConnection_);
    const auto connectMode = [this](const QString &name, auto action) {
        modeLabel_->setText(name);
        qDebug() << "[TIMER] mode:" << name;
        return connect(timer_, &QTimer::timeout, this, [this, name, action] {
            (list_->*action)();
            qDebug() << "[TIMER] timeout:" << name << "row:" << list_->currentRow();
        });
    };

    switch (reviewMode_) {
    case ReviewMode::Cycle:
        timerConnection_ = connectMode(tr("Цикл"), &ExpenseList::nextExpense);
        break;
    case ReviewMode::Random:
        timerConnection_ = connectMode(tr("Случайно"), &ExpenseList::randomExpense);
        break;
    case ReviewMode::Blink:
        timerConnection_ = connectMode(tr("Мигание"), &ExpenseList::blinkHighlight);
        break;
    }
}

void ExpenseWindow::connectSelectionHandler()
{
    disconnect(selectionConnection_);
    setWindowTitle(tr("Трекер расходов"));
    if (selectionInTitle_) {
        selectionConnection_ = connect(list_, &ExpenseList::expenseSelected, this,
                                       [this](const QString &text) {
            setWindowTitle(tr("Трекер расходов — %1").arg(text));
        });
        status_->setText(tr("Выбор отображается в заголовке окна."));
    } else {
        selectionConnection_ = connect(list_, &ExpenseList::expenseSelected, status_, &QLabel::setText);
        status_->setText(tr("Выбор отображается в строке состояния."));
    }
    qDebug() << "[LIST] reconnected selection handler, title:" << selectionInTitle_;
}

void ExpenseWindow::addShortcut()
{
    const auto *item = list_->currentItem();
    if (!item) {
        status_->setText(tr("Сначала выберите расход в списке."));
        return;
    }

    const int row = list_->currentRow();
    auto *bar = shortcutsLayout_->parentWidget();
    auto *button = new TrackedWidget<QPushButton>(QStringLiteral("expenseShortcut"), bar);
    const QString text = item->text();
    button->setText(tr("Добавлено"));
    button->setToolTip(text);
    shortcutsLayout_->insertWidget(shortcutsLayout_->count() - 1, button);
    shortcuts_.append(button);
    connect(button, &QPushButton::clicked, this, [this, row] {
        list_->selectExpense(row);
        qDebug() << "[DYNAMIC] shortcut clicked, row:" << row;
    });
    QTimer::singleShot(600, button, [button, text] {
        button->setText(button->fontMetrics().elidedText(text, Qt::ElideRight, 220));
    });
    qDebug() << "[DYNAMIC] shortcut connected, row:" << row;
}

void ExpenseWindow::clearShortcuts()
{
    for (const auto &button : shortcuts_) {
        if (button) {
            button->hide();
            shortcutsLayout_->removeWidget(button);
            button->deleteLater();
        }
    }
    shortcuts_.clear();
    qDebug() << "[DYNAMIC] shortcuts cleared";
}
