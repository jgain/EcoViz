#ifndef STARTUP_DIALOG_H
#define STARTUP_DIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QTextEdit>
#include <QVBoxLayout>
#include <iostream>

#include <streambuf>
#include <iostream>

class LogStreambuf : public std::streambuf {
public:
    LogStreambuf(QTextEdit* textEdit, std::streambuf* originalStreambuf) : m_textEdit(textEdit), m_originalStreambuf(originalStreambuf) {}

protected:
    virtual int_type overflow(int_type v = traits_type::eof()) {
        if (v != traits_type::eof()) {
            m_textEdit->moveCursor(QTextCursor::End);
            m_textEdit->insertPlainText(QString(traits_type::to_char_type(v)));
            m_textEdit->ensureCursorVisible();
            m_originalStreambuf->sputc(v); // Write to original streambuf
        }
        return v;
    }

    virtual std::streamsize xsputn(const char* s, std::streamsize n) {
        m_textEdit->moveCursor(QTextCursor::End);
        m_textEdit->insertPlainText(QString::fromUtf8(s, n));
        m_textEdit->ensureCursorVisible();
        m_originalStreambuf->sputn(s, n); // Write to original streambuf
        return n;
    }

private:
    QTextEdit* m_textEdit;
    std::streambuf* m_originalStreambuf;
};

class StartupDialog : public QDialog {
    Q_OBJECT

public:
    StartupDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("EcoViz Startup");
        setModal(true);
        resize(800, 600);

        m_progressBar = new QProgressBar(this);
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(0);

        m_logOutput = new QTextEdit(this);
        m_logOutput->setReadOnly(true);
        m_logOutput->setText("Starting Ecoviz, loading the scene....");

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(m_progressBar);
        layout->addWidget(m_logOutput);

        // Stream redirection handled by Window class
        m_streambuf = nullptr;
        m_originalStreambuf = nullptr;
    }

    ~StartupDialog() {
        // Stream redirection handled by Window class
    }

    void setProgress(int value) {
        m_progressBar->setValue(value);
    }

    QTextEdit* getLogOutput() {
        return m_logOutput;
    }

private:
    QProgressBar* m_progressBar;
    QTextEdit* m_logOutput;
    LogStreambuf* m_streambuf;
    std::streambuf* m_originalStreambuf;
};

#endif // STARTUP_DIALOG_H
