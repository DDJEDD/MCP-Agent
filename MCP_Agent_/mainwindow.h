#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QList>
#include <QString>
#include <QElapsedTimer>
#include <QPropertyAnimation>
#include <QFrame>
#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include "agents.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLineEdit;

class QPlainTextEdit;
class QTextEdit;
class QScrollArea;
class QVBoxLayout;
class QStackedWidget;
class QTimer;
class QMovie;
class QPainter;

// ---------------------------------------------------------------------------
// Ambient animated background painted behind the whole window.
// ---------------------------------------------------------------------------
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

    QString m_mode;
    QTimer *m_timer = nullptr;
    qreal m_phase = 0.0;
    int m_margin = 14;
    QMovie *m_movie = nullptr;
    QPixmap m_staticImage;
};

// ---------------------------------------------------------------------------
// Panel/card-level animated backdrop (network nodes or starfall), used for
// the sidebar, the chat surface and the chat window itself.
// ---------------------------------------------------------------------------
class NetworkBackdrop : public QWidget
{
    Q_OBJECT
public:
    enum class Surface { Window, Panel, Card };
    explicit NetworkBackdrop(Surface surface, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void paintNodes(QPainter &painter);
    void paintStarfall(QPainter &painter);

    Surface m_surface;
    QString m_mode;
    bool m_enabled = false;
    QTimer *m_timer = nullptr;
    qreal m_phase = 0.0;
};

// ---------------------------------------------------------------------------
// iOS-style animated toggle switch, used for the per-agent enable switch.
// ---------------------------------------------------------------------------
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
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_checked = false;
    qreal m_knobPos = 0.0;
    QPropertyAnimation *m_anim = nullptr;
};

// ---------------------------------------------------------------------------
// Small icon button with a rotate-on-hover flourish.
// ---------------------------------------------------------------------------
class AnimatedIconButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(qreal spin READ spin WRITE setSpin)
public:
    AnimatedIconButton(const QString &glyph, const QString &baseColor, const QString &hoverColor, QWidget *parent = nullptr);
    qreal spin() const { return m_spin; }
    void setSpin(qreal degrees);

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_glyph, m_baseColor, m_hoverColor;
    qreal m_spin = 0.0;
    QPropertyAnimation *m_anim = nullptr;
};

// ---------------------------------------------------------------------------
// Gradient avatar circle with the agent's initials.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Sidebar card representing one subagent (avatar, name, enable toggle,
// settings gear, role badge, status line). Renaming/deleting/enabling are
// surfaced as signals; MainWindow is responsible for talking to the agents
// backend.
// ---------------------------------------------------------------------------
class SubagentCard : public QFrame
{
    Q_OBJECT
public:
    enum class Status { Idle, Active, Busy, Error };

    SubagentCard(const QString &id, const QString &name, QWidget *parent = nullptr);

    QString id() const { return m_id; }
    QString name() const;
    void setName(const QString &name);

    QString role() const { return m_role; }
    void setRole(const QString &role);

    void setStatus(Status status);
    void setAgentEnabled(bool enabled);
    bool agentEnabled() const { return m_enabled; }

    void setCollapsed(bool collapsed);
    void setSelected(bool selected);

signals:
    void clicked(const QString &id);
    void settingsRequested(const QString &id);
    void nameEdited(const QString &id, const QString &newName);
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

    AgentAvatar *m_avatar = nullptr;
    QLabel *m_statusDot = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    ToggleSwitch *m_toggleSwitch = nullptr;
    AnimatedIconButton *m_settingsButton = nullptr;
    ClickableLabel *m_roleLabel = nullptr;
    ClickableLabel *m_statusLabel = nullptr;
    QVBoxLayout *m_outer = nullptr;
};

// ---------------------------------------------------------------------------
// MainWindow
// ---------------------------------------------------------------------------
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
    void refreshAgentsFromDisk();
    void setMainAgentName(const QString &name);
    void setMainAgentRole(const QString &role);

    QString addSubagent(const QString &name = QString(), const QString &role = QString());
    void removeSubagent(const QString &id);

    void setSubagentStatus(const QString &id, AgentStatus status);
    void setSubagentName(const QString &id, const QString &name);
    void setSubagentRole(const QString &id, const QString &role);

    QString subagentName(const QString &id) const;
    QString subagentRole(const QString &id) const;
    bool hasSubagent(const QString &id) const;
    QStringList subagentIds() const;

    void receiveAgentReply(const QString &agentId, const QString &text);

signals:
    // agentId is the GUI id ("" for the main agent); backendName is the
    // actual agent-folder name understood by the `agents` manager.
    void messageSubmitted(const QString &agentId, const QString &backendName, const QString &text);
    void addSubagentRequested();
    void subagentRemoveRequested(const QString &id);
    void subagentRenamed(const QString &id, const QString &newName);
    void subagentRoleChanged(const QString &id, const QString &newRole);
    void mainAgentSelected();
    void subagentSelected(const QString &id);
    void mainAgentRenamed(const QString &newName);
    void mainAgentRoleChanged(const QString &newRole);
    void appCredentialsChanged(const QString &botToken, const QString &geminiApiKey);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct ChatEntry { QString text; bool isUser; QString time; };
    enum class AgentSettingsResult { Cancelled, Saved, DeleteRequested };

    // ----- UI construction (interface layer, from the "pretty" version) -----
    void buildUi();
    void rebuildUiLive();
    void performUiRebuild();
    QWidget *buildMainView();
    QWidget *buildSidebar();
    QWidget *buildChatArea();
    QWidget *buildSettingsPage();
    SubagentCard *createCardWidget(const QString &id, const QString &name);
    void applyCollapsedVisualState();
    void toggleAgentListCollapsed();
    void switchTheme(const QString &themeName);
    void openThemeCreator(const QString &themeId);
    void openBackgroundPicker();
    void openAppSettings();
    void showSettings();
    void showMainView();
    AnimatedIconButton *refreshAgentsButton = nullptr;
    // ----- Chat rendering -----
    void appendHistory(const QString &agentId, const QString &text, bool isUser);
    void renderMessage(const ChatEntry &entry);
    void redrawActiveHistory();
    void refreshComposerPlaceholder();

    // ----- Agent selection -----
    void selectMainAgent();
    void selectSubagent(const QString &id);

    // ----- Settings dialog (UI from file 1, persistence via agents manager) --
    AgentSettingsResult runAgentSettingsDialog(const QString &title, const QString &backendAgentName,
                                               QString &name, QString &role, bool allowDelete);
    void openMainAgentSettings();
    void openSubagentSettings(const QString &id);

    // ----- Persistence / backend logic (from the "logic" version) -----------
    void loadAgentsFromDisk();
    void syncAgentName(const QString &backendName, const QString &newName);

private slots:
    void handleSendClicked();
    void updateUptime();

private:
    Ui::MainWindow *ui;
    agents *m_agentManager = nullptr;

    static constexpr int kSidebarExpandedWidth = 260;
    static constexpr int kSidebarCollapsedWidth = 64;

    QString m_mainAgentName;
    QString m_mainAgentRole;
    QString m_activeAgentId;
    int m_subagentSeq = 0;
    bool m_agentListCollapsed = false;

    QMap<QString, SubagentCard *> m_cards;
    QMap<QString, QList<ChatEntry>> m_histories;
    QMap<QString, QString> m_agentFolderNames; // GUI id -> backend agent folder name ("" key = main agent)
    QMap<QString, bool> m_agentEnabled;

    QElapsedTimer startTime;
    QTimer *uptimeTimer = nullptr;

    // Page navigation
    QStackedWidget *pageStack = nullptr;
    QWidget *sidebarPanel = nullptr;
    QLabel *sidebarTitleLabel = nullptr;

    ClickableFrame *mainStatusCard = nullptr;
    AgentAvatar *mainAvatar = nullptr;
    QLabel *agentStatusDot = nullptr;
    QLineEdit *mainNameEdit = nullptr;
    AnimatedIconButton *mainSettingsButton = nullptr;
    ClickableLabel *mainRoleLabel = nullptr;
    ClickableLabel *uptimeLabel = nullptr;

    QLabel *subagentCountLabel = nullptr;
    QPushButton *addSubagentButton = nullptr;
    AnimatedIconButton *collapseListButton = nullptr;
    AnimatedIconButton *settingsEntryButton = nullptr;
    QScrollArea *subagentsScroll = nullptr;
    QVBoxLayout *subagentsLayout = nullptr;

    AgentAvatar *chatHeaderAvatar = nullptr;
    QLabel *chatHeaderLabel = nullptr;
    QScrollArea *chatScroll = nullptr;
    QWidget *chatBubblesHost = nullptr;
    QVBoxLayout *chatBubblesLayout = nullptr;

    QPlainTextEdit *messageInput = nullptr;
    QPushButton *sendButton = nullptr;
};

#endif // MAINWINDOW_H
