/****************************************************************************
** Meta object code from reading C++ file 'yolov5detector.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.10)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "yolov5detector.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'yolov5detector.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.10. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_YOLOv5Detector_t {
    QByteArrayData data[18];
    char stringdata0[191];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_YOLOv5Detector_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_YOLOv5Detector_t qt_meta_stringdata_YOLOv5Detector = {
    {
QT_MOC_LITERAL(0, 0, 14), // "YOLOv5Detector"
QT_MOC_LITERAL(1, 15, 17), // "detectionComplete"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 11), // "resultImage"
QT_MOC_LITERAL(4, 46, 13), // "errorOccurred"
QT_MOC_LITERAL(5, 60, 12), // "errorMessage"
QT_MOC_LITERAL(6, 73, 15), // "trackingUpdated"
QT_MOC_LITERAL(7, 89, 4), // "rect"
QT_MOC_LITERAL(8, 94, 10), // "fpsUpdated"
QT_MOC_LITERAL(9, 105, 3), // "fps"
QT_MOC_LITERAL(10, 109, 12), // "detectQImage"
QT_MOC_LITERAL(11, 122, 5), // "image"
QT_MOC_LITERAL(12, 128, 12), // "processFrame"
QT_MOC_LITERAL(13, 141, 5), // "frame"
QT_MOC_LITERAL(14, 147, 12), // "stopTracking"
QT_MOC_LITERAL(15, 160, 10), // "initialize"
QT_MOC_LITERAL(16, 171, 9), // "modelPath"
QT_MOC_LITERAL(17, 181, 9) // "npu_index"

    },
    "YOLOv5Detector\0detectionComplete\0\0"
    "resultImage\0errorOccurred\0errorMessage\0"
    "trackingUpdated\0rect\0fpsUpdated\0fps\0"
    "detectQImage\0image\0processFrame\0frame\0"
    "stopTracking\0initialize\0modelPath\0"
    "npu_index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_YOLOv5Detector[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    1,   57,    2, 0x06 /* Public */,
       6,    1,   60,    2, 0x06 /* Public */,
       8,    1,   63,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    1,   66,    2, 0x0a /* Public */,
      12,    1,   69,    2, 0x0a /* Public */,
      14,    0,   72,    2, 0x0a /* Public */,

 // methods: name, argc, parameters, tag, flags
      15,    2,   73,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QImage,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QRect,    7,
    QMetaType::Void, QMetaType::Double,    9,

 // slots: parameters
    QMetaType::Void, QMetaType::QImage,   11,
    QMetaType::Void, QMetaType::QImage,   13,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Bool, QMetaType::QString, QMetaType::Int,   16,   17,

       0        // eod
};

void YOLOv5Detector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<YOLOv5Detector *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->detectionComplete((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 1: _t->errorOccurred((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->trackingUpdated((*reinterpret_cast< const QRect(*)>(_a[1]))); break;
        case 3: _t->fpsUpdated((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: _t->detectQImage((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 5: _t->processFrame((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 6: _t->stopTracking(); break;
        case 7: { bool _r = _t->initialize((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (YOLOv5Detector::*)(const QImage & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&YOLOv5Detector::detectionComplete)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (YOLOv5Detector::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&YOLOv5Detector::errorOccurred)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (YOLOv5Detector::*)(const QRect & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&YOLOv5Detector::trackingUpdated)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (YOLOv5Detector::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&YOLOv5Detector::fpsUpdated)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject YOLOv5Detector::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_YOLOv5Detector.data,
    qt_meta_data_YOLOv5Detector,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *YOLOv5Detector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *YOLOv5Detector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_YOLOv5Detector.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int YOLOv5Detector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void YOLOv5Detector::detectionComplete(const QImage & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void YOLOv5Detector::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void YOLOv5Detector::trackingUpdated(const QRect & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void YOLOv5Detector::fpsUpdated(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
