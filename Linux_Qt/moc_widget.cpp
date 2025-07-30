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
    QByteArrayData data[27];
    char stringdata0[377];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Widget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Widget_t qt_meta_stringdata_Widget = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Widget"
QT_MOC_LITERAL(1, 7, 13), // "sendTextToLLM"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 4), // "text"
QT_MOC_LITERAL(4, 27, 11), // "updateFrame"
QT_MOC_LITERAL(5, 39, 5), // "frame"
QT_MOC_LITERAL(6, 45, 17), // "updateInferResult"
QT_MOC_LITERAL(7, 63, 6), // "result"
QT_MOC_LITERAL(8, 70, 11), // "handleError"
QT_MOC_LITERAL(9, 82, 5), // "error"
QT_MOC_LITERAL(10, 88, 14), // "onStartClicked"
QT_MOC_LITERAL(11, 103, 16), // "onWorkerFinished"
QT_MOC_LITERAL(12, 120, 13), // "onSerialError"
QT_MOC_LITERAL(13, 134, 12), // "onPortOpened"
QT_MOC_LITERAL(14, 147, 20), // "onWhisperResultReady"
QT_MOC_LITERAL(15, 168, 17), // "initInferStreamer"
QT_MOC_LITERAL(16, 186, 23), // "onCancelTrackingClicked"
QT_MOC_LITERAL(17, 210, 17), // "on_btnllm_pressed"
QT_MOC_LITERAL(18, 228, 18), // "on_btnllm_released"
QT_MOC_LITERAL(19, 247, 16), // "onLlmResultReady"
QT_MOC_LITERAL(20, 264, 17), // "on_exitbt_clicked"
QT_MOC_LITERAL(21, 282, 21), // "onMqttMessageReceived"
QT_MOC_LITERAL(22, 304, 12), // "QMqttMessage"
QT_MOC_LITERAL(23, 317, 7), // "message"
QT_MOC_LITERAL(24, 325, 24), // "onSummarizeButtonClicked"
QT_MOC_LITERAL(25, 350, 20), // "onRawVideoFrameReady"
QT_MOC_LITERAL(26, 371, 5) // "image"

    },
    "Widget\0sendTextToLLM\0\0text\0updateFrame\0"
    "frame\0updateInferResult\0result\0"
    "handleError\0error\0onStartClicked\0"
    "onWorkerFinished\0onSerialError\0"
    "onPortOpened\0onWhisperResultReady\0"
    "initInferStreamer\0onCancelTrackingClicked\0"
    "on_btnllm_pressed\0on_btnllm_released\0"
    "onLlmResultReady\0on_exitbt_clicked\0"
    "onMqttMessageReceived\0QMqttMessage\0"
    "message\0onSummarizeButtonClicked\0"
    "onRawVideoFrameReady\0image"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Widget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  104,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,  107,    2, 0x08 /* Private */,
       6,    1,  110,    2, 0x08 /* Private */,
       8,    1,  113,    2, 0x08 /* Private */,
      10,    0,  116,    2, 0x08 /* Private */,
      11,    0,  117,    2, 0x08 /* Private */,
      12,    1,  118,    2, 0x08 /* Private */,
      13,    0,  121,    2, 0x08 /* Private */,
      14,    1,  122,    2, 0x08 /* Private */,
      15,    0,  125,    2, 0x08 /* Private */,
      16,    0,  126,    2, 0x08 /* Private */,
      17,    0,  127,    2, 0x08 /* Private */,
      18,    0,  128,    2, 0x08 /* Private */,
      19,    1,  129,    2, 0x08 /* Private */,
      20,    0,  132,    2, 0x08 /* Private */,
      21,    1,  133,    2, 0x08 /* Private */,
      24,    0,  136,    2, 0x08 /* Private */,
      25,    1,  137,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::QImage,    5,
    QMetaType::Void, QMetaType::QImage,    7,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 22,   23,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QImage,   26,

       0        // eod
};

void Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Widget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sendTextToLLM((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->updateFrame((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 2: _t->updateInferResult((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 3: _t->handleError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->onStartClicked(); break;
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
        case 16: _t->onSummarizeButtonClicked(); break;
        case 17: _t->onRawVideoFrameReady((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
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
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Widget::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Widget::sendTextToLLM)) {
                *result = 0;
                return;
            }
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
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void Widget::sendTextToLLM(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
