/****************************************************************************
** Meta object code from reading C++ file 'window.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "window.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.2. It"
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

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSPlantPanelENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSPlantPanelENDCLASS = QtMocHelpers::stringData(
    "PlantPanel"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSPlantPanelENDCLASS[] = {

 // content:
      12,       // revision
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

Q_CONSTINIT const QMetaObject PlantPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSPlantPanelENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSPlantPanelENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSPlantPanelENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PlantPanel, std::true_type>
    >,
    nullptr
} };

void PlantPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *PlantPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlantPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSPlantPanelENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int PlantPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSDataMapPanelENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSDataMapPanelENDCLASS = QtMocHelpers::stringData(
    "DataMapPanel"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDataMapPanelENDCLASS[] = {

 // content:
      12,       // revision
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

Q_CONSTINIT const QMetaObject DataMapPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSDataMapPanelENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDataMapPanelENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDataMapPanelENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DataMapPanel, std::true_type>
    >,
    nullptr
} };

void DataMapPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *DataMapPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DataMapPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDataMapPanelENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DataMapPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSWindowENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSWindowENDCLASS = QtMocHelpers::stringData(
    "Window",
    "repaintAllGL",
    "",
    "transectSyncPlace",
    "firstplace",
    "timelineSync",
    "t",
    "showRenderOptions",
    "showPlantOptions",
    "showDataMapOptions",
    "showContours",
    "show",
    "showGridLines",
    "exportMitsuba",
    "exportMitsubaJSON",
    "lineEditChange",
    "mapChange",
    "on",
    "cameraChange",
    "idx",
    "plantChange",
    "allPlantsOn",
    "allPlantsOff",
    "uncheckPlantPanel",
    "leftDataMapChoice",
    "id",
    "rightDataMapChoice",
    "leftRampChoice",
    "rightRampChoice",
    "uncheckDataMapPanel",
    "syncDataMapPanel",
    "lockViewsFromLeft",
    "lockViewsFromRight",
    "lockTransectFromLeft",
    "lockTransectFromRight",
    "lockTimelineFromLeft",
    "lockTimelineFromRight",
    "showTransectViews",
    "clearTransects",
    "extractNewSubTerrain",
    "sceneIdx",
    "x0",
    "y0",
    "x1",
    "y1"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSWindowENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      32,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  206,    2, 0x0a,    1 /* Public */,
       3,    1,  207,    2, 0x0a,    2 /* Public */,
       5,    1,  210,    2, 0x0a,    4 /* Public */,
       7,    0,  213,    2, 0x0a,    6 /* Public */,
       8,    0,  214,    2, 0x0a,    7 /* Public */,
       9,    0,  215,    2, 0x0a,    8 /* Public */,
      10,    1,  216,    2, 0x0a,    9 /* Public */,
      12,    1,  219,    2, 0x0a,   11 /* Public */,
      13,    0,  222,    2, 0x0a,   13 /* Public */,
      14,    0,  223,    2, 0x0a,   14 /* Public */,
      15,    0,  224,    2, 0x0a,   15 /* Public */,
      16,    1,  225,    2, 0x0a,   16 /* Public */,
      18,    1,  228,    2, 0x0a,   18 /* Public */,
      20,    1,  231,    2, 0x0a,   20 /* Public */,
      21,    0,  234,    2, 0x0a,   22 /* Public */,
      22,    0,  235,    2, 0x0a,   23 /* Public */,
      23,    0,  236,    2, 0x0a,   24 /* Public */,
      24,    1,  237,    2, 0x0a,   25 /* Public */,
      26,    1,  240,    2, 0x0a,   27 /* Public */,
      27,    1,  243,    2, 0x0a,   29 /* Public */,
      28,    1,  246,    2, 0x0a,   31 /* Public */,
      29,    0,  249,    2, 0x0a,   33 /* Public */,
      30,    0,  250,    2, 0x0a,   34 /* Public */,
      31,    0,  251,    2, 0x0a,   35 /* Public */,
      32,    0,  252,    2, 0x0a,   36 /* Public */,
      33,    0,  253,    2, 0x0a,   37 /* Public */,
      34,    0,  254,    2, 0x0a,   38 /* Public */,
      35,    0,  255,    2, 0x0a,   39 /* Public */,
      36,    0,  256,    2, 0x0a,   40 /* Public */,
      37,    0,  257,    2, 0x0a,   41 /* Public */,
      38,    0,  258,    2, 0x0a,   42 /* Public */,
      39,    5,  259,    2, 0x0a,   43 /* Public */,

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

Q_CONSTINIT const QMetaObject Window::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_CLASSWindowENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSWindowENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSWindowENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Window, std::true_type>,
        // method 'repaintAllGL'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'transectSyncPlace'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'timelineSync'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'showRenderOptions'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'showPlantOptions'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'showDataMapOptions'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'showContours'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'showGridLines'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'exportMitsuba'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'exportMitsubaJSON'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'lineEditChange'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'mapChange'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'cameraChange'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'plantChange'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'allPlantsOn'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'allPlantsOff'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'uncheckPlantPanel'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'leftDataMapChoice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'rightDataMapChoice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'leftRampChoice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'rightRampChoice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'uncheckDataMapPanel'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'syncDataMapPanel'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'lockViewsFromLeft'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'lockViewsFromRight'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'lockTransectFromLeft'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'lockTransectFromRight'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'lockTimelineFromLeft'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'lockTimelineFromRight'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'showTransectViews'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'clearTransects'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'extractNewSubTerrain'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void Window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Window *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->repaintAllGL(); break;
        case 1: _t->transectSyncPlace((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->timelineSync((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->showRenderOptions(); break;
        case 4: _t->showPlantOptions(); break;
        case 5: _t->showDataMapOptions(); break;
        case 6: _t->showContours((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->showGridLines((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->exportMitsuba(); break;
        case 9: _t->exportMitsubaJSON(); break;
        case 10: _t->lineEditChange(); break;
        case 11: _t->mapChange((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->cameraChange((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->plantChange((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->allPlantsOn(); break;
        case 15: _t->allPlantsOff(); break;
        case 16: _t->uncheckPlantPanel(); break;
        case 17: _t->leftDataMapChoice((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->rightDataMapChoice((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->leftRampChoice((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 20: _t->rightRampChoice((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
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
        case 31: _t->extractNewSubTerrain((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[5]))); break;
        default: ;
        }
    }
}

const QMetaObject *Window::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Window::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSWindowENDCLASS.stringdata0))
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
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 32;
    }
    return _id;
}
QT_WARNING_POP
