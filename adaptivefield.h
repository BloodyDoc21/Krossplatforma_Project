#pragma once

#include <QDebug>
#include <QResizeEvent>
#include <QString>

template <typename Widget>
class AdaptiveField : public Widget
{
public:
    AdaptiveField(const QString &name, QWidget *parent, int preferredColumns = 0,
                  int minimumColumns = 0, int preferredHeight = 0, int minimumHeight = 0)
        : Widget(parent), preferredColumns_(preferredColumns), minimumColumns_(minimumColumns),
          preferredHeight_(preferredHeight), minimumHeight_(minimumHeight)
    {
        this->setObjectName(name);
        qDebug() << "[CREATE]" << name << "parent:" << parent->objectName();
    }

    ~AdaptiveField() override
    {
        qDebug() << "[DESTROY]" << this->objectName();
    }

    QSize sizeHint() const override
    {
        return applyHints(Widget::sizeHint(), preferredColumns_, preferredHeight_);
    }

    QSize minimumSizeHint() const override
    {
        return applyHints(Widget::minimumSizeHint(), minimumColumns_, minimumHeight_);
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        Widget::resizeEvent(event);
        qDebug() << "[RESIZE]" << this->objectName()
                 << event->oldSize() << "->" << event->size();
    }

private:
    QSize applyHints(QSize size, int columns, int height) const
    {
        if (columns > 0)
            size.setWidth(columns * this->fontMetrics().horizontalAdvance(QLatin1Char('0')) + 20);
        if (height > 0)
            size.setHeight(height);
        return size;
    }

    int preferredColumns_;
    int minimumColumns_;
    int preferredHeight_;
    int minimumHeight_;
};
