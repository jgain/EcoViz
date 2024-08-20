/****************************************************************************
** Meta object code from reading C++ file 'window.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "window.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PlantPanel_t {
    QByteArrayData data[1];
    char stringdata0[11];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PlantPanel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PlantPanel_t qt_meta_stringdata_PlantPanel = {
    {
QT_MOC_LITERAL(0, 0, 10) // "PlantPanel"

    },
    "PlantPanel"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PlantPanel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void PlantPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject PlantPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_PlantPanel.data,
    qt_meta_data_PlantPanel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PlantPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlantPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlantPanel.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int PlantPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_DataMapPanel_t {
    QByteArrayData data[1];
    char stringdata0[13];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DataMapPanel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DataMapPanel_t qt_meta_stringdata_DataMapPanel = {
    {
QT_MOC_LITERAL(0, 0, 12) // "DataMapPanel"

    },
    "DataMapPanel"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DataMapPanel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void DataMapPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject DataMapPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_DataMapPanel.data,
    qt_meta_data_DataMapPanel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DataMapPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DataMapPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DataMapPanel.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DataMapPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_Window_t {
    QByteArrayData data[45];
    char stringdata0[589];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Window_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Window_t qt_meta_stringdata_Window = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Window"
QT_MOC_LITERAL(1, 7, 12), // "repaintAllGL"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 17), // "transectSyncPlace"
QT_MOC_LITERAL(4, 39, 10), // "firstplace"
QT_MOC_LITERAL(5, 50, 12), // "timelineSync"
QT_MOC_LITERAL(6, 63, 1), // "t"
QT_MOC_LITERAL(7, 65, 17), // "showRenderOptions"
QT_MOC_LITERAL(8, 83, 16), // "showPlantOptions"
QT_MOC_LITERAL(9, 100, 18), // "showDataMapOptions"
QT_MOC_LITERAL(10, 119, 12), // "showContours"
QT_MOC_LITERAL(11, 132, 4), // "show"
QT_MOC_LITERAL(12, 137, 13), // "showGridLines"
QT_MOC_LITERAL(13, 151, 13), // "exportMitsuba"
QT_MOC_LITERAL(14, 165, 17), // "exportMitsubaJSON"
QT_MOC_LITERAL(15, 183, 14), // "lineEditChange"
QT_MOC_LITERAL(16, 198, 9), // "mapChange"
QT_MOC_LITERAL(17, 208, 2), // "on"
QT_MOC_LITERAL(18, 211, 12), // "cameraChange"
QT_MOC_LITERAL(19, 224, 3), // "idx"
QT_MOC_LITERAL(20, 228, 11), // "plantChange"
QT_MOC_LITERAL(21, 240, 11), // "allPlantsOn"
QT_MOC_LITERAL(22, 252, 12), // "allPlantsOff"
QT_MOC_LITERAL(23, 265, 17), // "uncheckPlantPanel"
QT_MOC_LITERAL(24, 283, 17), // "leftDataMapChoice"
QT_MOC_LITERAL(25, 301, 2), // "id"
QT_MOC_LITERAL(26, 304, 18), // "rightDataMapChoice"
QT_MOC_LITERAL(27, 323, 14), // "leftRampChoice"
QT_MOC_LITERAL(28, 338, 15), // "rightRampChoice"
QT_MOC_LITERAL(29, 354, 19), // "uncheckDataMapPanel"
QT_MOC_LITERAL(30, 374, 16), // "syncDataMapPanel"
QT_MOC_LITERAL(31, 391, 17), // "lockViewsFromLeft"
QT_MOC_LITERAL(32, 409, 18), // "lockViewsFromRight"
QT_MOC_LITERAL(33, 428, 20), // "lockTransectFromLeft"
QT_MOC_LITERAL(34, 449, 21), // "lockTransectFromRight"
QT_MOC_LITERAL(35, 471, 20), // "lockTimelineFromLeft"
QT_MOC_LITERAL(36, 492, 21), // "lockTimelineFromRight"
QT_MOC_LITERAL(37, 514, 17), // "showTransectViews"
QT_MOC_LITERAL(38, 532, 14), // "clearTransects"
QT_MOC_LITERAL(39, 547, 20), // "extractNewSubTerrain"
QT_MOC_LITERAL(40, 568, 8), // "sceneIdx"
QT_MOC_LITERAL(41, 577, 2), // "x0"
QT_MOC_LITERAL(42, 580, 2), // "y0"
QT_MOC_LITERAL(43, 583, 2), // "x1"
QT_MOC_LITERAL(44, 586, 2) // "y1"

    },
    "Window\0repaintAllGL\0\0transectSyncPlace\0"
    "firstplace\0timelineSync\0t\0showRenderOptions\0"
    "showPlantOptions\0showDataMapOptions\0"
    "showContours\0show\0showGridLines\0"
    "exportMitsuba\0exportMitsubaJSON\0"
    "lineEditChange\0mapChange\0on\0cameraChange\0"
    "idx\0plantChange\0allPlantsOn\0allPlantsOff\0"
    "uncheckPlantPanel\0leftDataMapChoice\0"
    "id\0rightDataMapChoice\0leftRampChoice\0"
    "rightRampChoice\0uncheckDataMapPanel\0"
    "syncDataMapPanel\0lockViewsFromLeft\0"
    "lockViewsFromRight\0lockTransectFromLeft\0"
    "lockTransectFromRight\0lockTimelineFromLeft\0"
    "lockTimelineFromRight\0showTransectViews\0"
    "clearTransects\0extractNewSubTerrain\0"
    "sceneIdx\0x0\0y0\0x1\0y1"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Window[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      32,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  174,    2, 0x0a /* Public */,
       3,    1,  175,    2, 0x0a /* Public */,
       5,    1,  178,    2, 0x0a /* Public */,
       7,    0,  181,    2, 0x0a /* Public */,
       8,    0,  182,    2, 0x0a /* Public */,
       9,    0,  183,    2, 0x0a /* Public */,
      10,    1,  184,    2, 0x0a /* Public */,
      12,    1,  187,    2, 0x0a /* Public */,
      13,    0,  190,    2, 0x0a /* Public */,
      14,    0,  191,    2, 0x0a /* Public */,
      15,    0,  192,    2, 0x0a /* Public */,
      16,    1,  193,    2, 0x0a /* Public */,
      18,    1,  196,    2, 0x0a /* Public */,
      20,    1,  199,    2, 0x0a /* Public */,
      21,    0,  202,    2, 0x0a /* Public */,
      22,    0,  203,    2, 0x0a /* Public */,
      23,    0,  204,    2, 0x0a /* Public */,
      24,    1,  205,    2, 0x0a /* Public */,
      26,    1,  208,    2, 0x0a /* Public */,
      27,    1,  211,    2, 0x0a /* Public */,
      28,    1,  214,    2, 0x0a /* Public */,
      29,    0,  217,    2, 0x0a /* Public */,
      30,    0,  218,    2, 0x0a /* Public */,
      31,    0,  219,    2, 0x0a /* Public */,
      32,    0,  220,    2, 0x0a /* Public */,
      33,    0,  221,    2, 0x0a /* Public */,
      34,    0,  222,    2, 0x0a /* Public */,
      35,    0,  223,    2, 0x0a /* Public */,
      36,    0,  224,    2, 0x0a /* Public */,
      37,    0,  225,    2, 0x0a /* Public */,
      38,    0,  226,    2, 0x0a /* Public */,
      39,    5,  227,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   25,
    QMetaType::Void, QMetaType::Int,   25,
    QMetaType::Void, QMetaType::Int,   25,
    QMetaType::Void, QMetaType::Int,   25,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,   40,   41,   42,   43,   44,

       0        // eod
};

void Window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Window *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->repaintAllGL(); break;
        case 1: _t->transectSyncPlace((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->timelineSync((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->showRenderOptions(); break;
        case 4: _t->showPlantOptions(); break;
        case 5: _t->showDataMapOptions(); break;
        case 6: _t->showContours((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->showGridLines((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->exportMitsuba(); break;
        case 9: _t->exportMitsubaJSON(); break;
        case 10: _t->lineEditChange(); break;
        case 11: _t->mapChange((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->cameraChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->plantChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->allPlantsOn(); break;
        case 15: _t->allPlantsOff(); break;
        case 16: _t->uncheckPlantPanel(); break;
        case 17: _t->leftDataMapChoice((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: _t->rightDataMapChoice((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: _t->leftRampChoice((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 20: _t->rightRampChoice((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: _t->uncheckDataMapPanel(); break;
        case 22: _t->syncDataMapPanel(); break;
        case 23: _t->lockViewsFromLeft(); break;
        case 24: _t->lockViewsFromRight(); break;
        case 25: _t->lockTransectFromLeft(); break;
        case 26: _t->lockTransectFromRight(); break;
        case 27: _t->lockTimelineFromLeft(); break;
        case 28: _t->lockTimelineFromRight(); break;
        case 29: _t->showTransectViews(); break;
        case 30: _t->clearTransects(); break;
        case 31: _t->extractNewSubTerrain((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Window::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_Window.data,
    qt_meta_data_Window,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Window::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Window::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Window.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int Window::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 32)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 32;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 32)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 32;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
