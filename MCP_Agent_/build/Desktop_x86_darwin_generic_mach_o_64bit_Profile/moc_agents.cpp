/****************************************************************************
** Meta object code from reading C++ file 'agents.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../agents.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'agents.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN6agentsE_t {};
} // unnamed namespace

template <> constexpr inline auto agents::qt_create_metaobjectdata<qt_meta_tag_ZN6agentsE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "agents",
        "requestSendMessagesDelayed",
        "",
        "chatId",
        "QList<DelayedMessage>",
        "messages",
        "requestSendPhoto",
        "photoUrl",
        "caption",
        "requestSendSticker",
        "stickerId",
        "requestReqAgent",
        "userText",
        "agentName",
        "imageData"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'requestSendMessagesDelayed'
        QtMocHelpers::SignalData<void(qint64, const QList<DelayedMessage> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 3 }, { 0x80000000 | 4, 5 },
        }}),
        // Signal 'requestSendPhoto'
        QtMocHelpers::SignalData<void(qint64, const QString &, const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 3 }, { QMetaType::QString, 7 }, { QMetaType::QString, 8 },
        }}),
        // Signal 'requestSendSticker'
        QtMocHelpers::SignalData<void(qint64, const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 3 }, { QMetaType::QString, 10 },
        }}),
        // Signal 'requestReqAgent'
        QtMocHelpers::SignalData<void(const QString &, qint64, const QString &, const QByteArray &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 }, { QMetaType::LongLong, 3 }, { QMetaType::QString, 13 }, { QMetaType::QByteArray, 14 },
        }}),
        // Signal 'requestReqAgent'
        QtMocHelpers::SignalData<void(const QString &, qint64, const QString &)>(11, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 12 }, { QMetaType::LongLong, 3 }, { QMetaType::QString, 13 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<agents, qt_meta_tag_ZN6agentsE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject agents::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6agentsE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6agentsE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6agentsE_t>.metaTypes,
    nullptr
} };

void agents::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<agents *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->requestSendMessagesDelayed((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<DelayedMessage>>>(_a[2]))); break;
        case 1: _t->requestSendPhoto((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 2: _t->requestSendSticker((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->requestReqAgent((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QByteArray>>(_a[4]))); break;
        case 4: _t->requestReqAgent((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (agents::*)(qint64 , const QList<DelayedMessage> & )>(_a, &agents::requestSendMessagesDelayed, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (agents::*)(qint64 , const QString & , const QString & )>(_a, &agents::requestSendPhoto, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (agents::*)(qint64 , const QString & )>(_a, &agents::requestSendSticker, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (agents::*)(const QString & , qint64 , const QString & , const QByteArray & )>(_a, &agents::requestReqAgent, 3))
            return;
    }
}

const QMetaObject *agents::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *agents::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6agentsE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int agents::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void agents::requestSendMessagesDelayed(qint64 _t1, const QList<DelayedMessage> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void agents::requestSendPhoto(qint64 _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3);
}

// SIGNAL 2
void agents::requestSendSticker(qint64 _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void agents::requestReqAgent(const QString & _t1, qint64 _t2, const QString & _t3, const QByteArray & _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3, _t4);
}
QT_WARNING_POP
