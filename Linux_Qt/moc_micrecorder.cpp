/****************************************************************************
** Meta object code from reading C++ file 'micrecorder.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.10)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "micrecorder.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'micrecorder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.10. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MicRecorder_t {
    QByteArrayData data[10];
    char stringdata0[97];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MicRecorder_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MicRecorder_t qt_meta_stringdata_MicRecorder = {
    {
QT_MOC_LITERAL(0, 0, 11), // "MicRecorder"
QT_MOC_LITERAL(1, 12, 17), // "audioSegmentReady"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 14), // "QVector<float>"
QT_MOC_LITERAL(4, 46, 7), // "pcmData"
QT_MOC_LITERAL(5, 54, 13), // "pcmFrameReady"
QT_MOC_LITERAL(6, 68, 11), // "const char*"
QT_MOC_LITERAL(7, 80, 4), // "data"
QT_MOC_LITERAL(8, 85, 6), // "length"
QT_MOC_LITERAL(9, 92, 4) // "stop"

    },
    "MicRecorder\0audioSegmentReady\0\0"
    "QVector<float>\0pcmData\0pcmFrameReady\0"
    "const char*\0data\0length\0stop"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MicRecorder[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       5,    2,   32,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   37,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6, QMetaType::Int,    7,    8,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void MicRecorder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MicRecorder *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->audioSegmentReady((*reinterpret_cast< const QVector<float>(*)>(_a[1]))); break;
        case 1: _t->pcmFrameReady((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->stop(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<float> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MicRecorder::*)(const QVector<float> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MicRecorder::audioSegmentReady)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MicRecorder::*)(const char * , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MicRecorder::pcmFrameReady)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MicRecorder::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_MicRecorder.data,
    qt_meta_data_MicRecorder,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MicRecorder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MicRecorder::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MicRecorder.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MicRecorder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void MicRecorder::audioSegmentReady(const QVector<float> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MicRecorder::pcmFrameReady(const char * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
