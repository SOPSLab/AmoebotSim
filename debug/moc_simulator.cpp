/****************************************************************************
** Meta object code from reading C++ file 'simulator.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../sim/simulator.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'simulator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Simulator_t {
    QByteArrayData data[26];
    char stringdata0[305];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Simulator_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Simulator_t qt_meta_stringdata_Simulator = {
    {
QT_MOC_LITERAL(0, 0, 9), // "Simulator"
QT_MOC_LITERAL(1, 10, 13), // "systemChanged"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 23), // "std::shared_ptr<System>"
QT_MOC_LITERAL(4, 49, 7), // "_system"
QT_MOC_LITERAL(5, 57, 20), // "roundDurationChanged"
QT_MOC_LITERAL(6, 78, 2), // "ms"
QT_MOC_LITERAL(7, 81, 14), // "saveScreenshot"
QT_MOC_LITERAL(8, 96, 8), // "filePath"
QT_MOC_LITERAL(9, 105, 7), // "started"
QT_MOC_LITERAL(10, 113, 7), // "stopped"
QT_MOC_LITERAL(11, 121, 5), // "round"
QT_MOC_LITERAL(12, 127, 18), // "roundForParticleAt"
QT_MOC_LITERAL(13, 146, 4), // "Node"
QT_MOC_LITERAL(14, 151, 4), // "node"
QT_MOC_LITERAL(15, 156, 19), // "runUntilTermination"
QT_MOC_LITERAL(16, 176, 5), // "start"
QT_MOC_LITERAL(17, 182, 4), // "stop"
QT_MOC_LITERAL(18, 187, 19), // "saveScreenshotSetup"
QT_MOC_LITERAL(19, 207, 12), // "numParticles"
QT_MOC_LITERAL(20, 220, 12), // "numMovements"
QT_MOC_LITERAL(21, 233, 9), // "numRounds"
QT_MOC_LITERAL(22, 243, 20), // "leaderElectionRounds"
QT_MOC_LITERAL(23, 264, 10), // "weakBounds"
QT_MOC_LITERAL(24, 275, 12), // "strongBounds"
QT_MOC_LITERAL(25, 288, 16) // "setRoundDuration"

    },
    "Simulator\0systemChanged\0\0"
    "std::shared_ptr<System>\0_system\0"
    "roundDurationChanged\0ms\0saveScreenshot\0"
    "filePath\0started\0stopped\0round\0"
    "roundForParticleAt\0Node\0node\0"
    "runUntilTermination\0start\0stop\0"
    "saveScreenshotSetup\0numParticles\0"
    "numMovements\0numRounds\0leaderElectionRounds\0"
    "weakBounds\0strongBounds\0setRoundDuration"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Simulator[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  104,    2, 0x06 /* Public */,
       5,    1,  107,    2, 0x06 /* Public */,
       7,    1,  110,    2, 0x06 /* Public */,
       9,    0,  113,    2, 0x06 /* Public */,
      10,    0,  114,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,  115,    2, 0x0a /* Public */,
      12,    1,  116,    2, 0x0a /* Public */,
      15,    0,  119,    2, 0x0a /* Public */,
      16,    0,  120,    2, 0x0a /* Public */,
      17,    0,  121,    2, 0x0a /* Public */,
      18,    1,  122,    2, 0x0a /* Public */,
      19,    0,  125,    2, 0x0a /* Public */,
      20,    0,  126,    2, 0x0a /* Public */,
      21,    0,  127,    2, 0x0a /* Public */,
      22,    0,  128,    2, 0x0a /* Public */,
      23,    0,  129,    2, 0x0a /* Public */,
      24,    0,  130,    2, 0x0a /* Public */,
      25,    1,  131,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Void, QMetaType::Int,    6,

       0        // eod
};

void Simulator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Simulator *_t = static_cast<Simulator *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->systemChanged((*reinterpret_cast< std::shared_ptr<System>(*)>(_a[1]))); break;
        case 1: _t->roundDurationChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->saveScreenshot((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->started(); break;
        case 4: _t->stopped(); break;
        case 5: _t->round(); break;
        case 6: _t->roundForParticleAt((*reinterpret_cast< Node(*)>(_a[1]))); break;
        case 7: _t->runUntilTermination(); break;
        case 8: _t->start(); break;
        case 9: _t->stop(); break;
        case 10: _t->saveScreenshotSetup((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: { int _r = _t->numParticles();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 12: { int _r = _t->numMovements();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 13: { int _r = _t->numRounds();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 14: { int _r = _t->leaderElectionRounds();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 15: { int _r = _t->weakBounds();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 16: { int _r = _t->strongBounds();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 17: _t->setRoundDuration((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Simulator::*_t)(std::shared_ptr<System> );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Simulator::systemChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (Simulator::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Simulator::roundDurationChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (Simulator::*_t)(const QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Simulator::saveScreenshot)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (Simulator::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Simulator::started)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (Simulator::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Simulator::stopped)) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject Simulator::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Simulator.data,
      qt_meta_data_Simulator,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *Simulator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Simulator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Simulator.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Simulator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void Simulator::systemChanged(std::shared_ptr<System> _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Simulator::roundDurationChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Simulator::saveScreenshot(const QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Simulator::started()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Simulator::stopped()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
