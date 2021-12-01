#pragma once

#include <QtGlobal>
#include <QList>
#include <QVector>
#include <functional>

/**
@file Common stuff for the whole app.
*/



// Lambda function definitions
using FuncVoidVoid = std::function<void(void)>;	///< Sometimes you just need this
using FuncSetFloat = std::function<void(float)>;
using FuncGetFloat = std::function<float(void)>;
using FuncSetDouble = std::function<void(double)>;
using FuncGetDouble = std::function<double(void)>;
using FuncSetBool = std::function<void(bool)>;
using FuncGetBool = std::function<bool(void)>;
using FuncSetInt = std::function<void(int)>;
using FuncGetInt = std::function<int(void)>;
using FuncTxDouble = std::function<double(double)>;




// Handy bit macros
#define MAX_4BIT				((1 << 4)-1)
#define MAX_5BIT				((1 << 5)-1)
#define MAX_7BIT				((1 << 7)-1)
#define MAX_8BIT				((1 << 8)-1)
#define MAX_10BIT				((1 << 10)-1)
#define MAX_11BIT				((1 << 11)-1)
#define MAX_12BIT				((1 << 12)-1)
#define MAX_14BIT				((1 << 14)-1)
#define MAX_16BIT				((1 << 16)-1)

#define NO_TIMEOUT				-1
