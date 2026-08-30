#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFrame>
#include <QLabel>
#include <QElapsedTimer>
#include <QMap>
#include <QList>
#include <QStringList>
#include <QPixmap>
#include <QPushButton>

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
class QPropertyAnimation;
class QStackedWidget;

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

class QMovie;

class AnimatedBackdrop : public QWidget
{
    Q_OBJECT

public:
    explicit AnimatedBackdrop(QWidget *parent = nullptr);

    int contentMargin() const { return m_margin; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void paintBlobs(QPainter &painter);
    void paintAurora(QPainter &painter);
    void paintParticles(QPainter &painter);
    void paintStarfall(QPainter &painter);

    QTimer *m_timer;
    qreal m_phase = 0.0;
    QString m_mode;
    int m_margin = 14;
    QMovie *m_movie = nullptr;
    QPixmap m_staticImage;
};

class NetworkBackdrop : public QWidget
{
    Q_OBJECT

public:
    enum class Surface { Card, Panel, Window };

    explicit NetworkBackdrop(Surface surface, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void paintNodes(QPainter &painter);
    void paintStarfall(QPainter &painter);

    QTimer *m_timer;
    qreal m_phase = 0.0;
    bool m_enabled = false;
    QString m_mode;
    Surface m_surface;
};

class ToggleSwitch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal knobPos READ knobPos WRITE setKnobPos)

public:
    explicit ToggleSwitch(QWidget *parent = nullptr);

    bool isChecked() const { return m_checked; }
    void setChecked(bool checked, bool animate = true);

    qreal knobPos() const { return m_knobPos; }
    void setKnobPos(qreal pos);

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    bool m_checked = true;
    qreal m_knobPos = 1.0;
    QPropertyAnimation *m_anim;
};

class AnimatedIconButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(qreal spin READ spin WRITE setSpin)

public:
    AnimatedIconButton(const QString &glyph, const QString &baseColor, const QString &hoverColor, QWidget *parent = nullptr);

    qreal spin() const { return m_spin; }
    void setSpin(qreal degrees);

protected:
    void paintEvent(QPaintEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;

private:
    QString m_glyph;
    QString m_baseColor;
    QString m_hoverColor;
    qreal m_spin = 0.0;
    QPropertyAnimation *m_anim;
};

class AgentAvatar : public QWidget
{
    Q_OBJECT

public:
    explicit AgentAvatar(QWidget *parent = nullptr);

    void setAgentName(const QString &name);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_initials;
    int m_paletteIndex = 0;
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

    bool agentEnabled() const { return m_enabled; }
    void setAgentEnabled(bool enabled);

    bool isCollapsed() const { return m_collapsed; }
    void setCollapsed(bool collapsed);

signals:
    void nameEdited(const QString &id, const QString &newName);
    void settingsRequested(const QString &id);
    void clicked(const QString &id);
    void enabledToggled(const QString &id, bool enabled);

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
    bool m_enabled = true;
    bool m_collapsed = false;

    AgentAvatar *m_avatar;
    QLabel *m_statusDot;
    QLineEdit *m_nameEdit;
    ClickableLabel *m_roleLabel;
    ClickableLabel *m_statusLabel;
    QPushButton *m_settingsButton;
    ToggleSwitch *m_toggleSwitch;
    QVBoxLayout *m_outer;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class AgentStatus { Idle, Active, Busy, Error };
    enum class AgentSettingsResult { Cancelled, Saved, DeleteRequested };

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
    void openAppSettings();

public slots:
    void receiveAgentReply(const QString &agentId, const QString &text);

signals:
    void messageSubmitted(const QString &agentId, const QString &promptName, const QString &text);
    void appCredentialsChanged(const QString &botToken, const QString &geminiApiKey);
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
    void handleSendClicked();

private:
    struct ChatEntry
    {
        QString text;
        bool isUser;
        QString time;
    };

    void buildUi();
    QWidget *buildMainView();
    QWidget *buildSidebar();
    QWidget *buildChatArea();
    QWidget *buildSettingsPage();
    void showSettings();
    void showMainView();
    void toggleAgentListCollapsed();
    void applyCollapsedVisualState();
    void switchTheme(const QString &themeName);
    void rebuildUiLive();
    void performUiRebuild();
    void openThemeCreator(const QString &themeId);
    void openBackgroundPicker();

    void selectMainAgent();
    void selectSubagent(const QString &id);
    void refreshComposerPlaceholder();
    void appendHistory(const QString &agentId, const QString &text, bool isUser);
    void renderMessage(const ChatEntry &entry);
    void redrawActiveHistory();

    AgentSettingsResult runAgentSettingsDialog(const QString &title, const QString &promptAgentName, QString &name, QString &role, bool allowDelete);
    void openMainAgentSettings();
    void openSubagentSettings(const QString &id);
    SubagentCard *createCardWidget(const QString &id, const QString &name);

    Ui::MainWindow *ui;

    QStackedWidget *pageStack;
    QWidget *sidebarPanel;
    QLabel *sidebarTitleLabel;
    QPushButton *collapseListButton;
    QPushButton *settingsEntryButton;
    bool m_agentListCollapsed = false;
    static constexpr int kSidebarExpandedWidth = 280;
    static constexpr int kSidebarCollapsedWidth = 92;

    ClickableFrame *mainStatusCard;
    AgentAvatar *mainAvatar;
    QLabel *agentStatusDot;
    QLineEdit *mainNameEdit;
    QPushButton *mainSettingsButton;

    AgentAvatar *chatHeaderAvatar;
    QLabel *chatHeaderLabel;
    QScrollArea *chatScroll;
    QWidget *chatBubblesHost;
    QVBoxLayout *chatBubblesLayout;
    QPlainTextEdit *messageInput;
    QPushButton *sendButton;

    QScrollArea *subagentsScroll;
    QVBoxLayout *subagentsLayout;
    QLabel *subagentCountLabel;
    QPushButton *addSubagentButton;

    QMap<QString, SubagentCard *> m_cards;
    QMap<QString, QList<ChatEntry>> m_histories;
    QMap<QString, QString> m_agentFolderNames;
    QMap<QString, bool> m_agentEnabled;
    QString m_activeAgentId;
    QString m_mainAgentName;
    QString m_mainAgentRole;
    int m_subagentSeq = 0;
};
#endif
