/****************************************************************************
** Meta object code from reading C++ file 'timewindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.1.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "timewindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'timewindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.1.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TimeWindow_t {
    const uint offsetsAndSize[26];
    char stringdata0[162];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_TimeWindow_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_TimeWindow_t qt_meta_stringdata_TimeWindow = {
    {
QT_MOC_LITERAL(0, 10), // "TimeWindow"
QT_MOC_LITERAL(11, 18), // "signalRepaintAllGL"
QT_MOC_LITERAL(30, 0), // ""
QT_MOC_LITERAL(31, 18), // "signalRebindPlants"
QT_MOC_LITERAL(50, 23), // "doCohortMapsAdjustments"
QT_MOC_LITERAL(74, 20), // "setTimestepAndSample"
QT_MOC_LITERAL(95, 10), // "signalSync"
QT_MOC_LITERAL(106, 11), // "updateScene"
QT_MOC_LITERAL(118, 1), // "t"
QT_MOC_LITERAL(120, 11), // "synchronize"
QT_MOC_LITERAL(132, 7), // "advance"
QT_MOC_LITERAL(140, 9), // "backtrack"
QT_MOC_LITERAL(150, 11) // "playControl"

    },
    "TimeWindow\0signalRepaintAllGL\0\0"
    "signalRebindPlants\0doCohortMapsAdjustments\0"
    "setTimestepAndSample\0signalSync\0"
    "updateScene\0t\0synchronize\0advance\0"
    "backtrack\0playControl"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TimeWindow[] = {

 // content:
       9,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x06,    0 /* Public */,
       3,    0,   75,    2, 0x06,    1 /* Public */,
       4,    1,   76,    2, 0x06,    2 /* Public */,
       5,    1,   79,    2, 0x06,    4 /* Public */,
       6,    1,   82,    2, 0x06,    6 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       7,    1,   85,    2, 0x0a,    8 /* Public */,
       9,    1,   88,    2, 0x0a,   10 /* Public */,
      10,    0,   91,    2, 0x0a,   12 /* Public */,
      11,    0,   92,    2, 0x0a,   13 /* Public */,
      12,    0,   93,    2, 0x0a,   14 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void TimeWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TimeWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->signalRepaintAllGL(); break;
        case 1: _t->signalRebindPlants(); break;
        case 2: _t->doCohortMapsAdjustments((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->setTimestepAndSample((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->signalSync((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->updateScene((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->synchronize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->advance(); break;
        case 8: _t->backtrack(); break;
        case 9: _t->playControl(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TimeWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TimeWindow::signalRepaintAllGL)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TimeWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TimeWindow::signalRebindPlants)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TimeWindow::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TimeWindow::doCohortMapsAdjustments)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TimeWindow::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TimeWindow::setTimestepAndSample)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TimeWindow::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TimeWindow::signalSync)) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject TimeWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_TimeWindow.offsetsAndSize,
    qt_meta_data_TimeWindow,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_TimeWindow_t
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *TimeWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TimeWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TimeWindow.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int TimeWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void TimeWindow::signalRepaintAllGL()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void TimeWindow::signalRebindPlants()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TimeWindow::doCohortMapsAdjustments(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void TimeWindow::setTimestepAndSample(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void TimeWindow::signalSync(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
