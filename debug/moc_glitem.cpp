/****************************************************************************
** Meta object code from reading C++ file 'glitem.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../ui/glitem.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'glitem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GLItem_t {
    QByteArrayData data[16];
    char stringdata0[190];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GLItem_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GLItem_t qt_meta_stringdata_GLItem = {
    {
QT_MOC_LITERAL(0, 0, 6), // "GLItem"
QT_MOC_LITERAL(1, 7, 15), // "beforeRendering"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 14), // "afterRendering"
QT_MOC_LITERAL(4, 39, 10), // "initialize"
QT_MOC_LITERAL(5, 50, 5), // "paint"
QT_MOC_LITERAL(6, 56, 12), // "deinitialize"
QT_MOC_LITERAL(7, 69, 11), // "sizeChanged"
QT_MOC_LITERAL(8, 81, 5), // "width"
QT_MOC_LITERAL(9, 87, 6), // "height"
QT_MOC_LITERAL(10, 94, 19), // "handleWindowChanged"
QT_MOC_LITERAL(11, 114, 13), // "QQuickWindow*"
QT_MOC_LITERAL(12, 128, 6), // "window"
QT_MOC_LITERAL(13, 135, 13), // "delegatePaint"
QT_MOC_LITERAL(14, 149, 20), // "delegeteDeinitialize"
QT_MOC_LITERAL(15, 170, 19) // "delegateSizeChanged"

    },
    "GLItem\0beforeRendering\0\0afterRendering\0"
    "initialize\0paint\0deinitialize\0sizeChanged\0"
    "width\0height\0handleWindowChanged\0"
    "QQuickWindow*\0window\0delegatePaint\0"
    "delegeteDeinitialize\0delegateSizeChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GLItem[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x06 /* Public */,
       3,    0,   65,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   66,    2, 0x09 /* Protected */,
       5,    0,   67,    2, 0x09 /* Protected */,
       6,    0,   68,    2, 0x09 /* Protected */,
       7,    2,   69,    2, 0x09 /* Protected */,
      10,    1,   74,    2, 0x08 /* Private */,
      13,    0,   77,    2, 0x08 /* Private */,
      14,    0,   78,    2, 0x08 /* Private */,
      15,    0,   79,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    8,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void GLItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        GLItem *_t = static_cast<GLItem *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->beforeRendering(); break;
        case 1: _t->afterRendering(); break;
        case 2: _t->initialize(); break;
        case 3: _t->paint(); break;
        case 4: _t->deinitialize(); break;
        case 5: _t->sizeChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->handleWindowChanged((*reinterpret_cast< QQuickWindow*(*)>(_a[1]))); break;
        case 7: _t->delegatePaint(); break;
        case 8: _t->delegeteDeinitialize(); break;
        case 9: _t->delegateSizeChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (GLItem::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&GLItem::beforeRendering)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (GLItem::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&GLItem::afterRendering)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject GLItem::staticMetaObject = {
    { &QQuickItem::staticMetaObject, qt_meta_stringdata_GLItem.data,
      qt_meta_data_GLItem,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *GLItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GLItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GLItem.stringdata0))
        return static_cast<void*>(this);
    return QQuickItem::qt_metacast(_clname);
}

int GLItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void GLItem::beforeRendering()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void GLItem::afterRendering()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
