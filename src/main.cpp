#include <QApplication>
#include <QColor>
#include <QFontMetrics>
#include <QFrame>
#include <QPalette>
#include <QPlainTextEdit>
#include <QSize>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>

namespace focus
{
    class TextEdit : public QPlainTextEdit
    {
      public:
        TextEdit(
            QWidget* const parent,
            QFont const& font,
            QFontMetrics const& font_metrics
        )
            : QPlainTextEdit(parent),
              m_size_hint{60 * font_metrics.averageCharWidth(), 0}
        {
            setContentsMargins(0, 0, 0, 0);
            setFrameStyle(QFrame::NoFrame);
            setFont(font);

            auto p = palette();
            p.setColor(QPalette::Base, Qt::transparent);
            p.setColor(QPalette::Text, Qt::white);
            setPalette(p);
        }

        virtual QSize sizeHint() const override
        {
            return m_size_hint;
        }

      private:
        QSize m_size_hint;
    };
} // namespace focus

auto main(int argc, char** const argv) -> int
{
    auto app = QApplication{argc, argv};

    auto* const window = new QWidget{};
    auto p = window->palette();
    p.setColor(QPalette::Window, QColor{0x2c3e4c});
    window->setPalette(p);

    auto* const layout = new QVBoxLayout{window};
    auto const font = QFont{"Comic Code", 14, QFont::Bold};
    auto const font_metrics = QFontMetrics{font};
    auto const font_height = font_metrics.height();
    layout->setContentsMargins(0, font_height, 0, font_height);

    auto* const txt = new focus::TextEdit{window, font, font_metrics};
    layout->addWidget(txt, 1, Qt::AlignHCenter);

    window->show();
    app.exec();
    return 0;
}
