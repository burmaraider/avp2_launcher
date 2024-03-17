#include "../include/Screens/serverlist.h"

// Function pointers for the original render and update loops
static void *pOldRenderLoop;
static void *pOldUpdateLoop;

// BUTTONS
static Button xButton;
static Button okButton;
static Button cancelButton;
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

//mutex
CRITICAL_SECTION cs;
char *pszBuffer;


void ServerlistSetupScreen(void *pRenderLoop, void *pUpdateLoop)
{
    // save old render and update loops
    pOldRenderLoop = pRenderLoop;
    pOldUpdateLoop = pUpdateLoop;

    // set new render and update loops so our main loop is happy
    ScreenRenderLoop = ServerlistRenderScreen;
    ScreenUpdateLoop = ServerlistUpdateLoop;


    //setup critical section
    InitializeCriticalSection(&cs);
    // Start server communication in a separate thread
    StartServerCommunication(AVP2_MASTER_SERVER, AVP2_MASTER_PORT);
    
}
void ServerlistRenderScreen(void)
{

    // Start the 2D Canvas
    BeginDrawing();
    ClearBackground(BLACK);

    EndDrawing();
}
void ServerlistUpdateLoop(void)
{

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

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    
    free(pszBuffer);
    free(data); 
    LeaveCriticalSection(&cs);

    bSocketThreadDone = true;
}

void StartServerCommunication(const char* server, const char* port)
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