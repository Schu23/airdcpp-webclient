/*
* Copyright (C) 2011-2017 AirDC++ Project
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef DCPLUSPLUS_DCPP_WEBSERVER_SETTINGS_H
#define DCPLUSPLUS_DCPP_WEBSERVER_SETTINGS_H

#include <web-server/stdinc.h>

#include <api/ApiSettingItem.h>
#include <airdcpp/SettingsManager.h>

namespace webserver {
	class WebServerSettings {
	public:
		enum ServerSettings {
			PLAIN_PORT,
			PLAIN_BIND,

			TLS_PORT,
			TLS_BIND,

			TLS_CERT_PATH,
			TLS_CERT_KEY_PATH,

			SERVER_THREADS,
			DEFAULT_SESSION_IDLE_TIMEOUT,
			PING_INTERVAL,
			PING_TIMEOUT,

			EXTENSIONS_DEBUG_MODE,
		};

		// Initialized in WebServerManager
		static vector<ServerSettingItem> settings;

		static ServerSettingItem& getValue(ServerSettings aSetting) noexcept {
			return settings[aSetting];
		}

		static ServerSettingItem* getSettingItem(const string& aKey) noexcept {
			return ApiSettingItem::findSettingItem<ServerSettingItem>(settings, aKey);
		}
	};

#define WEBCFG(k) (webserver::WebServerSettings::getValue(webserver::WebServerSettings::k))
}

#endif