/****************************************************************************
** Meta object code from reading C++ file 'commandhistorymanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../ui/commandhistorymanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'commandhistorymanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CommandHistoryManager_t {
    QByteArrayData data[8];
    char stringdata0[72];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CommandHistoryManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CommandHistoryManager_t qt_meta_stringdata_CommandHistoryManager = {
    {
QT_MOC_LITERAL(0, 0, 21), // "CommandHistoryManager"
QT_MOC_LITERAL(1, 22, 10), // "setCommand"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 7), // "command"
QT_MOC_LITERAL(4, 42, 15), // "commandExecuted"
QT_MOC_LITERAL(5, 58, 2), // "up"
QT_MOC_LITERAL(6, 61, 4), // "down"
QT_MOC_LITERAL(7, 66, 5) // "reset"

    },
    "CommandHistoryManager\0setCommand\0\0"
    "command\0commandExecuted\0up\0down\0reset"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CommandHistoryManager[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   42,    2, 0x0a /* Public */,
       5,    0,   45,    2, 0x0a /* Public */,
       6,    0,   46,    2, 0x0a /* Public */,
       7,    0,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CommandHistoryManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CommandHistoryManager *_t = static_cast<CommandHistoryManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setCommand((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->commandExecuted((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->up(); break;
        case 3: _t->down(); break;
        case 4: _t->reset(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (CommandHistoryManager::*_t)(const QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CommandHistoryManager::setCommand)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject CommandHistoryManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CommandHistoryManager.data,
      qt_meta_data_CommandHistoryManager,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CommandHistoryManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CommandHistoryManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CommandHistoryManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CommandHistoryManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void CommandHistoryManager::setCommand(const QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
