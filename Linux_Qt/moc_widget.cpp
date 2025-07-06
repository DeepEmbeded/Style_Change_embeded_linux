/****************************************************************************
** Meta object code from reading C++ file 'widget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.10)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "widget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.10. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Widget_t {
    QByteArrayData data[24];
    char stringdata0[325];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Widget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Widget_t qt_meta_stringdata_Widget = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Widget"
QT_MOC_LITERAL(1, 7, 11), // "updateFrame"
QT_MOC_LITERAL(2, 19, 0), // ""
QT_MOC_LITERAL(3, 20, 5), // "frame"
QT_MOC_LITERAL(4, 26, 17), // "updateInferResult"
QT_MOC_LITERAL(5, 44, 6), // "result"
QT_MOC_LITERAL(6, 51, 11), // "handleError"
QT_MOC_LITERAL(7, 63, 5), // "error"
QT_MOC_LITERAL(8, 69, 14), // "onStartClicked"
QT_MOC_LITERAL(9, 84, 13), // "onStopClicked"
QT_MOC_LITERAL(10, 98, 16), // "onWorkerFinished"
QT_MOC_LITERAL(11, 115, 13), // "onSerialError"
QT_MOC_LITERAL(12, 129, 12), // "onPortOpened"
QT_MOC_LITERAL(13, 142, 20), // "onWhisperResultReady"
QT_MOC_LITERAL(14, 163, 4), // "text"
QT_MOC_LITERAL(15, 168, 17), // "initInferStreamer"
QT_MOC_LITERAL(16, 186, 23), // "onCancelTrackingClicked"
QT_MOC_LITERAL(17, 210, 17), // "on_btnllm_pressed"
QT_MOC_LITERAL(18, 228, 18), // "on_btnllm_released"
QT_MOC_LITERAL(19, 247, 16), // "onLlmResultReady"
QT_MOC_LITERAL(20, 264, 17), // "on_exitbt_clicked"
QT_MOC_LITERAL(21, 282, 21), // "onMqttMessageReceived"
QT_MOC_LITERAL(22, 304, 12), // "QMqttMessage"
QT_MOC_LITERAL(23, 317, 7) // "message"

    },
    "Widget\0updateFrame\0\0frame\0updateInferResult\0"
    "result\0handleError\0error\0onStartClicked\0"
    "onStopClicked\0onWorkerFinished\0"
    "onSerialError\0onPortOpened\0"
    "onWhisperResultReady\0text\0initInferStreamer\0"
    "onCancelTrackingClicked\0on_btnllm_pressed\0"
    "on_btnllm_released\0onLlmResultReady\0"
    "on_exitbt_clicked\0onMqttMessageReceived\0"
    "QMqttMessage\0message"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Widget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   94,    2, 0x08 /* Private */,
       4,    1,   97,    2, 0x08 /* Private */,
       6,    1,  100,    2, 0x08 /* Private */,
       8,    0,  103,    2, 0x08 /* Private */,
       9,    0,  104,    2, 0x08 /* Private */,
      10,    0,  105,    2, 0x08 /* Private */,
      11,    1,  106,    2, 0x08 /* Private */,
      12,    0,  109,    2, 0x08 /* Private */,
      13,    1,  110,    2, 0x08 /* Private */,
      15,    0,  113,    2, 0x08 /* Private */,
      16,    0,  114,    2, 0x08 /* Private */,
      17,    0,  115,    2, 0x08 /* Private */,
      18,    0,  116,    2, 0x08 /* Private */,
      19,    1,  117,    2, 0x08 /* Private */,
      20,    0,  120,    2, 0x08 /* Private */,
      21,    1,  121,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QImage,    3,
    QMetaType::Void, QMetaType::QImage,    5,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 22,   23,

       0        // eod
};

void Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Widget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->updateFrame((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 1: _t->updateInferResult((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 2: _t->handleError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->onStartClicked(); break;
        case 4: _t->onStopClicked(); break;
        case 5: _t->onWorkerFinished(); break;
        case 6: _t->onSerialError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->onPortOpened(); break;
        case 8: _t->onWhisperResultReady((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->initInferStreamer(); break;
        case 10: _t->onCancelTrackingClicked(); break;
        case 11: _t->on_btnllm_pressed(); break;
        case 12: _t->on_btnllm_released(); break;
        case 13: _t->onLlmResultReady((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->on_exitbt_clicked(); break;
        case 15: _t->onMqttMessageReceived((*reinterpret_cast< const QMqttMessage(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 15:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QMqttMessage >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Widget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Widget.data,
    qt_meta_data_Widget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Widget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
