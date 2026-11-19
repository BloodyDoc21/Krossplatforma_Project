#pragma once

#include "expensetracker.h"

#include <QPointer>
#include <QWidget>

class ExpenseList;
class QComboBox;
class QFormLayout;
class QPlainTextEdit;
class QLabel;
class QLineEdit;
class QHBoxLayout;
class QPushButton;
class QTimer;
class QVBoxLayout;

class ExpenseWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ExpenseWindow(QWidget *parent = nullptr);
    ~ExpenseWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void submitExpense();
    void refreshExpenses();
    void moveTotal();
    void removeSummary();
    void demonstrateDetach();
    void cycleReviewMode();
    void addShortcut();
    void clearShortcuts();

private:
    void setupEntryForm();
    void clearEntryForm();
    void setCompactLayout(bool compact);
    void dumpFieldPolicies();
    void dumpFieldSizes();
    void setupSummaryPanel();
    void setupReview();
    void updateIntervalLabel();
    void connectTimerHandler();
    void connectSelectionHandler();

    enum class ReviewMode { Cycle, Random, Blink };

    ExpenseTracker *tracker_;
    QVBoxLayout *rootLayout_;
    QLineEdit *title_;
    QLineEdit *amount_;
    QComboBox *category_;
    QPlainTextEdit *note_;
    QFormLayout *formLayout_;
    QHBoxLayout *amountRow_;
    ExpenseList *list_;
    QLabel *status_;
    QPointer<QWidget> summary_;
    QPointer<QVBoxLayout> summaryLayout_;
    QPointer<QLabel> totalLabel_;
    QPointer<QLabel> countLabel_;
    QPushButton *moveButton_;
    QPushButton *removeButton_;
    QTimer *timer_;
    QPushButton *timerButton_;
    QLabel *intervalLabel_;
    QLabel *modeLabel_;
    QHBoxLayout *shortcutsLayout_;
    QList<QPointer<QPushButton>> shortcuts_;
    ReviewMode reviewMode_ = ReviewMode::Cycle;
    bool selectionInTitle_ = false;
    QMetaObject::Connection timerConnection_;
    QMetaObject::Connection selectionConnection_;
};
