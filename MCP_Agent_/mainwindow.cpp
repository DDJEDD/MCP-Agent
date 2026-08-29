#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "template_agents.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QDateTime>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QDir>
#include <utility>

namespace {
constexpr const char *kBgWindow    = "#11111b";
constexpr const char *kBgPanel     = "#181825";
constexpr const char *kBgCard      = "#1e1e2e";
constexpr const char *kBgInput     = "#1e1e2e";
constexpr const char *kBgCardHover = "#232336";
constexpr const char *kBorder      = "#313244";
constexpr const char *kBorderSel   = "#89b4fa";
constexpr const char *kTextMain    = "#cdd6f4";
constexpr const char *kTextSubtle  = "#a6adc8";
constexpr const char *kTextMuted   = "#6c7086";
constexpr const char *kAccent      = "#89b4fa";
constexpr const char *kAccentGreen = "#a6e3a1";
constexpr const char *kAccentRed   = "#f38ba8";

constexpr const char *kStatusIdle   = "#6c7086";
constexpr const char *kStatusActive = "#a6e3a1";
constexpr const char *kStatusBusy   = "#f9e2af";
constexpr const char *kStatusError  = "#f38ba8";

QString iconButtonStyle(const QString &hoverColor, const QString &hoverBg)
{
    return QString(
               "QPushButton {"
               "   background: transparent;"
               "   border: none;"
               "   color: %1;"
               "   font-size: 12px;"
               "   border-radius: 10px;"
               "}"
               "QPushButton:hover {"
               "   background: %2;"
               "   color: %3;"
               "}"
               ).arg(kTextMuted, hoverBg, hoverColor);
}
}

ClickableLabel::ClickableLabel(QWidget *parent)
    : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

ClickableLabel::ClickableLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
    QLabel::mousePressEvent(event);
}

ClickableFrame::ClickableFrame(QWidget *parent)
    : QFrame(parent)
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
    applyStyle();
}

void ClickableFrame::setSelected(bool selected)
{
    m_selected = selected;
    applyStyle();
}

void ClickableFrame::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
    QFrame::mousePressEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ClickableFrame::enterEvent(QEnterEvent *event)
#else
void ClickableFrame::enterEvent(QEvent *event)
#endif
{
    m_hovered = true;
    applyStyle();
    QFrame::enterEvent(event);
}

void ClickableFrame::leaveEvent(QEvent *event)
{
    m_hovered = false;
    applyStyle();
    QFrame::leaveEvent(event);
}

void ClickableFrame::applyStyle()
{
    const QString bg = m_hovered ? kBgCardHover : kBgCard;
    const QString border = m_selected ? kBorderSel : kBorder;
    const QString borderWidth = m_selected ? "1.5px" : "1px";
    setStyleSheet(QString(
                      "ClickableFrame {"
                      "   background-color: %1;"
                      "   border-radius: 12px;"
                      "   border: %2 solid %3;"
                      "}"
                      ).arg(bg, borderWidth, border));
}

SubagentCard::SubagentCard(const QString &id, const QString &name, QWidget *parent)
    : QFrame(parent)
    , m_id(id)
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(14, 12, 10, 12);
    outer->setSpacing(6);

    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(4);

    m_statusDot = new QLabel("●", this);
    m_statusDot->setFixedWidth(14);
    m_statusDot->setStyleSheet(QString("font-size: 11px; border: none; background: transparent; color: %1;").arg(kStatusIdle));

    m_nameEdit = new QLineEdit(name, this);
    m_nameEdit->setPlaceholderText("Имя агента");
    m_nameEdit->setToolTip("Имя агента");
    m_nameEdit->setFrame(false);
    m_nameEdit->setStyleSheet(QString(
                                  "QLineEdit {"
                                  "   background: transparent;"
                                  "   border: none;"
                                  "   color: %1;"
                                  "   font-size: 14px;"
                                  "   font-weight: 600;"
                                  "   padding: 0px;"
                                  "}"
                                  "QLineEdit:focus { color: #ffffff; }"
                                  ).arg(kTextMain));
    connect(m_nameEdit, &QLineEdit::editingFinished, this, [this]() {
        emit nameEdited(m_id, m_nameEdit->text());
    });

    m_settingsButton = new QPushButton("⚙", this);
    m_settingsButton->setFixedSize(22, 22);
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    m_settingsButton->setToolTip("Настройки агента");
    m_settingsButton->setStyleSheet(iconButtonStyle(kAccent, "rgba(137, 180, 250, 0.15)"));
    connect(m_settingsButton, &QPushButton::clicked, this, [this]() {
        emit settingsRequested(m_id);
    });

    m_removeButton = new QPushButton("✕", this);
    m_removeButton->setFixedSize(22, 22);
    m_removeButton->setCursor(Qt::PointingHandCursor);
    m_removeButton->setToolTip("Удалить агента");
    m_removeButton->setStyleSheet(iconButtonStyle(kAccentRed, "rgba(243, 139, 168, 0.15)"));
    connect(m_removeButton, &QPushButton::clicked, this, [this]() {
        emit removeRequested(m_id);
    });

    topRow->addWidget(m_statusDot);
    topRow->addWidget(m_nameEdit, 1);
    topRow->addWidget(m_settingsButton);
    topRow->addWidget(m_removeButton);
    outer->addLayout(topRow);

    m_roleLabel = new ClickableLabel(this);
    m_roleLabel->setStyleSheet(QString(
                                   "font-size: 11px; border: none; background: transparent; color: %1; padding-left: 22px;"
                                   ).arg(kTextSubtle));
    m_roleLabel->setToolTip("Нажмите, чтобы открыть историю переписки");
    connect(m_roleLabel, &ClickableLabel::clicked, this, [this]() {
        emit clicked(m_id);
    });
    outer->addWidget(m_roleLabel);

    m_statusLabel = new ClickableLabel("Ожидание", this);
    m_statusLabel->setStyleSheet(QString(
                                     "font-size: 11px; border: none; background: transparent; color: %1; padding-left: 22px;"
                                     ).arg(kTextMuted));
    m_statusLabel->setToolTip("Нажмите, чтобы открыть историю переписки");
    connect(m_statusLabel, &ClickableLabel::clicked, this, [this]() {
        emit clicked(m_id);
    });
    outer->addWidget(m_statusLabel);

    setRole(QString());
    applyCardStyle();
    applyStatusStyle();
}

QString SubagentCard::name() const
{
    return m_nameEdit->text();
}

void SubagentCard::setName(const QString &name)
{
    if (m_nameEdit->text() != name)
        m_nameEdit->setText(name);
}

void SubagentCard::setRole(const QString &role)
{
    m_role = role;
    refreshRolePreview();
}

void SubagentCard::setStatus(Status status)
{
    m_status = status;
    applyStatusStyle();
}

void SubagentCard::setSelected(bool selected)
{
    m_selected = selected;
    applyCardStyle();
}

void SubagentCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked(m_id);
    QFrame::mousePressEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SubagentCard::enterEvent(QEnterEvent *event)
#else
void SubagentCard::enterEvent(QEvent *event)
#endif
{
    m_hovered = true;
    applyCardStyle();
    QFrame::enterEvent(event);
}

void SubagentCard::leaveEvent(QEvent *event)
{
    m_hovered = false;
    applyCardStyle();
    QFrame::leaveEvent(event);
}

void SubagentCard::applyCardStyle()
{
    const QString bg = m_hovered ? kBgCardHover : kBgCard;
    const QString border = m_selected ? kBorderSel : kBorder;
    const QString borderWidth = m_selected ? "1.5px" : "1px";
    setStyleSheet(QString(
                      "SubagentCard {"
                      "   background-color: %1;"
                      "   border: %2 solid %3;"
                      "   border-radius: 10px;"
                      "}"
                      ).arg(bg, borderWidth, border));
}

void SubagentCard::applyStatusStyle()
{
    QString color;
    QString text;
    switch (m_status) {
    case Status::Idle:   color = kStatusIdle;   text = "Ожидание";  break;
    case Status::Active: color = kStatusActive; text = "Активен";   break;
    case Status::Busy:   color = kStatusBusy;   text = "Выполняет"; break;
    case Status::Error:  color = kStatusError;  text = "Ошибка";    break;
    }
    m_statusDot->setStyleSheet(QString("font-size: 11px; border: none; background: transparent; color: %1;").arg(color));
    m_statusLabel->setText(text);
    m_statusLabel->setStyleSheet(QString(
                                     "font-size: 11px; border: none; background: transparent; color: %1; padding-left: 22px;"
                                     ).arg(color));
}

void SubagentCard::refreshRolePreview()
{
    QString preview = m_role.trimmed().isEmpty() ? QString("Роль не задана") : m_role;
    preview.replace('\n', ' ');
    const QFontMetrics metrics(m_roleLabel->font());
    m_roleLabel->setText(metrics.elidedText(preview, Qt::ElideRight, 208));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Agent Console");
    resize(980, 660);
    setStyleSheet(QString("QMainWindow { background-color: %1; }").arg(kBgWindow));

    m_mainAgentName = "Главный агент";

    buildUi();

    QDir agentsDir(QString(APP_SRC_DIR) + "/agents");
    const QStringList existingAgents = agentsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &folderName : existingAgents) {
        if (folderName == "main")
            continue;
        addSubagent(folderName);
    }

    startTime.start();
    uptimeTimer = new QTimer(this);
    connect(uptimeTimer, &QTimer::timeout, this, &MainWindow::updateUptime);
    uptimeTimer->start(1000);

    mainStatusCard->setSelected(true);
    refreshComposerPlaceholder();

    appendMainMessage("Главный агент подключён. Ожидаю команд.", false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    rootLayout->addWidget(buildLeftPanel(), 1);

    auto *vSep = new QFrame(this);
    vSep->setFrameShape(QFrame::VLine);
    vSep->setStyleSheet(QString("background-color: %1; border: none; max-width: 1px;").arg(kBorder));
    rootLayout->addWidget(vSep);

    auto *right = buildRightPanel();
    right->setFixedWidth(300);
    rootLayout->addWidget(right);

    setCentralWidget(central);
}

QWidget *MainWindow::buildLeftPanel()
{
    auto *panel = new QWidget(this);
    panel->setStyleSheet(QString("background-color: %1;").arg(kBgWindow));

    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(20, 18, 20, 20);
    layout->setSpacing(12);

    mainStatusCard = new ClickableFrame(panel);
    connect(mainStatusCard, &ClickableFrame::clicked, this, &MainWindow::selectMainAgent);

    auto *statusLayout = new QVBoxLayout(mainStatusCard);
    statusLayout->setContentsMargins(18, 14, 18, 14);
    statusLayout->setSpacing(6);

    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(10);

    agentStatusDot = new QLabel("●", mainStatusCard);
    agentStatusDot->setStyleSheet(QString("color: %1; font-size: 16px; border: none; background: transparent;").arg(kAccentGreen));

    mainNameEdit = new QLineEdit(m_mainAgentName, mainStatusCard);
    mainNameEdit->setToolTip("Имя главного агента");
    mainNameEdit->setFrame(false);
    mainNameEdit->setStyleSheet(QString(
                                    "QLineEdit {"
                                    "   background: transparent;"
                                    "   border: none;"
                                    "   color: %1;"
                                    "   font-size: 15px;"
                                    "   font-weight: bold;"
                                    "   padding: 0px;"
                                    "}"
                                    "QLineEdit:focus { color: #ffffff; }"
                                    ).arg(kTextMain));
    connect(mainNameEdit, &QLineEdit::editingFinished, this, [this]() {
        setMainAgentName(mainNameEdit->text());
        emit mainAgentRenamed(m_mainAgentName);
    });

    mainSettingsButton = new QPushButton("⚙", mainStatusCard);
    mainSettingsButton->setFixedSize(24, 24);
    mainSettingsButton->setCursor(Qt::PointingHandCursor);
    mainSettingsButton->setToolTip("Настройки главного агента");
    mainSettingsButton->setStyleSheet(iconButtonStyle(kAccent, "rgba(137, 180, 250, 0.15)"));
    connect(mainSettingsButton, &QPushButton::clicked, this, &MainWindow::openMainAgentSettings);

    topRow->addWidget(agentStatusDot);
    topRow->addWidget(mainNameEdit, 1);
    topRow->addWidget(mainSettingsButton);
    statusLayout->addLayout(topRow);

    auto *metaRow = new QHBoxLayout();
    metaRow->setSpacing(10);

    agentStatusText = new ClickableLabel("Активен", mainStatusCard);
    agentStatusText->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600; border: none; background: transparent;").arg(kAccentGreen));
    connect(agentStatusText, &ClickableLabel::clicked, this, &MainWindow::selectMainAgent);

    uptimeLabel = new ClickableLabel("Uptime: 00:00:00", mainStatusCard);
    uptimeLabel->setStyleSheet(QString("color: %1; font-size: 12px; border: none; background: transparent;").arg(kTextSubtle));
    connect(uptimeLabel, &ClickableLabel::clicked, this, &MainWindow::selectMainAgent);

    metaRow->addWidget(agentStatusText);
    metaRow->addStretch();
    metaRow->addWidget(uptimeLabel);
    statusLayout->addLayout(metaRow);

    startedAtLabel = new ClickableLabel(
        "Запущено: " + QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy"), mainStatusCard);
    startedAtLabel->setStyleSheet(QString("color: %1; font-size: 11px; border: none; background: transparent;").arg(kTextMuted));
    connect(startedAtLabel, &ClickableLabel::clicked, this, &MainWindow::selectMainAgent);
    statusLayout->addWidget(startedAtLabel);

    layout->addWidget(mainStatusCard);

    chatHeaderLabel = new QLabel(m_mainAgentName, panel);
    chatHeaderLabel->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 600; border: none; background: transparent;").arg(kTextSubtle));
    layout->addWidget(chatHeaderLabel);

    chatLog = new QTextEdit(panel);
    chatLog->setReadOnly(true);
    chatLog->setFrameShape(QFrame::NoFrame);
    chatLog->setStyleSheet(QString(
                               "QTextEdit {"
                               "   background-color: %1;"
                               "   border: 1px solid %2;"
                               "   border-radius: 12px;"
                               "   padding: 10px;"
                               "   color: %3;"
                               "   font-size: 13px;"
                               "}"
                               "QScrollBar:vertical { background: transparent; width: 8px; margin: 0px; }"
                               "QScrollBar::handle:vertical { background: %2; border-radius: 4px; min-height: 24px; }"
                               "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
                               ).arg(kBgCard, kBorder, kTextMain));
    layout->addWidget(chatLog, 1);

    auto *inputFrame = new QFrame(panel);
    inputFrame->setStyleSheet(QString(
                                  "QFrame {"
                                  "   background-color: %1;"
                                  "   border: 1px solid %2;"
                                  "   border-radius: 12px;"
                                  "}"
                                  ).arg(kBgInput, kBorder));
    auto *inputLayout = new QHBoxLayout(inputFrame);
    inputLayout->setContentsMargins(12, 8, 8, 8);
    inputLayout->setSpacing(8);

    messageInput = new QPlainTextEdit(inputFrame);
    messageInput->setFixedHeight(56);
    messageInput->setFrameShape(QFrame::NoFrame);
    messageInput->setStyleSheet(QString(
                                    "QPlainTextEdit {"
                                    "   background: transparent;"
                                    "   border: none;"
                                    "   color: %1;"
                                    "   font-size: 13px;"
                                    "}"
                                    ).arg(kTextMain));
    messageInput->installEventFilter(this);

    sendButton = new QPushButton("➤", inputFrame);
    sendButton->setFixedSize(40, 40);
    sendButton->setCursor(Qt::PointingHandCursor);
    sendButton->setToolTip("Отправить");
    sendButton->setStyleSheet(QString(
                                  "QPushButton {"
                                  "   background-color: %1;"
                                  "   color: #11111b;"
                                  "   border: none;"
                                  "   border-radius: 20px;"
                                  "   font-size: 15px;"
                                  "   font-weight: bold;"
                                  "}"
                                  "QPushButton:hover { background-color: #a6c8ff; }"
                                  "QPushButton:pressed { background-color: #74a8fc; }"
                                  ).arg(kAccent));
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::handleSendClicked);

    inputLayout->addWidget(messageInput, 1);
    inputLayout->addWidget(sendButton, 0, Qt::AlignBottom);

    layout->addWidget(inputFrame);

    return panel;
}

QWidget *MainWindow::buildRightPanel()
{
    auto *panel = new QWidget(this);
    panel->setStyleSheet(QString("background-color: %1;").arg(kBgPanel));

    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 18, 16, 16);
    layout->setSpacing(12);

    auto *headerRow = new QHBoxLayout();
    headerRow->setSpacing(8);

    auto *title = new QLabel("Сабагенты", panel);
    title->setStyleSheet(QString("color: %1; font-size: 15px; font-weight: bold; border: none; background: transparent;").arg(kTextMain));

    subagentCountLabel = new QLabel("0", panel);
    subagentCountLabel->setAlignment(Qt::AlignCenter);
    subagentCountLabel->setFixedSize(20, 20);
    subagentCountLabel->setStyleSheet(QString(
                                          "color: %1; font-size: 11px; font-weight: bold; background-color: %2; border-radius: 10px;"
                                          ).arg(kTextSubtle, kBgCard));

    addSubagentButton = new QPushButton("+", panel);
    addSubagentButton->setFixedSize(26, 26);
    addSubagentButton->setCursor(Qt::PointingHandCursor);
    addSubagentButton->setToolTip("Добавить сабагента");
    addSubagentButton->setStyleSheet(QString(
                                         "QPushButton {"
                                         "   background-color: %1;"
                                         "   color: %2;"
                                         "   border: 1px solid %3;"
                                         "   border-radius: 13px;"
                                         "   font-size: 15px;"
                                         "   font-weight: bold;"
                                         "}"
                                         "QPushButton:hover { border-color: %4; color: %4; }"
                                         ).arg(kBgCard, kTextSubtle, kBorder, kAccent));
    connect(addSubagentButton, &QPushButton::clicked, this, [this]() {
        const QString id = addSubagent();
        emit addSubagentRequested();
        selectSubagent(id);
    });

    headerRow->addWidget(title);
    headerRow->addWidget(subagentCountLabel);
    headerRow->addStretch();
    headerRow->addWidget(addSubagentButton);
    layout->addLayout(headerRow);

    subagentsScroll = new QScrollArea(panel);
    subagentsScroll->setWidgetResizable(true);
    subagentsScroll->setFrameShape(QFrame::NoFrame);
    subagentsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    subagentsScroll->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #313244; border-radius: 4px; min-height: 24px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        );

    auto *scrollContent = new QWidget(subagentsScroll);
    scrollContent->setStyleSheet("background: transparent;");
    subagentsLayout = new QVBoxLayout(scrollContent);
    subagentsLayout->setContentsMargins(0, 0, 4, 0);
    subagentsLayout->setSpacing(10);
    subagentsLayout->addStretch(1);

    subagentsScroll->setWidget(scrollContent);
    layout->addWidget(subagentsScroll, 1);

    return panel;
}

void MainWindow::appendMainMessage(const QString &text, bool isUser)
{
    appendHistory(QString(), text, isUser);
}

void MainWindow::appendSubagentMessage(const QString &id, const QString &text, bool isUser)
{
    appendHistory(id, text, isUser);
}

void MainWindow::setAgentActive(bool active)
{
    agentStatusDot->setStyleSheet(QString("color: %1; font-size: 16px; border: none; background: transparent;")
                                      .arg(active ? kAccentGreen : kAccentRed));
    agentStatusText->setText(active ? "Активен" : "Офлайн");
    agentStatusText->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600; border: none; background: transparent;")
                                       .arg(active ? kAccentGreen : kAccentRed));
}

void MainWindow::setMainAgentName(const QString &name)
{
    m_mainAgentName = name.trimmed().isEmpty() ? QString("Главный агент") : name.trimmed();
    if (mainNameEdit->text() != m_mainAgentName)
        mainNameEdit->setText(m_mainAgentName);
    if (m_activeAgentId.isEmpty())
        chatHeaderLabel->setText(m_mainAgentName);
    refreshComposerPlaceholder();
}

void MainWindow::setMainAgentRole(const QString &role)
{
    m_mainAgentRole = role;
}

QString MainWindow::addSubagent(const QString &name, const QString &role)
{
    ++m_subagentSeq;
    const QString id = QString("agent-%1").arg(m_subagentSeq);
    const QString displayName = name.isEmpty() ? QString("Агент %1").arg(m_subagentSeq) : name;

    template_agents::Generate(displayName);

    auto *card = new SubagentCard(id, displayName, subagentsScroll->widget());
    card->setRole(role);

    connect(card, &SubagentCard::clicked, this, &MainWindow::selectSubagent);
    connect(card, &SubagentCard::settingsRequested, this, &MainWindow::openSubagentSettings);
    connect(card, &SubagentCard::removeRequested, this, [this](const QString &cardId) {
        emit subagentRemoveRequested(cardId);
        removeSubagent(cardId);
    });
    connect(card, &SubagentCard::nameEdited, this, [this](const QString &cardId, const QString &newName) {
        const QString oldName = m_agentFolderNames.value(cardId);
        if (oldName != newName) {
            template_agents::Rename(oldName, newName);
            m_agentFolderNames.insert(cardId, newName);
        }
        emit subagentRenamed(cardId, newName);
        if (cardId == m_activeAgentId)
            chatHeaderLabel->setText(newName);
    });

    subagentsLayout->insertWidget(subagentsLayout->count() - 1, card);
    m_cards.insert(id, card);
    m_histories.insert(id, {});
    m_agentFolderNames.insert(id, displayName);
    subagentCountLabel->setText(QString::number(m_cards.size()));

    return id;
}

void MainWindow::removeSubagent(const QString &id)
{
    if (auto *card = m_cards.take(id)) {
        card->deleteLater();
        m_histories.remove(id);
        m_agentFolderNames.remove(id);
        subagentCountLabel->setText(QString::number(m_cards.size()));
        if (m_activeAgentId == id)
            selectMainAgent();
    }
}

void MainWindow::setSubagentStatus(const QString &id, AgentStatus status)
{
    auto *card = m_cards.value(id, nullptr);
    if (!card)
        return;
    switch (status) {
    case AgentStatus::Idle:   card->setStatus(SubagentCard::Status::Idle);   break;
    case AgentStatus::Active: card->setStatus(SubagentCard::Status::Active); break;
    case AgentStatus::Busy:   card->setStatus(SubagentCard::Status::Busy);   break;
    case AgentStatus::Error:  card->setStatus(SubagentCard::Status::Error);  break;
    }
}

void MainWindow::setSubagentName(const QString &id, const QString &name)
{
    auto *card = m_cards.value(id, nullptr);
    if (!card)
        return;
    card->setName(name);
    if (id == m_activeAgentId)
        chatHeaderLabel->setText(name);
}

void MainWindow::setSubagentRole(const QString &id, const QString &role)
{
    if (auto *card = m_cards.value(id, nullptr))
        card->setRole(role);
}

QString MainWindow::subagentName(const QString &id) const
{
    auto *card = m_cards.value(id, nullptr);
    return card ? card->name() : QString();
}

QString MainWindow::subagentRole(const QString &id) const
{
    auto *card = m_cards.value(id, nullptr);
    return card ? card->role() : QString();
}

bool MainWindow::hasSubagent(const QString &id) const
{
    return m_cards.contains(id);
}

QStringList MainWindow::subagentIds() const
{
    return m_cards.keys();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == messageInput && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
            && !(keyEvent->modifiers() & Qt::ShiftModifier)) {
            handleSendClicked();
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::selectMainAgent()
{
    m_activeAgentId.clear();
    mainStatusCard->setSelected(true);
    for (SubagentCard *card : std::as_const(m_cards))
        card->setSelected(false);
    chatHeaderLabel->setText(m_mainAgentName);
    refreshComposerPlaceholder();
    redrawActiveHistory();
    emit mainAgentSelected();
}

void MainWindow::selectSubagent(const QString &id)
{
    auto *target = m_cards.value(id, nullptr);
    if (!target)
        return;
    m_activeAgentId = id;
    mainStatusCard->setSelected(false);
    for (SubagentCard *card : std::as_const(m_cards))
        card->setSelected(card->id() == id);
    chatHeaderLabel->setText(target->name());
    refreshComposerPlaceholder();
    redrawActiveHistory();
    emit subagentSelected(id);
}

void MainWindow::refreshComposerPlaceholder()
{
    const QString targetName = m_activeAgentId.isEmpty()
    ? m_mainAgentName
    : subagentName(m_activeAgentId);
    messageInput->setPlaceholderText(
        QString("Написать агенту «%1»…  (Enter — отправить, Shift+Enter — новая строка)").arg(targetName));
}

void MainWindow::appendHistory(const QString &agentId, const QString &text, bool isUser)
{
    ChatEntry entry{text, isUser, QDateTime::currentDateTime().toString("hh:mm")};
    m_histories[agentId].append(entry);
    if (agentId == m_activeAgentId)
        renderMessage(entry);
}

void MainWindow::renderMessage(const ChatEntry &entry)
{
    const QString escaped = entry.text.toHtmlEscaped().replace("\n", "<br/>");
    const QString author = entry.isUser
                               ? "Вы"
                               : (m_activeAgentId.isEmpty() ? m_mainAgentName : subagentName(m_activeAgentId));
    const QString authorColor = entry.isUser ? "#89b4fa" : "#a6e3a1";
    const QString align = entry.isUser ? "right" : "left";
    const QString bubbleStyle = entry.isUser
                                    ? "background-color:#313244;"
                                    : QString("background-color:%1; border: 1px solid #313244;").arg(kBgCard);

    const QString block = QString(
                              "<table align='%1' width='75%%' cellspacing='0' cellpadding='9' style='%2'>"
                              "<tr><td>"
                              "<div style='font-size: 11px; font-weight: bold; color: %3;'>%4 &middot; %5</div>"
                              "<div style='font-size: 13px; color: #cdd6f4; margin-top: 3px;'>%6</div>"
                              "</td></tr></table>"
                              "<div style='height: 8px;'></div>"
                              ).arg(align, bubbleStyle, authorColor, author, entry.time, escaped);

    chatLog->append(block);
    QScrollBar *bar = chatLog->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void MainWindow::redrawActiveHistory()
{
    chatLog->clear();
    const auto entries = m_histories.value(m_activeAgentId);
    for (const auto &entry : entries)
        renderMessage(entry);
}

bool MainWindow::runAgentSettingsDialog(const QString &title, QString &name, QString &role)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumWidth(440);
    dialog.setStyleSheet(QString(
                             "QDialog { background-color: %1; }"
                             "QLabel { color: %2; font-size: 12px; border: none; background: transparent; }"
                             "QLineEdit, QTextEdit {"
                             "   background-color: %3;"
                             "   color: %2;"
                             "   border: 1px solid %4;"
                             "   border-radius: 8px;"
                             "   padding: 8px;"
                             "   font-size: 13px;"
                             "}"
                             "QLineEdit:focus, QTextEdit:focus { border-color: %5; }"
                             "QPushButton {"
                             "   background-color: %3;"
                             "   color: %2;"
                             "   border: 1px solid %4;"
                             "   border-radius: 8px;"
                             "   padding: 6px 16px;"
                             "   font-size: 13px;"
                             "}"
                             "QPushButton:hover { border-color: %5; color: %5; }"
                             ).arg(kBgWindow, kTextMain, kBgCard, kBorder, kAccent));

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    auto *nameLabel = new QLabel("Имя", &dialog);
    auto *nameEdit = new QLineEdit(name, &dialog);
    nameEdit->setPlaceholderText("Название агента");

    auto *roleLabel = new QLabel("Роль / системный промпт", &dialog);
    auto *roleEdit = new QTextEdit(&dialog);
    roleEdit->setPlainText(role);
    roleEdit->setPlaceholderText("Опишите роль агента, его задачи, стиль общения, ограничения…");
    roleEdit->setMinimumHeight(180);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Save)->setText("Сохранить");
    buttons->button(QDialogButtonBox::Cancel)->setText("Отмена");
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout->addWidget(nameLabel);
    layout->addWidget(nameEdit);
    layout->addWidget(roleLabel);
    layout->addWidget(roleEdit, 1);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted)
        return false;

    name = nameEdit->text().trimmed();
    role = roleEdit->toPlainText();
    return true;
}

void MainWindow::openMainAgentSettings()
{
    QString name = m_mainAgentName;
    QString role = m_mainAgentRole;
    if (!runAgentSettingsDialog("Настройки главного агента", name, role))
        return;

    setMainAgentName(name);
    setMainAgentRole(role);
    emit mainAgentRenamed(m_mainAgentName);
    emit mainAgentRoleChanged(m_mainAgentRole);
}

void MainWindow::openSubagentSettings(const QString &id)
{
    auto *card = m_cards.value(id, nullptr);
    if (!card)
        return;

    QString name = card->name();
    QString role = card->role();
    if (!runAgentSettingsDialog("Настройки агента", name, role))
        return;

    const QString finalName = name.isEmpty() ? card->name() : name;
    const QString oldName = m_agentFolderNames.value(id);
    if (oldName != finalName) {
        template_agents::Rename(oldName, finalName);
        m_agentFolderNames.insert(id, finalName);
    }
    setSubagentName(id, finalName);
    setSubagentRole(id, role);
    emit subagentRenamed(id, finalName);
    emit subagentRoleChanged(id, role);
}

void MainWindow::handleSendClicked()
{
    const QString text = messageInput->toPlainText().trimmed();
    if (text.isEmpty())
        return;

    appendHistory(m_activeAgentId, text, true);
    const QString promptName = m_activeAgentId.isEmpty() ? QStringLiteral("main") : m_agentFolderNames.value(m_activeAgentId);
    emit messageSubmitted(m_activeAgentId, promptName, text);
    messageInput->clear();
}

void MainWindow::receiveAgentReply(const QString &agentId, const QString &text)
{
    appendHistory(agentId, text, false);
}

void MainWindow::updateUptime()
{
    qint64 secs = startTime.elapsed() / 1000;
    int hours = secs / 3600;
    int mins = (secs % 3600) / 60;
    int seconds = secs % 60;
    QString timeStr = QString("Uptime: %1:%2:%3")
                          .arg(hours, 2, 10, QChar('0'))
                          .arg(mins, 2, 10, QChar('0'))
                          .arg(seconds, 2, 10, QChar('0'));
    uptimeLabel->setText(timeStr);
}
