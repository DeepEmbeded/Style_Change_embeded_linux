/****************************************************************************
** Meta object code from reading C++ file 'llmworker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.10)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "llmworker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'llmworker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.10. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LLMWorker_t {
    QByteArrayData data[12];
    char stringdata0[96];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LLMWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LLMWorker_t qt_meta_stringdata_LLMWorker = {
    {
QT_MOC_LITERAL(0, 0, 9), // "LLMWorker"
QT_MOC_LITERAL(1, 10, 14), // "llmResultReady"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 4), // "text"
QT_MOC_LITERAL(4, 31, 11), // "initialized"
QT_MOC_LITERAL(5, 43, 7), // "success"
QT_MOC_LITERAL(6, 51, 4), // "init"
QT_MOC_LITERAL(7, 56, 6), // "deinit"
QT_MOC_LITERAL(8, 63, 9), // "runPrompt"
QT_MOC_LITERAL(9, 73, 6), // "prompt"
QT_MOC_LITERAL(10, 80, 10), // "clearCache"
QT_MOC_LITERAL(11, 91, 4) // "stop"

    },
    "LLMWorker\0llmResultReady\0\0text\0"
    "initialized\0success\0init\0deinit\0"
    "runPrompt\0prompt\0clearCache\0stop"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LLMWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       4,    1,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   55,    2, 0x0a /* Public */,
       7,    0,   56,    2, 0x0a /* Public */,
       8,    1,   57,    2, 0x0a /* Public */,
      10,    0,   60,    2, 0x0a /* Public */,
      11,    0,   61,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Bool,    5,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void LLMWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LLMWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->llmResultReady((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->initialized((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->init(); break;
        case 3: _t->deinit(); break;
        case 4: _t->runPrompt((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->clearCache(); break;
        case 6: _t->stop(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LLMWorker::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LLMWorker::llmResultReady)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LLMWorker::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LLMWorker::initialized)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject LLMWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_LLMWorker.data,
    qt_meta_data_LLMWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LLMWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LLMWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LLMWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LLMWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void LLMWorker::llmResultReady(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LLMWorker::initialized(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
