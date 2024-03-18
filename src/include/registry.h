#ifndef _REGISTRY_H_
#define _REGISTRY_H_


#include <Windows.h>
// C headers
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "string.h"

// Program headers
#include "constants.h"

/// @brief This function gets a registry value as an integer
/// @param hk Type of registry key to open
/// @param key The key to open
/// @param value The value to get
/// @param defaultValue The default value to return if the value is not found
/// @return The value of the registry key as an integer
int GetRegistryInt(HKEY hk, const char *key, const char *value, int defaultValue);

/// @brief This function gets a registry value as a string
/// @param hK The registry key to open
/// @param StringName The name of the string to get
/// @param valueBuffer The buffer to store the value in
/// @param value_length The length of the value
/// @return The value of the registry key as a string
char *GetRegistryString(HKEY hK, const char *StringName, char *valueBuffer, DWORD *value_length);
/// @brief This function gets the registry entries for the launcher
/// @return  True if the registry entries were found, false otherwise
bool GetRegistryEntries();

BOOL IsRunAsAdministrator(void);
/// @brief This function installs the registry entries for the launcher and will launch the launcher as administrator if it isn't already running as administrator
/// @return  True if the registry entries were installed, false otherwise
bool InstallAVP2Registry(void);

void SetRegistryValue(HKEY hKey, const char *pszValueName, DWORD dwType, const void *pData, DWORD dwDataSize);

#endif // _REGISTRY_H_