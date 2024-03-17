#include "include\registry.h"

/// @brief This function gets a registry value as an integer
/// @param hk Type of registry key to open
/// @param key The key to open
/// @param value The value to get
/// @param defaultValue The default value to return if the value is not found
/// @return The value of the registry key as an integer
int GetRegistryInt(HKEY hk, const char *key, const char *value, int defaultValue)
{
    HKEY hKey;
    // Open the registry key for reading, no write, and no create
    LONG lRes = RegOpenKeyExA(HKEY_CURRENT_USER, AVP2_USER_REGISTRY_x64, 0, KEY_WOW64_32KEY | KEY_QUERY_VALUE, &hKey);
    if (lRes != 0)
    {
        return defaultValue;
    }

    DWORD dwType;
    DWORD dwSize = sizeof(int);
    int nResult = 0;
    lRes = RegQueryValueEx(hKey, value, 0, &dwType, (LPBYTE)&nResult, &dwSize);
    RegCloseKey(hKey);

    if (lRes != 0)
    {
        return defaultValue;
    }
    else
    {
        return nResult;
    }
}

/// @brief This function gets a registry value as a string
/// @param hK The registry key to open
/// @param StringName The name of the string to get
/// @param valueBuffer The buffer to store the value in
/// @param value_length The length of the value
/// @return The value of the registry key as a string
char *GetRegistryString(HKEY hK, const char *StringName, char *valueBuffer, DWORD *value_length)
{
    DWORD dwType = RRF_RT_REG_SZ;
    HKEY hKey = 0;

    LONG results;

    results = (hK == HKEY_LOCAL_MACHINE) ? RegOpenKeyEx(hK, AVP2_REGISTRY_x64, 0, KEY_READ | KEY_WOW64_32KEY, &hKey) : RegOpenKeyEx(hK, AVP2_USER_REGISTRY_x64, 0, KEY_READ | KEY_WOW64_32KEY, &hKey);

    if (results != ERROR_SUCCESS)
    {
        return NULL;
    }

    RegQueryValueEx(hKey, StringName, NULL, &dwType, (LPBYTE)valueBuffer, value_length);
    RegCloseKey(hKey);

    return valueBuffer;
}

/// @brief This function gets the registry entries for the launcher
/// @return  True if the registry entries were found, false otherwise
bool GetRegistryEntries(void)
{
    // Set this to true by default, if we fail to get the registry entries, we'll set it to false
    g_bAVP2Installed = true;

    // Allocate a buffer for the registry string
    char buffer[2048];
    DWORD bufferSize;

    // Get the command line from the registry
    char *text = GetRegistryString(HKEY_LOCAL_MACHINE, AVP2_REGISTRY_COMMANDLINE, buffer, &bufferSize);
    if (bufferSize == 0 || text == NULL || text[0] == '\0' || bufferSize > 2048)
    {
        g_bAVP2Installed = false;
        return false;
    }
    g_pszCommandLine = strdup(text);

    // Get the installation directory from the registry
    text = GetRegistryString(HKEY_LOCAL_MACHINE, AVP2_REGISTRY_INSTALLDIR, buffer, &bufferSize);

    // check if  text is /0x0
    if (bufferSize == 0 || text == NULL || text[0] == '\0' || bufferSize > 2048)
    {
        g_bAVP2Installed = false;
        return false;
    }
    g_pszInstallDir = strdup(text);

    g_Settings.szCommands = calloc(256, sizeof(char));
    g_Settings.szCommands = GetRegistryString(HKEY_CURRENT_USER, AVP2_REGISTRY_COMMANDS, g_Settings.szCommands, &bufferSize);
    g_Settings.szLanguage = calloc(256, sizeof(char));
    g_Settings.szLanguage = GetRegistryString(HKEY_CURRENT_USER, "Language", g_Settings.szLanguage, &bufferSize);
    g_Settings.nDisableHardwareCursor = GetRegistryInt(HKEY_CURRENT_USER, AVP2_REGISTRY_x64, "Disable Hardware Cursor", 0);
    g_Settings.nDisableJoysticks = GetRegistryInt(HKEY_CURRENT_USER, AVP2_REGISTRY_x64, "Disable Joysticks", 0);
    g_Settings.nDisableMovies = GetRegistryInt(HKEY_CURRENT_USER, AVP2_REGISTRY_x64, "Disable Movies", 0);
    g_Settings.nDisableMusic = GetRegistryInt(HKEY_CURRENT_USER, AVP2_REGISTRY_x64, "Disable Music", 0);
    g_Settings.nDisableSound = GetRegistryInt(HKEY_CURRENT_USER, AVP2_REGISTRY_x64, "Disable Sound", 0);
    g_Settings.nDisableTripleBuffering = GetRegistryInt(HKEY_CURRENT_USER, AVP2_REGISTRY_x64, "Disable Triple Buffering", 0);
    g_Settings.nDisplayWarning = GetRegistryInt(HKEY_CURRENT_USER, AVP2_REGISTRY_x64, "DisplayWarning", 0);
    g_Settings.nNumLauncherRuns = GetRegistryInt(HKEY_CURRENT_USER, AVP2_REGISTRY_x64, "Num Launcher Runs", 0);
    g_Settings.nOptionsWarning = GetRegistryInt(HKEY_CURRENT_USER, AVP2_REGISTRY_x64, "OptionsWarning", 0);
    g_Settings.nSaveCommands = GetRegistryInt(HKEY_CURRENT_USER, AVP2_REGISTRY_x64, "Save Commands", 0);

    return true;
}

BOOL IsRunAsAdministrator(void)
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = 0;
    PSID pAdministratorsGroup = NULL;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
            &NtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pAdministratorsGroup))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    // Determine whether the SID of administrators group is enabled in
    // the primary access token of the process.
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    // Throw the error if something failed in the function.
    if (0 != dwError)
    {
        exit(dwError);
    }

    return fIsRunAsAdmin;
}

/// @brief This function installs the registry entries for the launcher and will launch the launcher as administrator if it isn't already running as administrator
/// @return  True if the registry entries were installed, false otherwise
bool InstallAVP2Registry(void)
{
    BOOL bAlreadyRunningAsAdministrator = FALSE;
    bAlreadyRunningAsAdministrator = IsRunAsAdministrator();

    if (!bAlreadyRunningAsAdministrator)
    {
        char szPath[MAX_PATH];
        if (GetModuleFileNameA(NULL, szPath, (DWORD)MAX_PATH))
        {
            // Launch itself as admin
            SHELLEXECUTEINFO sei = {sizeof(sei)};
            sei.lpVerb = "runas";
            sei.lpFile = szPath;
            sei.lpParameters = "-install";
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;

            if (!ShellExecuteEx(&sei))
            {
                DWORD dwError = GetLastError();
                if (dwError == 1223)
                {
                    // The user refused to allow privileges elevation.
                    printf("The user refused to allow privileges elevation.\n");
                }
            }
            else
            {
                printf("The launcher is now running as administrator.\n");
                _exit(1); // Quit itself
            }
        }
    }

    if (IsRunAsAdministrator())
    {

        HKEY hKey = 0;
        DWORD dwDisposition = 0;

        LONG results = RegCreateKeyEx(HKEY_LOCAL_MACHINE, AVP2_REGISTRY_x64, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WOW64_32KEY, NULL, &hKey, &dwDisposition);

        if (results != ERROR_SUCCESS)
        {
            return false;
        }

        results = RegSetValueEx(hKey, AVP2_REGISTRY_COMMANDLINE, 0, REG_SZ, (LPBYTE)AVP2_DEFAULT_COMMANDLINE, strlen(AVP2_DEFAULT_COMMANDLINE) + 1);

        if (results != ERROR_SUCCESS)
        {
            return false;
        }

        results = RegSetValueEx(hKey, AVP2_REGISTRY_INSTALLDIR, 0, REG_SZ, (LPBYTE)AVP2_DEFAULT_INSTALLDIR, strlen(AVP2_DEFAULT_INSTALLDIR) + 1);

        if (results != ERROR_SUCCESS)
        {
            return false;
        }

        RegCloseKey(hKey);
        return true;
    }

    return false;
}

void SetRegistryValue(HKEY hKey, const char *pszValueName, DWORD dwType, const void *pData, DWORD dwDataSize)
{
    HKEY hSubKey = 0;
    DWORD dwDisposition = 0;

    LONG results = RegOpenKeyEx(hKey, AVP2_USER_REGISTRY_x64, 0, KEY_WRITE | KEY_WOW64_32KEY, &hSubKey);

    // if the key doesn't exist, create it
    if (results != ERROR_SUCCESS)
    {
        // CREATE THE KEY
        results = RegCreateKeyEx(hKey, AVP2_USER_REGISTRY_x64, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WOW64_32KEY, NULL, &hSubKey, &dwDisposition);

        // if we failed to create the key, return
        if (results != ERROR_SUCCESS)
        {
            MessageBox(NULL, "Failed to create registry key!", "Error", MB_OK | MB_ICONERROR);
            return;
        }
    }

    results = RegSetValueEx(hSubKey, pszValueName, 0, dwType, (LPBYTE)pData, dwDataSize);

    if (results != ERROR_SUCCESS)
    {
        MessageBox(NULL, "Failed to set registry value!", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    RegCloseKey(hSubKey);
}