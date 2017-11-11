/****************************************************************************
** Meta object code from reading C++ file 'scriptinterface.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../script/scriptinterface.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scriptinterface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ScriptInterface_t {
    QByteArrayData data[66];
    char stringdata0[721];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ScriptInterface_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ScriptInterface_t qt_meta_stringdata_ScriptInterface = {
    {
QT_MOC_LITERAL(0, 0, 15), // "ScriptInterface"
QT_MOC_LITERAL(1, 16, 3), // "log"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 3), // "msg"
QT_MOC_LITERAL(4, 25, 5), // "error"
QT_MOC_LITERAL(5, 31, 9), // "runScript"
QT_MOC_LITERAL(6, 41, 14), // "scriptFilePath"
QT_MOC_LITERAL(7, 56, 11), // "writeToFile"
QT_MOC_LITERAL(8, 68, 8), // "filePath"
QT_MOC_LITERAL(9, 77, 4), // "text"
QT_MOC_LITERAL(10, 82, 5), // "round"
QT_MOC_LITERAL(11, 88, 19), // "runUntilTermination"
QT_MOC_LITERAL(12, 108, 15), // "getNumParticles"
QT_MOC_LITERAL(13, 124, 15), // "getNumMovements"
QT_MOC_LITERAL(14, 140, 12), // "getNumRounds"
QT_MOC_LITERAL(15, 153, 23), // "getLeaderElectionRounds"
QT_MOC_LITERAL(16, 177, 12), // "getWeakBound"
QT_MOC_LITERAL(17, 190, 14), // "getStrongBound"
QT_MOC_LITERAL(18, 205, 16), // "setRoundDuration"
QT_MOC_LITERAL(19, 222, 2), // "ms"
QT_MOC_LITERAL(20, 225, 7), // "focusOn"
QT_MOC_LITERAL(21, 233, 1), // "x"
QT_MOC_LITERAL(22, 235, 1), // "y"
QT_MOC_LITERAL(23, 237, 7), // "setZoom"
QT_MOC_LITERAL(24, 245, 4), // "zoom"
QT_MOC_LITERAL(25, 250, 14), // "saveScreenshot"
QT_MOC_LITERAL(26, 265, 14), // "filmSimulation"
QT_MOC_LITERAL(27, 280, 9), // "iterLimit"
QT_MOC_LITERAL(28, 290, 5), // "adder"
QT_MOC_LITERAL(29, 296, 12), // "numParticles"
QT_MOC_LITERAL(30, 309, 10), // "countValue"
QT_MOC_LITERAL(31, 320, 11), // "compression"
QT_MOC_LITERAL(32, 332, 6), // "lambda"
QT_MOC_LITERAL(33, 339, 7), // "hexagon"
QT_MOC_LITERAL(34, 347, 8), // "holeProb"
QT_MOC_LITERAL(35, 356, 13), // "infObjCoating"
QT_MOC_LITERAL(36, 370, 5), // "ising"
QT_MOC_LITERAL(37, 376, 4), // "beta"
QT_MOC_LITERAL(38, 381, 8), // "linesort"
QT_MOC_LITERAL(39, 390, 6), // "matrix"
QT_MOC_LITERAL(40, 397, 11), // "whichStream"
QT_MOC_LITERAL(41, 409, 4), // "mode"
QT_MOC_LITERAL(42, 414, 10), // "edgedetect"
QT_MOC_LITERAL(43, 425, 9), // "rectangle"
QT_MOC_LITERAL(44, 435, 10), // "sierpinski"
QT_MOC_LITERAL(45, 446, 9), // "tokenDemo"
QT_MOC_LITERAL(46, 456, 14), // "twositecbridge"
QT_MOC_LITERAL(47, 471, 5), // "alpha"
QT_MOC_LITERAL(48, 477, 14), // "twositeebridge"
QT_MOC_LITERAL(49, 492, 9), // "explambda"
QT_MOC_LITERAL(50, 502, 10), // "complambda"
QT_MOC_LITERAL(51, 513, 12), // "siteDistance"
QT_MOC_LITERAL(52, 526, 11), // "faultRepair"
QT_MOC_LITERAL(53, 538, 17), // "boundedObjCoating"
QT_MOC_LITERAL(54, 556, 18), // "numStaticParticles"
QT_MOC_LITERAL(55, 575, 10), // "compaction"
QT_MOC_LITERAL(56, 586, 18), // "holeelimcompaction"
QT_MOC_LITERAL(57, 605, 16), // "holeelimstandard"
QT_MOC_LITERAL(58, 622, 14), // "leaderelection"
QT_MOC_LITERAL(59, 637, 18), // "leaderelectiondemo"
QT_MOC_LITERAL(60, 656, 4), // "line"
QT_MOC_LITERAL(61, 661, 4), // "ring"
QT_MOC_LITERAL(62, 666, 6), // "square"
QT_MOC_LITERAL(63, 673, 8), // "triangle"
QT_MOC_LITERAL(64, 682, 16), // "universalcoating"
QT_MOC_LITERAL(65, 699, 21) // "staticParticlesRadius"

    },
    "ScriptInterface\0log\0\0msg\0error\0runScript\0"
    "scriptFilePath\0writeToFile\0filePath\0"
    "text\0round\0runUntilTermination\0"
    "getNumParticles\0getNumMovements\0"
    "getNumRounds\0getLeaderElectionRounds\0"
    "getWeakBound\0getStrongBound\0"
    "setRoundDuration\0ms\0focusOn\0x\0y\0setZoom\0"
    "zoom\0saveScreenshot\0filmSimulation\0"
    "iterLimit\0adder\0numParticles\0countValue\0"
    "compression\0lambda\0hexagon\0holeProb\0"
    "infObjCoating\0ising\0beta\0linesort\0"
    "matrix\0whichStream\0mode\0edgedetect\0"
    "rectangle\0sierpinski\0tokenDemo\0"
    "twositecbridge\0alpha\0twositeebridge\0"
    "explambda\0complambda\0siteDistance\0"
    "faultRepair\0boundedObjCoating\0"
    "numStaticParticles\0compaction\0"
    "holeelimcompaction\0holeelimstandard\0"
    "leaderelection\0leaderelectiondemo\0"
    "line\0ring\0square\0triangle\0universalcoating\0"
    "staticParticlesRadius"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ScriptInterface[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      93,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,  479,    2, 0x0a /* Public */,
       1,    1,  484,    2, 0x2a /* Public | MethodCloned */,
       5,    1,  487,    2, 0x0a /* Public */,
       7,    2,  490,    2, 0x0a /* Public */,
      10,    0,  495,    2, 0x0a /* Public */,
      11,    0,  496,    2, 0x0a /* Public */,
      12,    0,  497,    2, 0x0a /* Public */,
      13,    0,  498,    2, 0x0a /* Public */,
      14,    0,  499,    2, 0x0a /* Public */,
      15,    0,  500,    2, 0x0a /* Public */,
      16,    0,  501,    2, 0x0a /* Public */,
      17,    0,  502,    2, 0x0a /* Public */,
      18,    1,  503,    2, 0x0a /* Public */,
      20,    2,  506,    2, 0x0a /* Public */,
      23,    1,  511,    2, 0x0a /* Public */,
      25,    1,  514,    2, 0x0a /* Public */,
      25,    0,  517,    2, 0x2a /* Public | MethodCloned */,
      26,    2,  518,    2, 0x0a /* Public */,
      28,    2,  523,    2, 0x0a /* Public */,
      28,    1,  528,    2, 0x2a /* Public | MethodCloned */,
      28,    0,  531,    2, 0x2a /* Public | MethodCloned */,
      31,    2,  532,    2, 0x0a /* Public */,
      31,    1,  537,    2, 0x2a /* Public | MethodCloned */,
      31,    0,  540,    2, 0x2a /* Public | MethodCloned */,
      33,    2,  541,    2, 0x0a /* Public */,
      33,    1,  546,    2, 0x2a /* Public | MethodCloned */,
      33,    0,  549,    2, 0x2a /* Public | MethodCloned */,
      35,    2,  550,    2, 0x0a /* Public */,
      35,    1,  555,    2, 0x2a /* Public | MethodCloned */,
      35,    0,  558,    2, 0x2a /* Public | MethodCloned */,
      36,    2,  559,    2, 0x0a /* Public */,
      36,    1,  564,    2, 0x2a /* Public | MethodCloned */,
      36,    0,  567,    2, 0x2a /* Public | MethodCloned */,
      38,    2,  568,    2, 0x0a /* Public */,
      38,    1,  573,    2, 0x2a /* Public | MethodCloned */,
      38,    0,  576,    2, 0x2a /* Public | MethodCloned */,
      39,    4,  577,    2, 0x0a /* Public */,
      39,    3,  586,    2, 0x2a /* Public | MethodCloned */,
      39,    2,  593,    2, 0x2a /* Public | MethodCloned */,
      39,    1,  598,    2, 0x2a /* Public | MethodCloned */,
      39,    0,  601,    2, 0x2a /* Public | MethodCloned */,
      42,    3,  602,    2, 0x0a /* Public */,
      42,    2,  609,    2, 0x2a /* Public | MethodCloned */,
      42,    1,  614,    2, 0x2a /* Public | MethodCloned */,
      42,    0,  617,    2, 0x2a /* Public | MethodCloned */,
      43,    2,  618,    2, 0x0a /* Public */,
      43,    1,  623,    2, 0x2a /* Public | MethodCloned */,
      43,    0,  626,    2, 0x2a /* Public | MethodCloned */,
      44,    2,  627,    2, 0x0a /* Public */,
      44,    1,  632,    2, 0x2a /* Public | MethodCloned */,
      44,    0,  635,    2, 0x2a /* Public | MethodCloned */,
      45,    2,  636,    2, 0x0a /* Public */,
      45,    1,  641,    2, 0x2a /* Public | MethodCloned */,
      45,    0,  644,    2, 0x2a /* Public | MethodCloned */,
      46,    3,  645,    2, 0x0a /* Public */,
      46,    2,  652,    2, 0x2a /* Public | MethodCloned */,
      46,    1,  657,    2, 0x2a /* Public | MethodCloned */,
      46,    0,  660,    2, 0x2a /* Public | MethodCloned */,
      48,    4,  661,    2, 0x0a /* Public */,
      48,    3,  670,    2, 0x2a /* Public | MethodCloned */,
      48,    2,  677,    2, 0x2a /* Public | MethodCloned */,
      48,    1,  682,    2, 0x2a /* Public | MethodCloned */,
      48,    0,  685,    2, 0x2a /* Public | MethodCloned */,
      52,    2,  686,    2, 0x0a /* Public */,
      52,    1,  691,    2, 0x2a /* Public | MethodCloned */,
      52,    0,  694,    2, 0x2a /* Public | MethodCloned */,
      53,    3,  695,    2, 0x0a /* Public */,
      53,    2,  702,    2, 0x2a /* Public | MethodCloned */,
      55,    1,  707,    2, 0x0a /* Public */,
      55,    0,  710,    2, 0x2a /* Public | MethodCloned */,
      56,    1,  711,    2, 0x0a /* Public */,
      56,    0,  714,    2, 0x2a /* Public | MethodCloned */,
      57,    1,  715,    2, 0x0a /* Public */,
      57,    0,  718,    2, 0x2a /* Public | MethodCloned */,
      58,    1,  719,    2, 0x0a /* Public */,
      58,    0,  722,    2, 0x2a /* Public | MethodCloned */,
      59,    0,  723,    2, 0x0a /* Public */,
      60,    2,  724,    2, 0x0a /* Public */,
      60,    1,  729,    2, 0x2a /* Public | MethodCloned */,
      60,    0,  732,    2, 0x2a /* Public | MethodCloned */,
      61,    2,  733,    2, 0x0a /* Public */,
      61,    1,  738,    2, 0x2a /* Public | MethodCloned */,
      61,    0,  741,    2, 0x2a /* Public | MethodCloned */,
      62,    2,  742,    2, 0x0a /* Public */,
      62,    1,  747,    2, 0x2a /* Public | MethodCloned */,
      62,    0,  750,    2, 0x2a /* Public | MethodCloned */,
      63,    2,  751,    2, 0x0a /* Public */,
      63,    1,  756,    2, 0x2a /* Public | MethodCloned */,
      63,    0,  759,    2, 0x2a /* Public | MethodCloned */,
      64,    3,  760,    2, 0x0a /* Public */,
      64,    2,  767,    2, 0x2a /* Public | MethodCloned */,
      64,    1,  772,    2, 0x2a /* Public | MethodCloned */,
      64,    0,  775,    2, 0x2a /* Public | MethodCloned */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,    4,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    8,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Int,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   21,   22,
    QMetaType::Void, QMetaType::Float,   24,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    8,   27,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   29,   30,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,   29,   32,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::UInt,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,   29,   37,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,   29,   30,   40,   41,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   29,   30,   40,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   29,   30,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   29,   30,   41,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   29,   30,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Float, QMetaType::Float,   29,   32,   47,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,   29,   32,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Float, QMetaType::Float, QMetaType::Float,   29,   49,   50,   51,
    QMetaType::Void, QMetaType::Int, QMetaType::Float, QMetaType::Float,   29,   49,   50,
    QMetaType::Void, QMetaType::Int, QMetaType::Float,   29,   49,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::UInt,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Float,   54,   29,   34,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   54,   29,
    QMetaType::Void, QMetaType::UInt,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt,   29,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::UInt,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::UInt,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::UInt,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt, QMetaType::Float,   29,   34,
    QMetaType::Void, QMetaType::UInt,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Float,   65,   29,   34,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   65,   29,
    QMetaType::Void, QMetaType::Int,   65,
    QMetaType::Void,

       0        // eod
};

void ScriptInterface::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ScriptInterface *_t = static_cast<ScriptInterface *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->log((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->log((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->runScript((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->writeToFile((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 4: _t->round(); break;
        case 5: _t->runUntilTermination(); break;
        case 6: { int _r = _t->getNumParticles();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 7: { int _r = _t->getNumMovements();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 8: { int _r = _t->getNumRounds();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 9: { int _r = _t->getLeaderElectionRounds();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 10: { int _r = _t->getWeakBound();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 11: { int _r = _t->getStrongBound();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 12: _t->setRoundDuration((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->focusOn((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 14: _t->setZoom((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 15: _t->saveScreenshot((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 16: _t->saveScreenshot(); break;
        case 17: _t->filmSimulation((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2]))); break;
        case 18: _t->adder((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 19: _t->adder((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 20: _t->adder(); break;
        case 21: _t->compression((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 22: _t->compression((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->compression(); break;
        case 24: _t->hexagon((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 25: _t->hexagon((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 26: _t->hexagon(); break;
        case 27: _t->infObjCoating((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 28: _t->infObjCoating((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 29: _t->infObjCoating(); break;
        case 30: _t->ising((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 31: _t->ising((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 32: _t->ising(); break;
        case 33: _t->linesort((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 34: _t->linesort((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 35: _t->linesort(); break;
        case 36: _t->matrix((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 37: _t->matrix((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 38: _t->matrix((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 39: _t->matrix((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 40: _t->matrix(); break;
        case 41: _t->edgedetect((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 42: _t->edgedetect((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 43: _t->edgedetect((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 44: _t->edgedetect(); break;
        case 45: _t->rectangle((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 46: _t->rectangle((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 47: _t->rectangle(); break;
        case 48: _t->sierpinski((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 49: _t->sierpinski((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 50: _t->sierpinski(); break;
        case 51: _t->tokenDemo((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 52: _t->tokenDemo((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 53: _t->tokenDemo(); break;
        case 54: _t->twositecbridge((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2])),(*reinterpret_cast< float(*)>(_a[3]))); break;
        case 55: _t->twositecbridge((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 56: _t->twositecbridge((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 57: _t->twositecbridge(); break;
        case 58: _t->twositeebridge((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2])),(*reinterpret_cast< float(*)>(_a[3])),(*reinterpret_cast< float(*)>(_a[4]))); break;
        case 59: _t->twositeebridge((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2])),(*reinterpret_cast< float(*)>(_a[3]))); break;
        case 60: _t->twositeebridge((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 61: _t->twositeebridge((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 62: _t->twositeebridge(); break;
        case 63: _t->faultRepair((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 64: _t->faultRepair((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 65: _t->faultRepair(); break;
        case 66: _t->boundedObjCoating((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2])),(*reinterpret_cast< const float(*)>(_a[3]))); break;
        case 67: _t->boundedObjCoating((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2]))); break;
        case 68: _t->compaction((*reinterpret_cast< const uint(*)>(_a[1]))); break;
        case 69: _t->compaction(); break;
        case 70: _t->holeelimcompaction((*reinterpret_cast< const uint(*)>(_a[1]))); break;
        case 71: _t->holeelimcompaction(); break;
        case 72: _t->holeelimstandard((*reinterpret_cast< const uint(*)>(_a[1]))); break;
        case 73: _t->holeelimstandard(); break;
        case 74: _t->leaderelection((*reinterpret_cast< const uint(*)>(_a[1]))); break;
        case 75: _t->leaderelection(); break;
        case 76: _t->leaderelectiondemo(); break;
        case 77: _t->line((*reinterpret_cast< const uint(*)>(_a[1])),(*reinterpret_cast< const float(*)>(_a[2]))); break;
        case 78: _t->line((*reinterpret_cast< const uint(*)>(_a[1]))); break;
        case 79: _t->line(); break;
        case 80: _t->ring((*reinterpret_cast< const uint(*)>(_a[1])),(*reinterpret_cast< const float(*)>(_a[2]))); break;
        case 81: _t->ring((*reinterpret_cast< const uint(*)>(_a[1]))); break;
        case 82: _t->ring(); break;
        case 83: _t->square((*reinterpret_cast< const uint(*)>(_a[1])),(*reinterpret_cast< const float(*)>(_a[2]))); break;
        case 84: _t->square((*reinterpret_cast< const uint(*)>(_a[1]))); break;
        case 85: _t->square(); break;
        case 86: _t->triangle((*reinterpret_cast< const uint(*)>(_a[1])),(*reinterpret_cast< const float(*)>(_a[2]))); break;
        case 87: _t->triangle((*reinterpret_cast< const uint(*)>(_a[1]))); break;
        case 88: _t->triangle(); break;
        case 89: _t->universalcoating((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2])),(*reinterpret_cast< const float(*)>(_a[3]))); break;
        case 90: _t->universalcoating((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2]))); break;
        case 91: _t->universalcoating((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 92: _t->universalcoating(); break;
        default: ;
        }
    }
}

const QMetaObject ScriptInterface::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ScriptInterface.data,
      qt_meta_data_ScriptInterface,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ScriptInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScriptInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ScriptInterface.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ScriptInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 93)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 93;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 93)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 93;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
