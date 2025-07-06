/****************************************************************************
** Meta object code from reading C++ file 'whisperworker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.10)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "whisperworker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'whisperworker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.10. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WhisperWorker_t {
    QByteArrayData data[10];
    char stringdata0[97];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WhisperWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WhisperWorker_t qt_meta_stringdata_WhisperWorker = {
    {
QT_MOC_LITERAL(0, 0, 13), // "WhisperWorker"
QT_MOC_LITERAL(1, 14, 11), // "resultReady"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 6), // "result"
QT_MOC_LITERAL(4, 34, 8), // "finished"
QT_MOC_LITERAL(5, 43, 11), // "processLoop"
QT_MOC_LITERAL(6, 55, 13), // "pushAudioData"
QT_MOC_LITERAL(7, 69, 14), // "QVector<float>"
QT_MOC_LITERAL(8, 84, 7), // "pcmData"
QT_MOC_LITERAL(9, 92, 4) // "stop"

    },
    "WhisperWorker\0resultReady\0\0result\0"
    "finished\0processLoop\0pushAudioData\0"
    "QVector<float>\0pcmData\0stop"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WhisperWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       4,    0,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   43,    2, 0x0a /* Public */,
       6,    1,   44,    2, 0x0a /* Public */,
       9,    0,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void,

       0        // eod
};

void WhisperWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WhisperWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->resultReady((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->finished(); break;
        case 2: _t->processLoop(); break;
        case 3: _t->pushAudioData((*reinterpret_cast< const QVector<float>(*)>(_a[1]))); break;
        case 4: _t->stop(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
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
            using _t = void (WhisperWorker::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WhisperWorker::resultReady)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (WhisperWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WhisperWorker::finished)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject WhisperWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_WhisperWorker.data,
    qt_meta_data_WhisperWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *WhisperWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WhisperWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WhisperWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WhisperWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void WhisperWorker::resultReady(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void WhisperWorker::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
