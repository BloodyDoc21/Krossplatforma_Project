#include "expensewindow.h"

#include <QApplication>

#include <cstring>

int runConsole(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    if (argc > 1 && std::strcmp(argv[1], "--console") == 0)
        return runConsole(argc, argv);

    QApplication app(argc, argv);
    ExpenseWindow window;
    window.show();
    return app.exec();
}
