QT = core gui widgets
CONFIG += console c++17 warn_on
CONFIG -= app_bundle
TARGET = ExpenseTracker
SOURCES += main.cpp console.cpp expensetracker.cpp expensewindow.cpp
HEADERS += expensetracker.h expensewindow.h
