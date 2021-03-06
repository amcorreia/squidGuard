/*
 * By accepting this notice, you agree to be bound by the following
 * agreements:
 *
 * This software product, squidGuard, is copyrighted
 * (C) 2012, Andreas Hofmeister, Collax GmbH,
 * (C) 1998-2009 by Christine Kronberg, Shalla Secure Services.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (version 2) as
 * published by the Free Software Foundation.  It is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License (GPL) for more details.
 *
 * You should have received a copy of the GNU General Public License
 * (GPL) along with this program.
 */

#include <string.h>

#include "sgSetting.h"
#include "sgLog.h"
#include "sgMemory.h"

struct Setting {
	struct Setting *next;
	char *		name;
	char *		value;
	char *		defaultValue;
	SettingCB	cb;
};

static struct Setting *firstSetting = NULL;
static struct Setting *lastSetting = NULL;

static struct Setting *newSetting(const char *key, const char *value)
{
	struct Setting *result = sgMalloc(sizeof(struct Setting));

	result->name = sgStrdup(key);
	result->value = value ? sgStrdup(value) : NULL;
	result->defaultValue = NULL;
	result->cb = NULL;

	result->next = NULL;

	if (lastSetting == NULL) {
		firstSetting = result;
		lastSetting = result;
	} else {
		lastSetting->next = result;
		lastSetting = result;
	}

	return result;
}

static void freeSetting(struct Setting *setting)
{
	if (setting == NULL)
		return;

	sgFree(setting->name);
	sgFree(setting->value);
	sgFree(setting->defaultValue);
	sgFree(setting);
}

void freeAllSettings()
{
	struct Setting *now = firstSetting;

	while (now) {
		struct Setting *next = now->next;
		freeSetting(now);
		now = next;
	}
}

static struct Setting *findSetting(const char *key)
{
	struct Setting *now;

	for (now = firstSetting; now; now = now->next)
		if (strcasecmp(now->name, key) == 0)
			return now;

	return now;
}

void registerSetting(const char *key, const char *defaultValue, SettingCB cb)
{
	struct Setting *setting;

	if ((setting = findSetting(key)) != NULL) {
		sgLogError("setting %s redefined", key);
		return;
	} else {
		sgLogDebug("register setting '%s', default '%s'", key, defaultValue);
		setting = newSetting(key, defaultValue);
		setting->defaultValue = defaultValue ? sgStrdup(defaultValue) : NULL;
		setting->cb = cb;

		if (setting->cb && defaultValue)
			setting->cb(defaultValue);
	}
}

void setSetting(const char *key, const char *value)
{
	struct Setting *setting;

	sgLogDebug("set setting '%s', value '%s'", key, value);

	if ((setting = findSetting(key)) == NULL) {
		sgLogWarn("setting '%s' is not registered (value='%s')", key, value);
		setting = newSetting(key, value);
	} else {
		sgFree(setting->value);
		setting->value = sgStrdup(value);
	}

	if (setting->cb)
		setting->cb(value);
}

const char *getSetting(const char *key)
{
	struct Setting *setting = findSetting(key);

	if (setting == NULL)
		return NULL;

	return setting->value;
}

int booleanSetting(const char *value)
{
	if (strcmp(value, "true") == 0)
		return 1;
	if (strcmp(value, "enable") == 0)
		return 1;
	if (strcmp(value, "1") == 0)
		return 1;
	if (strcmp(value, "false") == 0)
		return 0;
	if (strcmp(value, "disable") == 0)
		return 0;
	if (strcmp(value, "0") == 0)
		return 0;

	sgLogError("'%s' is not a boolean value, defaulting to 'false'");
	return 0;
}
