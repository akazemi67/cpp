#ifndef P2PCHAT_IMAGEWIDGET_H
#define P2PCHAT_IMAGEWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QByteArray>
#include <QVBoxLayout>

class ImageWidget : public QWidget {
Q_OBJECT

public:

    ImageWidget(const std::vector<uint8_t>&imageData, QWidget* parent = nullptr)
            : QWidget(parent) {
        QPixmap pixmap;
        pixmap.loadFromData((const uchar*)imageData.data(), imageData.size());

        m_label = new QLabel(this);
        m_label->setPixmap(pixmap);

        auto layout = new QVBoxLayout(this);
        layout->addWidget(m_label);
    }

private:
    QLabel* m_label;
};


#endif
