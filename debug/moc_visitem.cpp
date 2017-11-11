/****************************************************************************
** Meta object code from reading C++ file 'visitem.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../ui/visitem.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'visitem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VisItem_t {
    QByteArrayData data[22];
    char stringdata0[225];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VisItem_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VisItem_t qt_meta_stringdata_VisItem = {
    {
QT_MOC_LITERAL(0, 0, 7), // "VisItem"
QT_MOC_LITERAL(1, 8, 18), // "roundForParticleAt"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 4), // "Node"
QT_MOC_LITERAL(4, 33, 4), // "node"
QT_MOC_LITERAL(5, 38, 15), // "inspectParticle"
QT_MOC_LITERAL(6, 54, 4), // "text"
QT_MOC_LITERAL(7, 59, 13), // "systemChanged"
QT_MOC_LITERAL(8, 73, 23), // "std::shared_ptr<System>"
QT_MOC_LITERAL(9, 97, 7), // "_system"
QT_MOC_LITERAL(10, 105, 19), // "focusOnCenterOfMass"
QT_MOC_LITERAL(11, 125, 7), // "focusOn"
QT_MOC_LITERAL(12, 133, 7), // "setZoom"
QT_MOC_LITERAL(13, 141, 4), // "zoom"
QT_MOC_LITERAL(14, 146, 14), // "saveScreenshot"
QT_MOC_LITERAL(15, 161, 8), // "filePath"
QT_MOC_LITERAL(16, 170, 10), // "initialize"
QT_MOC_LITERAL(17, 181, 5), // "paint"
QT_MOC_LITERAL(18, 187, 12), // "deinitialize"
QT_MOC_LITERAL(19, 200, 11), // "sizeChanged"
QT_MOC_LITERAL(20, 212, 5), // "width"
QT_MOC_LITERAL(21, 218, 6) // "height"

    },
    "VisItem\0roundForParticleAt\0\0Node\0node\0"
    "inspectParticle\0text\0systemChanged\0"
    "std::shared_ptr<System>\0_system\0"
    "focusOnCenterOfMass\0focusOn\0setZoom\0"
    "zoom\0saveScreenshot\0filePath\0initialize\0"
    "paint\0deinitialize\0sizeChanged\0width\0"
    "height"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VisItem[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       5,    1,   72,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   75,    2, 0x0a /* Public */,
      10,    0,   78,    2, 0x0a /* Public */,
      11,    1,   79,    2, 0x0a /* Public */,
      12,    1,   82,    2, 0x0a /* Public */,
      14,    1,   85,    2, 0x0a /* Public */,
      16,    0,   88,    2, 0x09 /* Protected */,
      17,    0,   89,    2, 0x09 /* Protected */,
      18,    0,   90,    2, 0x09 /* Protected */,
      19,    2,   91,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    6,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Float,   13,
    QMetaType::Void, QMetaType::QString,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   20,   21,

       0        // eod
};

void VisItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        VisItem *_t = static_cast<VisItem *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->roundForParticleAt((*reinterpret_cast< Node(*)>(_a[1]))); break;
        case 1: _t->inspectParticle((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->systemChanged((*reinterpret_cast< std::shared_ptr<System>(*)>(_a[1]))); break;
        case 3: _t->focusOnCenterOfMass(); break;
        case 4: _t->focusOn((*reinterpret_cast< Node(*)>(_a[1]))); break;
        case 5: _t->setZoom((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 6: _t->saveScreenshot((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: _t->initialize(); break;
        case 8: _t->paint(); break;
        case 9: _t->deinitialize(); break;
        case 10: _t->sizeChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (VisItem::*_t)(Node );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&VisItem::roundForParticleAt)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (VisItem::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&VisItem::inspectParticle)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject VisItem::staticMetaObject = {
    { &GLItem::staticMetaObject, qt_meta_stringdata_VisItem.data,
      qt_meta_data_VisItem,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *VisItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VisItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VisItem.stringdata0))
        return static_cast<void*>(this);
    return GLItem::qt_metacast(_clname);
}

int VisItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = GLItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void VisItem::roundForParticleAt(Node _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void VisItem::inspectParticle(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
