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

namespace
{
    class TextEdit : public QPlainTextEdit
    {
      public:
        TextEdit(
            QWidget* const parent,
            QClipboard* const clipboard,
            int const widthHint
        )
            : QPlainTextEdit(parent), m_clipboard(clipboard),
              m_size_hint{widthHint, 0}
        {
        }

        virtual QSize sizeHint() const override
        {
            return m_size_hint;
        }

        auto setTextFromFirstNonEmptyClipboardMode() -> void
        {
            setTextFromClipboardSelectionMode() ||
                setTextFromClipboardClipboardMode();
        }

        auto setTextFromClipboardClipboardMode() -> bool
        {
            return setTextFromClipboardMode(QClipboard::Clipboard);
        }

        auto setTextFromClipboardSelectionMode() -> bool
        {
            return setTextFromClipboardMode(QClipboard::Selection);
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

    template <typename F>
    auto shortcut(QWidget* const parent, Qt::Key const key, F&& func) -> void
    {
        QObject::connect(
            new QShortcut{QKeySequence{key}, parent},
            &QShortcut::activated,
            func
        );
    }
} // namespace

auto main(int argc, char** const argv) -> int
{
    auto app = QApplication{argc, argv};
    app.setCursorFlashTime(0);
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

    auto const text_edit = new TextEdit{
        window.get(), clipboard, 60 * font_metrics.averageCharWidth()
    };
    text_edit->setTextFromFirstNonEmptyClipboardMode();
    text_edit->setContentsMargins(0, 0, 0, 0);
    text_edit->setFrameStyle(QFrame::NoFrame);
    text_edit->setFont(font);
    text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(text_edit, 1, Qt::AlignHCenter);

    // Import
    shortcut(text_edit, Qt::Key_F5, [&text_edit]() {
        text_edit->setTextFromFirstNonEmptyClipboardMode();
    });

    shortcut(text_edit, Qt::Key_F6, [&text_edit]() {
        text_edit->setTextFromClipboardSelectionMode();
    });

    shortcut(text_edit, Qt::Key_F7, [&text_edit]() {
        text_edit->setTextFromClipboardClipboardMode();
    });

    // Say
    shortcut(text_edit, Qt::Key_F1, [&text_edit, &clipboard]() {
        auto text = text_edit->textCursor().selectedText().trimmed();

        if (text.isEmpty())
        {
            text = text_edit->toPlainText().trimmed();
        }

        if (!text.isEmpty())
        {
            clipboard->setText(text, QClipboard::Selection);
            tts();
        }
    });

    // Stop
    shortcut(text_edit, Qt::Key_F2, stop_tts);

    // Export
    shortcut(text_edit, Qt::Key_F9, [&clipboard, &text_edit]() {
        clipboard->setText(text_edit->toPlainText().trimmed());
    });

    return app.exec();
}
