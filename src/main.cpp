#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QFontMetrics>
#include <QMimeData>
#include <QObject>
#include <QPalette>
#include <QPlainTextEdit>
#include <QShortcut>
#include <QSize>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>
#include <memory>
#include <unistd.h>

namespace focus
{
    class TextEdit : public QPlainTextEdit
    {
      public:
        TextEdit(
            QWidget* const parent,
            QClipboard* const clipboard,
            QFont const& font,
            QFontMetrics const& font_metrics
        )
            : QPlainTextEdit(parent), m_clipboard(clipboard),
              m_size_hint{60 * font_metrics.averageCharWidth(), 0}
        {
            setContentsMargins(0, 0, 0, 0);
            setFrameStyle(QFrame::NoFrame);
            setFont(font);
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }

        virtual QSize sizeHint() const override
        {
            return m_size_hint;
        }

        auto setTextFromClipboard() -> void
        {
            setTextFromClipboardMode(QClipboard::Selection) ||
                setTextFromClipboardMode(QClipboard::Clipboard);
        }

      private:
        QClipboard* const m_clipboard;
        QSize m_size_hint;

        auto setTextFromClipboardMode(QClipboard::Mode const mode) -> bool
        {
            auto* const mimeData = m_clipboard->mimeData(mode);

            if (!mimeData->hasText())
            {
                return false;
            }

            auto const text = mimeData->text().trimmed();

            if (text.isEmpty())
            {
                return false;
            }

            clear();
            setPlainText(text);
            return true;
        }
    };

    auto tts() -> void
    {
        if (::fork() == 0)
        {
            execlp("mouth", "mouth", nullptr);
        }
    }

    auto stop_tts() noexcept -> void
    {
        if (::fork() == 0)
        {
            execlp("mouth", "mouth", "stop", nullptr);
        }
    }
} // namespace focus

auto main(int argc, char** const argv) -> int
{
    auto app = QApplication{argc, argv};
    auto const clipboard = app.clipboard();

    auto const window = std::make_unique<QWidget>();
    window->setWindowTitle("Focus");
    auto p = window->palette();
    auto const bg_color = QColor{0x2c3e4c};
    auto const fg_color = QColor{0xdaf7a6};
    p.setColor(QPalette::Base, bg_color);
    p.setColor(QPalette::Highlight, fg_color);
    p.setColor(QPalette::HighlightedText, bg_color);
    p.setColor(QPalette::Text, fg_color);
    p.setColor(QPalette::Window, bg_color);
    p.setColor(QPalette::Window, bg_color);
    window->setPalette(p);
    window->show();

    auto const font = QFont{"Delius", 14, QFont::Bold};
    auto const font_metrics = QFontMetrics{font};
    auto const font_height = font_metrics.height();

    auto const layout = new QVBoxLayout{window.get()};
    layout->setContentsMargins(
        font_height, font_height, font_height, font_height
    );

    auto const text_edit =
        new focus::TextEdit{window.get(), clipboard, font, font_metrics};
    text_edit->setTextFromClipboard();
    layout->addWidget(text_edit, 1, Qt::AlignHCenter);

    // Import
    QObject::connect(
        new QShortcut{QKeySequence{Qt::Key_F5}, text_edit},
        &QShortcut::activated,
        [&text_edit]() {
            text_edit->setTextFromClipboard();
        }
    );

    // Say
    QObject::connect(
        new QShortcut{QKeySequence{Qt::Key_F1}, text_edit},
        &QShortcut::activated,
        [&]() {
            auto text = text_edit->textCursor().selectedText();

            if (text.isEmpty())
            {
                text = text_edit->toPlainText();
            }

            if (!text.isEmpty())
            {
                clipboard->setText(text, QClipboard::Selection);
                focus::tts();
            }
        }
    );

    // Stop
    QObject::connect(
        new QShortcut{QKeySequence{Qt::Key_F2}, text_edit},
        &QShortcut::activated,
        focus::stop_tts
    );

    // Export
    QObject::connect(
        new QShortcut{QKeySequence{Qt::Key_F6}, text_edit},
        &QShortcut::activated,
        [&clipboard, &text_edit]() {
            clipboard->setText(text_edit->toPlainText());
        }
    );

    app.exec();
    return 0;
}
