/*
    This file is part of LibQtLua.

    LibQtLua is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LibQtLua is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LibQtLua.  If not, see <http://www.gnu.org/licenses/>.

    Copyright (C) 2008, Alexandre Becoulet <alexandre.becoulet@free.fr>

*/

#ifndef QTLUASTRING_HH_
#define QTLUASTRING_HH_

#include <QString>
#include <QByteArray>

namespace QtLua {

/** @internal */
#define QTLUA_THROW(context, str, ...) do { throw QtLua::String(#context ":" str) __VA_ARGS__; } while (0)

  /**
   * @short Character string class
   * @header QtLua/String
   * @module {Base}
   *
   * Lua use 8 bits character strings so @ref QString is not suitable
   * for QtLua.
   *
   * This class is used as string object in the QtLua project. It is
   * based on @ref QByteArray with added conversion functions.
   *
   * This class is also used as exceptions type for exceptions
   * associated with lua errors.
   */
  class String : public QByteArray
  {
  public:
    /** Create an empty string */
    inline String();
    /** Create a string from @tt{const char *} */
    inline String(const char *s);
    /** Create a string from @tt{const char *} with given length */
    inline String(const char *s, int size);
    /** Create a string from @ref QByteArray */
    inline String(const QByteArray &s);
    /** Copy constructor */
    inline String(const QString &s);
    /** Replace next @tt % character in string with given string  */
    inline String & arg(const QByteArray &arg);
    /** Replace next @tt % character in string with given string  */
    inline String & arg(const QString &arg);
    /** Replace next @tt % character in string with given integer  */
    inline String & arg(const char *arg);
    /** Replace next @tt % character in string with given integer  */
    inline String & arg(int arg);
    /** @tt{const char *} cast operator */
    inline operator const char * () const;
    /** @ref QString cast operator */
    inline QString to_qstring() const;
  };

  String::String()
  {
  }

  String::String(const char *s)
    : QByteArray(s)
  {
  }

  String::String(const char *s, int size)
    : QByteArray(s, size)
  {
  }

  String::String(const QByteArray &s)
    : QByteArray(s)
  {
  }

  String::String(const QString &s)
    : QByteArray(s.toUtf8())
  {
  }

  String & String::arg(const QByteArray &a)
  {
    int i = indexOf('%');

    if (i >= 0)
      replace(i, 1, a);

    return *this;
  }

  String & String::arg(const QString &a)
  {
    int i = indexOf('%');

    if (i >= 0)
      replace(i, 1, a.toUtf8());

    return *this;
  }

  String & String::arg(const char *a)
  {
    int i = indexOf('%');

    if (i >= 0)
      replace(i, 1, a);

    return *this;
  }

  String & String::arg(int a)
  {
    int i = indexOf('%');

    if (i >= 0)
      replace(i, 1, QByteArray::number(a));

    return *this;
  }

  String::operator const char * () const
  {
    return constData();
  }

  QString String::to_qstring () const
  {
    return QString::fromUtf8(constData(), size());
  }
}

#endif

