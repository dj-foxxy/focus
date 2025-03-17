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
#include <QtSpell.hpp>
#include <memory>
#include <unistd.h>

namespace
{
    class TextEdit : public QPlainTextEdit
    {
      public:
        TextEdit(QWidget* const parent, int const widthHint)
            : QPlainTextEdit(parent), m_size_hint{widthHint, 0}
        {
        }

        virtual QSize sizeHint() const override
        {
            return m_size_hint;
        }

      private:
        QSize m_size_hint;
    };

    class ClipboardActions
    {
      public:
        ClipboardActions(TextEdit* const textEdit, QClipboard* const clipboard)
            : m_textEdit{textEdit}, m_clipboard{clipboard}
        {
        }

        auto setTextFromFirstNonEmptyClipboardMode() const -> void
        {
            setTextFromClipboardSelectionMode() ||
                setTextFromClipboardClipboardMode();
        }

        auto setTextFromClipboardClipboardMode() const -> bool
        {
            return setTextFromClipboardMode(QClipboard::Clipboard);
        }

        auto setTextFromClipboardSelectionMode() const -> bool
        {
            return setTextFromClipboardMode(QClipboard::Selection);
        }

      private:
        TextEdit* const m_textEdit;
        QClipboard* const m_clipboard;

        auto setTextFromClipboardMode(QClipboard::Mode const mode) const -> bool
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

            m_textEdit->clear();
            m_textEdit->setPlainText(text);
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
    auto const fontMetric = QFontMetrics{font};
    auto const fontHeight = fontMetric.height();

    auto const layout = new QVBoxLayout{window.get()};
    layout->setContentsMargins(fontHeight, fontHeight, fontHeight, fontHeight);

    auto const textEdit =
        new TextEdit{window.get(), 60 * fontMetric.averageCharWidth()};
    textEdit->setContentsMargins(0, 0, 0, 0);
    textEdit->setFrameStyle(QFrame::NoFrame);
    textEdit->setFont(font);
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(textEdit, 1, Qt::AlignHCenter);
    auto const clipboardActions = ClipboardActions{textEdit, clipboard};
    clipboardActions.setTextFromFirstNonEmptyClipboardMode();

    auto checker = QtSpell::TextEditChecker{};
    checker.setLanguage("en_GB");
    checker.setTextEdit(textEdit);

    // Import
    shortcut(textEdit, Qt::Key_F5, [&clipboardActions]() {
        clipboardActions.setTextFromFirstNonEmptyClipboardMode();
    });

    shortcut(textEdit, Qt::Key_F6, [&clipboardActions]() {
        clipboardActions.setTextFromClipboardSelectionMode();
    });

    shortcut(textEdit, Qt::Key_F7, [&clipboardActions]() {
        clipboardActions.setTextFromClipboardClipboardMode();
    });

    // Say
    shortcut(textEdit, Qt::Key_F1, [&textEdit, &clipboard]() {
        auto text = textEdit->textCursor().selectedText().trimmed();

        if (text.isEmpty())
        {
            text = textEdit->toPlainText().trimmed();
        }

        if (!text.isEmpty())
        {
            clipboard->setText(text, QClipboard::Selection);
            tts();
        }
    });

    // Stop
    shortcut(textEdit, Qt::Key_F2, stop_tts);

    // Export
    shortcut(textEdit, Qt::Key_F9, [&clipboard, &textEdit]() {
        clipboard->setText(textEdit->toPlainText().trimmed());
    });

    return app.exec();
}
