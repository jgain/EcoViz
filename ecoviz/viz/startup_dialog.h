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
    LogStreambuf(QTextEdit* textEdit) : m_textEdit(textEdit) {}

protected:
    virtual int_type overflow(int_type v = traits_type::eof()) {
        if (v != traits_type::eof()) {
            m_textEdit->moveCursor(QTextCursor::End);
            m_textEdit->insertPlainText(QString(traits_type::to_char_type(v)));
            m_textEdit->ensureCursorVisible();
        }
        return v;
    }

    virtual std::streamsize xsputn(const char* s, std::streamsize n) {
        m_textEdit->moveCursor(QTextCursor::End);
        m_textEdit->insertPlainText(QString::fromUtf8(s, n));
        m_textEdit->ensureCursorVisible();
        return n;
    }

private:
    QTextEdit* m_textEdit;
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

        m_streambuf = new LogStreambuf(m_logOutput);
        m_originalStreambuf = std::cerr.rdbuf(m_streambuf);
    }

    ~StartupDialog() {
        std::cerr.rdbuf(m_originalStreambuf);
        delete m_streambuf;
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