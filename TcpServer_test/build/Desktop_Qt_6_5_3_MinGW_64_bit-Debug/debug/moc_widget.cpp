/****************************************************************************
** Meta object code from reading C++ file 'widget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../widget.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.3. It"
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
struct qt_meta_stringdata_CLASSWidgetENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSWidgetENDCLASS = QtMocHelpers::stringData(
    "Widget",
    "on_newClient_connect",
    "",
    "on_readyRead_handler",
    "mdisconnected",
    "mComboBox_refresh",
    "on_btnListen_clicked",
    "on_btnSend_clicked",
    "on_comboBoxChildren_activated",
    "index",
    "on_btnStopListen_clicked",
    "on_btnLineout_clicked",
    "on_pushButton_clear_clicked"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSWidgetENDCLASS_t {
    uint offsetsAndSizes[26];
    char stringdata0[7];
    char stringdata1[21];
    char stringdata2[1];
    char stringdata3[21];
    char stringdata4[14];
    char stringdata5[18];
    char stringdata6[21];
    char stringdata7[19];
    char stringdata8[30];
    char stringdata9[6];
    char stringdata10[25];
    char stringdata11[22];
    char stringdata12[28];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSWidgetENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSWidgetENDCLASS_t qt_meta_stringdata_CLASSWidgetENDCLASS = {
    {
        QT_MOC_LITERAL(0, 6),  // "Widget"
        QT_MOC_LITERAL(7, 20),  // "on_newClient_connect"
        QT_MOC_LITERAL(28, 0),  // ""
        QT_MOC_LITERAL(29, 20),  // "on_readyRead_handler"
        QT_MOC_LITERAL(50, 13),  // "mdisconnected"
        QT_MOC_LITERAL(64, 17),  // "mComboBox_refresh"
        QT_MOC_LITERAL(82, 20),  // "on_btnListen_clicked"
        QT_MOC_LITERAL(103, 18),  // "on_btnSend_clicked"
        QT_MOC_LITERAL(122, 29),  // "on_comboBoxChildren_activated"
        QT_MOC_LITERAL(152, 5),  // "index"
        QT_MOC_LITERAL(158, 24),  // "on_btnStopListen_clicked"
        QT_MOC_LITERAL(183, 21),  // "on_btnLineout_clicked"
        QT_MOC_LITERAL(205, 27)   // "on_pushButton_clear_clicked"
    },
    "Widget",
    "on_newClient_connect",
    "",
    "on_readyRead_handler",
    "mdisconnected",
    "mComboBox_refresh",
    "on_btnListen_clicked",
    "on_btnSend_clicked",
    "on_comboBoxChildren_activated",
    "index",
    "on_btnStopListen_clicked",
    "on_btnLineout_clicked",
    "on_pushButton_clear_clicked"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSWidgetENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x0a,    1 /* Public */,
       3,    0,   75,    2, 0x0a,    2 /* Public */,
       4,    0,   76,    2, 0x0a,    3 /* Public */,
       5,    0,   77,    2, 0x0a,    4 /* Public */,
       6,    0,   78,    2, 0x08,    5 /* Private */,
       7,    0,   79,    2, 0x08,    6 /* Private */,
       8,    1,   80,    2, 0x08,    7 /* Private */,
      10,    0,   83,    2, 0x08,    9 /* Private */,
      11,    0,   84,    2, 0x08,   10 /* Private */,
      12,    0,   85,    2, 0x08,   11 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject Widget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Widget, std::true_type>,
        // method 'on_newClient_connect'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_readyRead_handler'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'mdisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'mComboBox_refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnListen_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnSend_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_comboBoxChildren_activated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'on_btnStopListen_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnLineout_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButton_clear_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Widget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->on_newClient_connect(); break;
        case 1: _t->on_readyRead_handler(); break;
        case 2: _t->mdisconnected(); break;
        case 3: _t->mComboBox_refresh(); break;
        case 4: _t->on_btnListen_clicked(); break;
        case 5: _t->on_btnSend_clicked(); break;
        case 6: _t->on_comboBoxChildren_activated((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->on_btnStopListen_clicked(); break;
        case 8: _t->on_btnLineout_clicked(); break;
        case 9: _t->on_pushButton_clear_clicked(); break;
        default: ;
        }
    }
}

const QMetaObject *Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_WARNING_POP
