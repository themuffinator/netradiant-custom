/*
   Copyright (C) 2001-2006, William Joseph.
   All Rights Reserved.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "i18n.h"

#include <utility>

namespace i18n
{
namespace
{
QHash<QString, QString> g_translations;
QString g_language;
}

void setTranslations( QHash<QString, QString> translations, const QString& languageCode ){
	g_translations = std::move( translations );
	g_language = languageCode;
}

QString tr( const char* text ){
	if ( text == nullptr || *text == '\0' ) {
		return {};
	}

	const QString key = QString::fromUtf8( text );
	const auto it = g_translations.constFind( key );
	if ( it != g_translations.constEnd() ) {
		return *it;
	}
	return key;
}

QString tr( const QString& text ){
	if ( text.isEmpty() ) {
		return text;
	}
	const auto it = g_translations.constFind( text );
	if ( it != g_translations.constEnd() ) {
		return *it;
	}
	return text;
}

const QString& language(){
	return g_language;
}
}
