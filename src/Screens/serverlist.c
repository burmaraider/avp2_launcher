#include "../include/Screens/serverlist.h"

// Function pointers for the original render and update loops
static void *pOldRenderLoop;
static void *pOldUpdateLoop;
static int nOldDragHeight;

// BUTTONS
static Button xButton;
static Button okButton;
static Button cancelButton;
static Button joinButton;
static Button refreshButton;
static Button **buttons;

HANDLE hSocketThread;
static bool bSocketThreadDone = false;

int mainSocket;
int opt = true;
struct sockaddr_in address;
struct timeval timeout = {1, 0};
fd_set readfds;
char string_read[255];
char buffer[1025]; // data buffer of 1K

//server list
ServerEntry *serverList;
int serverCount = 0;
char *pszBuffer;

//mutex
CRITICAL_SECTION cs;


//textures
Texture2D tServerListTopBar;
Texture2D tServerTitle;


//status bar info it can hold 10 messages each 64 characters long
char szStatusBar[10][64];
bool bStatusText = false;
float fStatusTextTimer = 0.0f;
int nStatusTextIndex = 0;


void ServerlistSetupScreen(void *pRenderLoop, void *pUpdateLoop)
{
    // save old render and update loops
    pOldRenderLoop = pRenderLoop;
    pOldUpdateLoop = pUpdateLoop;

    // set new render and update loops so our main loop is happy
    ScreenRenderLoop = ServerlistRenderScreen;
    ScreenUpdateLoop = ServerlistUpdateLoop;

    // Set the window size
    nOldDragHeight = g_nDragHeight;
    g_nDragHeight = 32;
    SetWindowSize(1280, 800);

    memset(szStatusBar, 0, sizeof(szStatusBar));

    //Load textures
    LoadTextureFromResource(&tServerListTopBar, "SERVERLISTBAR", false);
    LoadTextureFromResource(&tServerTitle, "SERVERTITLE", false);

    LoadTextureFromResource(&xButton.texture[0], "SERVERLISTXN", false);
    LoadTextureFromResource(&xButton.texture[1], "SERVERLISTXH", false);
    LoadTextureFromResource(&xButton.texture[2], "SERVERLISTXD", false);
    xButton.position = (Vector2){1280 - 30, 4};
    xButton.onPress = OnButtonPressCancel;
    xButton.onUnload = UnloadButton;
    xButton.isEnabled = true;

    LoadTextureFromResource(&joinButton.texture[0], "SERVERLISTJOINN", false);
    LoadTextureFromResource(&joinButton.texture[1], "SERVERLISTJOINH", false);
    LoadTextureFromResource(&joinButton.texture[2], "SERVERLISTJOIND", false);
    joinButton.position = (Vector2){694, 45};
    joinButton.onPress = OnButtonPressJoin;
    joinButton.onUnload = UnloadButton;
    joinButton.isEnabled = true;
    
    LoadTextureFromResource(&refreshButton.texture[0], "SERVERLISTREFRESHN", false);
    LoadTextureFromResource(&refreshButton.texture[1], "SERVERLISTREFRESHH", false);
    LoadTextureFromResource(&refreshButton.texture[2], "SERVERLISTREFRESHD", false);
    refreshButton.position = (Vector2){637, 45};
    refreshButton.onPress = OnButtonPressRefresh;
    refreshButton.onUnload = UnloadButton;
    refreshButton.isEnabled = true;

    buttons = (Button **)malloc(sizeof(Button *) * SERVERLIST_BUTTON_COUNT);

    if (!buttons)
    {
        MessageBox(NULL, "Failed to allocate memory for buttons!", "Error", MB_OK | MB_ICONERROR);
        exit(1);
    }

    for (size_t i = 0; i < SERVERLIST_BUTTON_COUNT; i++)
    {
        buttons[i] = (Button *)malloc(sizeof(Button));

        if (!buttons[i])
        {
            MessageBox(NULL, "Failed to allocate memory for buttons!", "Error", MB_OK | MB_ICONERROR);
            exit(1);
        }
    }

    buttons[0] = &xButton;
    buttons[1] = &joinButton;
    buttons[2] = &refreshButton;


    //setup critical section
    InitializeCriticalSection(&cs);
    
}
void ServerlistRenderScreen(void)
{

    // Start the 2D Canvas
    BeginDrawing();
    ClearBackground((Color){28,28,28});

    // Draw the top bar
    DrawTextureTiled(tServerListTopBar, (Rect){0, 0, 32, 32}, (Rect){0, 0, 1280, 32}, (Vector2){0, 0}, 0, 1, WHITE);
    DrawTexture(tServerTitle, 0, 0, WHITE);

    //12 x 91
    DrawRectangle(12, 91, 884, 664, (Color){18,18,18,255});
    

    //Draw Statusbar
    DrawRectangle(0, 800-32, 1280, 32, (Color){36,36,36,255});

    //draw status text

    if(fStatusTextTimer > 0.01f)
    {
        int nAlpha = (int)(fStatusTextTimer * 255.0f / 1.0f);
        DrawTextRay(szStatusBar[0], 10, 800 - 32 + 8, 16, (Color){107, 154, 194, nAlpha});
    }

    for (size_t i = 0; i < SERVERLIST_BUTTON_COUNT; i++)
    {
        Color color = WHITE;
        if (!buttons[i]->isEnabled)
        {
            color = (Color){110, 110, 110, 255};
        }

        DrawTexture(buttons[i]->texture[buttons[i]->currentTexture], buttons[i]->position.x, buttons[i]->position.y, color);
    }

    EndDrawing();
}

static void CheckAllButtons(void)
{
    for (size_t i = 0; i < SERVERLIST_BUTTON_COUNT; i++)
    {
        Button* button = buttons[i];
        Vector2 mousePos = GetMousePosition();
        Rect buttonRect = {button->position.x, button->position.y, button->texture[0].width, button->texture[0].height};

        if (CheckCollisionPointRec(mousePos, buttonRect) && button->isEnabled)
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                button->currentTexture = DOWN;
            }
            else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                button->currentTexture = HOVER;
                button->onPress(button);
            }
            else
            {
                button->currentTexture = HOVER;
            }
        }
        else
        {
            button->currentTexture = UP;
            button->isPressed = FALSE;
        }
    }
}

void ServerlistUpdateLoop(void)
{

    CheckAllButtons();
    UpdateStatusText();

    if(bSocketThreadDone)
    {
        //kill the thread
        CloseHandle(hSocketThread);
        //reset the flag
        bSocketThreadDone = false;
    }

}
void ServerlistUnloadScreen(void)
{
    // Unload textures
    UnloadTexture(tServerListTopBar);
    UnloadTexture(tServerTitle);

    // Unload buttons
    for (size_t i = 0; i < SERVERLIST_BUTTON_COUNT; i++)
    {
        buttons[i]->onUnload(buttons[i]);
        free(buttons[i]);
    }

    g_nDragHeight = nOldDragHeight;
}


DWORD WINAPI ServerCommunicationThread(LPVOID lpParam)
{
    ServerData *data = (ServerData *)lpParam;

    char *pszServerIP = data->pszServerIP;
    char *pszServerPort = data->pszServerPort;

    // Initialize Winsock
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;
    char *sendbuf = "001";

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed\n");
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    if (getaddrinfo(pszServerIP, pszServerPort, &hints, &result) != 0)
    {
        printf("getaddrinfo failed\n");
        WSACleanup();
    }

    int connectionAttempts = 0;

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed\n");
            WSACleanup();
        }

        if (connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            connectionAttempts++;

            if (connectionAttempts >= 5)
            {
                printf("Failed to connect after 5 attempts\n");
                break;
            }

            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server\n");
        WSACleanup();
    }

    EnterCriticalSection(&cs);
    AddStatusText("Connected to server");
    LeaveCriticalSection(&cs);

    // Set the mode of the socket to be nonblocking
    // u_long iMode = 1;
    // ioctlsocket(ConnectSocket, FIONBIO, &iMode);

    // Send an initial buffer
    int iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed\n");
        closesocket(ConnectSocket);
        WSACleanup();
    }

    EnterCriticalSection(&cs);
    pszBuffer = (char *)malloc(1024);
    if (pszBuffer == NULL)
    {
        // Handle memory allocation failure...
        printf("Memory allocation failed!\n");
        return;
    }

    memset(pszBuffer, 0, 1024);
    LeaveCriticalSection(&cs);

    bool bEndOfMessage = false;
    bool bGreeting = false;

    EnterCriticalSection(&cs);
    AddStatusText("Receiving data from server");
    LeaveCriticalSection(&cs);
    // Receive until the peer closes the connection
    do
    {
        iResult = recv(ConnectSocket, buffer, 1024, 0);
        if (iResult > 0)
        {
            printf("Bytes received: %d\n", iResult);

            if(buffer[iResult - 1] == 0x02 && bGreeting == false)
            {
                bGreeting = true;
                continue;
            }

            if (buffer[iResult - 2] == 0x0a && buffer[iResult - 1] == 0x02)
            {
                printf("End of message\n");
                bEndOfMessage = true;
            }

            EnterCriticalSection(&cs);
            // Reallocate memory
            char *temp = realloc(pszBuffer, strlen(pszBuffer) + iResult + 1);
            if (temp == NULL)
            {
                // Handle memory allocation failure...
                printf("Memory allocation failed!\n");
                free(pszBuffer);
                return;
            }
            else
            {
                pszBuffer = temp;
            }

            // Append received data to buffer
            strncat(pszBuffer, buffer, iResult);
            LeaveCriticalSection(&cs);
            if(bEndOfMessage)
            {
                break;
            }
        }
        else if (iResult == 0)
        {
            printf("Connection closed\n");
        }
        else
        {
            if (WSAGetLastError() == WSAETIMEDOUT)
            {
                printf("Timeout occurred\n");
            }
            else
            {
                printf("recv failed\n");
            }
            break;
        }
    } while (iResult > 0);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed\n");
        closesocket(ConnectSocket);
        WSACleanup();
    }

    EnterCriticalSection(&cs);
    printf("Received: %s\n", pszBuffer);
    AddStatusText("Received data from server");
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    
    free(pszBuffer);
    free(data); 
    refreshButton.isEnabled = true;
    LeaveCriticalSection(&cs);

    bSocketThreadDone = true;
}

void StartServerCommunication(char* server, char* port)
{
    ServerData* data = (ServerData*)malloc(sizeof(ServerData));
    if (data == NULL)
    {
        return;
    }

    data->pszServerIP = server;
    data->pszServerPort = port;


    HANDLE hThread;

    // Create thread for server communication
    hThread = CreateThread(NULL, 0, ServerCommunicationThread, data, 0, NULL);
    if (hThread == NULL)
    {
        free(data);
        return;
    }

    // Close thread handle
    CloseHandle(hThread);
}
void AddStatusText(char* text)
{
    //add to the next empty slot
    for (size_t i = 0; i < 10; i++)
    {
        if(szStatusBar[i][0] == 0)
        {
            strncpy(szStatusBar[i], text, 64);
            nStatusTextIndex = i;  // Set nStatusTextIndex
            break;
        }
    }
}
void UpdateStatusText(void)
{
    //using nStatusTextIndex as a counter show the status text for 2 seconds then move to the next one if there is one
    if(fStatusTextTimer > 0.0f)
    {
        fStatusTextTimer -= GetFrameTime();
    }
    else
    {
        if(szStatusBar[0][0] != 0)
        {
            // Display the status text at index 0 for 2 seconds
            fStatusTextTimer = 1.0f;

            // Shift the elements in the array and count non-blank status texts
            int nonBlankCount = 0;
            for(int i = 0; i < 10 - 1; i++)
            {
                strcpy(szStatusBar[i], szStatusBar[i + 1]);
                if(szStatusBar[i][0] != 0)
                {
                    nonBlankCount++;
                }
            }

            // Clear the last element in the array
            szStatusBar[10 - 1][0] = 0;
        }
    }
}

static void OnButtonPressCancel(Button *button)
{
    ServerlistUnloadScreen();

    SetWindowSize(AVP2_MAIN_SCREEN_WIDTH, AVP2_MAIN_SCREEN_HEIGHT);

    g_nCurrentScreen = SCREEN_SPLASH;

    PlaySoundResource("BACK");

    ScreenUpdateLoop = pOldUpdateLoop;
    ScreenRenderLoop = pOldRenderLoop;
}

static void OnButtonPressJoin(Button *button)
{
    PlaySoundResource("OK");

}

static void OnButtonPressRefresh(Button *button)
{
    PlaySoundResource("OK");
    refreshButton.isEnabled = false;
    StartServerCommunication((char*)AVP2_MASTER_SERVER, (char*)AVP2_MASTER_PORT);
}