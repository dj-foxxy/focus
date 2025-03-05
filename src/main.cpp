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
        explicit TextEdit(QWidget* const parent) : QPlainTextEdit(parent)
        {
            setContentsMargins(0, 0, 0, 0);

            // Font
            {
                auto const font = QFont{"Comic Code", 14, QFont::Bold};
                setFont(font);

                auto const font_metric = QFontMetrics{font};
                m_size_hint = QSize{60 * font_metric.averageCharWidth(), 0};
            }

            setFrameStyle(QFrame::NoFrame);

            // Palette
            {
                auto p = palette();
                p.setColor(QPalette::Base, Qt::transparent);
                p.setColor(QPalette::Text, Qt::white);
                setPalette(p);
            }
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
    {
        auto const font = QFont{"Comic Code", 14, QFont::Bold};
        auto const font_metric = QFontMetrics{font};
        layout->setContentsMargins(
            0, font_metric.height(), 0, font_metric.height()
        );
    }
    auto* const txt = new focus::TextEdit{window};
    layout->addWidget(txt, 1, Qt::AlignHCenter);
    window->show();
    app.exec();
}
