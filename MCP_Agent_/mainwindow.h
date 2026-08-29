#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFrame>
#include <QLabel>
#include <QElapsedTimer>
#include <QMap>
#include <QList>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTimer;
class QLineEdit;
class QTextEdit;
class QPlainTextEdit;
class QPushButton;
class QVBoxLayout;
class QScrollArea;

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget *parent = nullptr);
    explicit ClickableLabel(const QString &text, QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

class ClickableFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ClickableFrame(QWidget *parent = nullptr);

    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;

private:
    void applyStyle();

    bool m_selected = false;
    bool m_hovered = false;
};

class SubagentCard : public QFrame
{
    Q_OBJECT

public:
    enum class Status { Idle, Active, Busy, Error };

    explicit SubagentCard(const QString &id, const QString &name, QWidget *parent = nullptr);

    QString id() const { return m_id; }

    QString name() const;
    void setName(const QString &name);

    QString role() const { return m_role; }
    void setRole(const QString &role);

    Status status() const { return m_status; }
    void setStatus(Status status);

    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }

signals:
    void nameEdited(const QString &id, const QString &newName);
    void removeRequested(const QString &id);
    void settingsRequested(const QString &id);
    void clicked(const QString &id);

protected:
    void mousePressEvent(QMouseEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;

private:
    void applyCardStyle();
    void applyStatusStyle();
    void refreshRolePreview();

    QString m_id;
    QString m_role;
    Status m_status = Status::Idle;
    bool m_selected = false;
    bool m_hovered = false;

    QLabel *m_statusDot;
    QLineEdit *m_nameEdit;
    ClickableLabel *m_roleLabel;
    ClickableLabel *m_statusLabel;
    QPushButton *m_settingsButton;
    QPushButton *m_removeButton;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class AgentStatus { Idle, Active, Busy, Error };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void appendMainMessage(const QString &text, bool isUser);
    void appendSubagentMessage(const QString &id, const QString &text, bool isUser);
    void setAgentActive(bool active);

    void setMainAgentName(const QString &name);
    QString mainAgentName() const { return m_mainAgentName; }
    void setMainAgentRole(const QString &role);
    QString mainAgentRole() const { return m_mainAgentRole; }

    QString addSubagent(const QString &name = QString(), const QString &role = QString());
    void removeSubagent(const QString &id);
    void setSubagentStatus(const QString &id, AgentStatus status);
    void setSubagentName(const QString &id, const QString &name);
    void setSubagentRole(const QString &id, const QString &role);
    QString subagentName(const QString &id) const;
    QString subagentRole(const QString &id) const;
    bool hasSubagent(const QString &id) const;
    QStringList subagentIds() const;

signals:
    void messageSubmitted(const QString &agentId, const QString &text);
    void addSubagentRequested();
    void subagentSelected(const QString &id);
    void mainAgentSelected();
    void subagentRemoveRequested(const QString &id);
    void subagentRenamed(const QString &id, const QString &newName);
    void subagentRoleChanged(const QString &id, const QString &newRole);
    void mainAgentRenamed(const QString &newName);
    void mainAgentRoleChanged(const QString &newRole);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void updateUptime();
    void handleSendClicked();

private:
    struct ChatEntry
    {
        QString text;
        bool isUser;
        QString time;
    };

    void buildUi();
    QWidget *buildLeftPanel();
    QWidget *buildRightPanel();

    void selectMainAgent();
    void selectSubagent(const QString &id);
    void refreshComposerPlaceholder();
    void appendHistory(const QString &agentId, const QString &text, bool isUser);
    void renderMessage(const ChatEntry &entry);
    void redrawActiveHistory();

    bool runAgentSettingsDialog(const QString &title, QString &name, QString &role);
    void openMainAgentSettings();
    void openSubagentSettings(const QString &id);

    Ui::MainWindow *ui;

    QElapsedTimer startTime;
    QTimer *uptimeTimer;

    ClickableFrame *mainStatusCard;
    QLabel *agentStatusDot;
    QLineEdit *mainNameEdit;
    ClickableLabel *agentStatusText;
    ClickableLabel *uptimeLabel;
    ClickableLabel *startedAtLabel;
    QPushButton *mainSettingsButton;

    QLabel *chatHeaderLabel;
    QTextEdit *chatLog;
    QPlainTextEdit *messageInput;
    QPushButton *sendButton;

    QScrollArea *subagentsScroll;
    QVBoxLayout *subagentsLayout;
    QLabel *subagentCountLabel;
    QPushButton *addSubagentButton;

    QMap<QString, SubagentCard *> m_cards;
    QMap<QString, QList<ChatEntry>> m_histories;
    QString m_activeAgentId;
    QString m_mainAgentName;
    QString m_mainAgentRole;
    int m_subagentSeq = 0;
};
#endif
