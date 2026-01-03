/*
   Copyright (C) 1999-2006 Id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

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

#include "localization.h"

#include "gtkutil/i18n.h"
#include "mainframe.h"
#include "preferences.h"
#include "preferencesystem.h"
#include "stream/stringstream.h"
#include "stringio.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QStringList>

#include <utility>

namespace
{
struct LanguageOption
{
	const char* code;
	const char* name;
};

constexpr LanguageOption kLanguageOptions[] = {
	{ "auto", "Auto (System)" },
	{ "en", "English" },
	{ "fr", "French" },
	{ "de", "German" },
	{ "pl", "Polish" },
	{ "es", "Spanish" },
	{ "it", "Italian" },
	{ "pt", "Portuguese" },
	{ "ru", "Russian" },
	{ "uk", "Ukrainian" },
	{ "cs", "Czech" },
	{ "sk", "Slovak" },
	{ "hu", "Hungarian" },
	{ "tr", "Turkish" },
	{ "nl", "Dutch" },
	{ "sv", "Swedish" },
	{ "nb", "Norwegian Bokmal" },
	{ "da", "Danish" },
	{ "fi", "Finnish" },
	{ "ja", "Japanese" },
	{ "zh", "Chinese (Simplified)" },
};

constexpr int kLanguageCount = static_cast<int>( sizeof( kLanguageOptions ) / sizeof( kLanguageOptions[0] ) );

static const char* const kLanguageNames[] = {
	"Auto (System)",
	"English",
	"French",
	"German",
	"Polish",
	"Spanish",
	"Italian",
	"Portuguese",
	"Russian",
	"Ukrainian",
	"Czech",
	"Slovak",
	"Hungarian",
	"Turkish",
	"Dutch",
	"Swedish",
	"Norwegian Bokmal",
	"Danish",
	"Finnish",
	"Japanese",
	"Chinese (Simplified)",
};

static_assert( kLanguageCount == static_cast<int>( sizeof( kLanguageNames ) / sizeof( kLanguageNames[0] ) ),
               "language name list must match language option list" );

LatchedInt g_language_option( 0, "Language" );

int clamp_language_index( int value ){
	if ( value < 0 || value >= kLanguageCount ) {
		return 0;
	}
	return value;
}

QString normalize_code( QString code ){
	code = code.trimmed().toLower();
	code.replace( '_', '-' );
	return code;
}

bool is_supported_code( const QString& code ){
	for ( int i = 1; i < kLanguageCount; ++i ) {
		if ( code == kLanguageOptions[i].code ) {
			return true;
		}
	}
	return false;
}

QString match_supported_code( const QString& rawCode ){
	const QString normalized = normalize_code( rawCode );
	if ( normalized.isEmpty() ) {
		return {};
	}
	if ( is_supported_code( normalized ) ) {
		return normalized;
	}
	const QString base = normalized.section( '-', 0, 0 );
	if ( base == "no" && is_supported_code( "nb" ) ) {
		return "nb";
	}
	if ( is_supported_code( base ) ) {
		return base;
	}
	return {};
}

QString resolve_system_language(){
	const QStringList uiLanguages = QLocale::system().uiLanguages();
	for ( const auto& language : uiLanguages ) {
		if ( const QString match = match_supported_code( language ); !match.isEmpty() ) {
			return match;
		}
	}

	if ( const QString match = match_supported_code( QLocale::system().name() ); !match.isEmpty() ) {
		return match;
	}

	return "en";
}

QString resolve_language_code(){
	const int index = clamp_language_index( g_language_option.m_value );
	if ( index == 0 ) {
		return resolve_system_language();
	}
	return kLanguageOptions[index].code;
}

QHash<QString, QString> load_translations( const QString& code ){
	const auto path = StringStream( AppPath_get(), "i18n/", code.toLatin1().constData(), ".json" );
	QFile file( QString::fromUtf8( path.c_str() ) );
	if ( !file.open( QIODevice::ReadOnly ) ) {
		return {};
	}

	const QByteArray data = file.readAll();
	QJsonParseError error{};
	const QJsonDocument doc = QJsonDocument::fromJson( data, &error );
	if ( error.error != QJsonParseError::NoError || !doc.isObject() ) {
		return {};
	}

	QHash<QString, QString> out;
	const QJsonObject obj = doc.object();
	for ( auto it = obj.begin(); it != obj.end(); ++it ) {
		if ( it.key().startsWith( "_" ) ) {
			continue;
		}
		if ( it.value().isString() ) {
			out.insert( it.key(), it.value().toString() );
		}
	}
	return out;
}

void apply_language(){
	const QString code = resolve_language_code();
	if ( code == "en" ) {
		i18n::setTranslations( {}, "en" );
		return;
	}

	QHash<QString, QString> translations = load_translations( code );
	if ( translations.isEmpty() ) {
		i18n::setTranslations( {}, "en" );
		return;
	}

	i18n::setTranslations( std::move( translations ), code );
}

void LanguagePreferenceAssign( int value ){
	g_language_option.assign( clamp_language_index( value ) );
	apply_language();
}

void LanguagePreferenceImport( int value ){
	g_language_option.import( clamp_language_index( value ) );
}
}

void Localization_init(){
	apply_language();
}

void Localization_registerGlobalPreference( PreferenceSystem& preferences ){
	preferences.registerPreference(
	    "Language",
	    makeIntStringImportCallback( FreeCaller<void( int ), LanguagePreferenceAssign>() ),
	    IntExportStringCaller( g_language_option.m_latched )
	);
}

void Localization_constructPreferences( PreferencesPage& page, bool applyImmediately ){
	const auto importCallback = applyImmediately
		? IntImportCallback( FreeCaller<void( int ), LanguagePreferenceAssign>() )
		: IntImportCallback( FreeCaller<void( int ), LanguagePreferenceImport>() );

	page.appendCombo(
	    "Language",
	    StringArrayRange( kLanguageNames ),
	    importCallback,
	    IntExportCallback( IntExportCaller( g_language_option.m_latched ) )
	);
}
