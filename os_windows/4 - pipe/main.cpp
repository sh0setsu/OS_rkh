#include <windows.h>
#include <iostream>
#include <string>

int main() {
    HANDLE pipe1Read, pipe1Write;
    HANDLE pipe2Read, pipe2Write;
    HANDLE pipe3Read, pipe3Write;

    SECURITY_ATTRIBUTES secAttr;
    secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    secAttr.lpSecurityDescriptor = NULL;
    secAttr.bInheritHandle = TRUE;

    CreatePipe(&pipe1Read, &pipe1Write, &secAttr, 0);
    CreatePipe(&pipe2Read, &pipe2Write, &secAttr, 0);
    CreatePipe(&pipe3Read, &pipe3Write, &secAttr, 0);

    PROCESS_INFORMATION procM, procA, procP, procS;

    STARTUPINFO startupM;
    ZeroMemory(&startupM, sizeof(startupM));
    startupM.cb = sizeof(STARTUPINFO);
    startupM.dwFlags = STARTF_USESTDHANDLES;
    startupM.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupM.hStdOutput = pipe1Write;

    STARTUPINFO startupA;
    ZeroMemory(&startupA, sizeof(startupA));
    startupA.cb = sizeof(STARTUPINFO);
    startupA.dwFlags = STARTF_USESTDHANDLES;
    startupA.hStdInput = pipe1Read;
    startupA.hStdOutput = pipe2Write;

    STARTUPINFO startupP;
    ZeroMemory(&startupP, sizeof(startupP));
    startupP.cb = sizeof(STARTUPINFO);
    startupP.dwFlags = STARTF_USESTDHANDLES;
    startupP.hStdInput = pipe2Read;
    startupP.hStdOutput = pipe3Write;

    STARTUPINFO startupS;
    ZeroMemory(&startupS, sizeof(startupS));
    startupS.cb = sizeof(STARTUPINFO);
    startupS.dwFlags = STARTF_USESTDHANDLES;
    startupS.hStdInput = pipe3Read;
    startupS.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    char cmdM[] = "M.exe";
    char cmdA[] = "A.exe";
    char cmdP[] = "P.exe";
    char cmdS[] = "S.exe";

    CreateProcess(NULL, cmdM, NULL, NULL, TRUE, 0, NULL, NULL, &startupM, &procM);
    CreateProcess(NULL, cmdA, NULL, NULL, TRUE, 0, NULL, NULL, &startupA, &procA);
    CreateProcess(NULL, cmdP, NULL, NULL, TRUE, 0, NULL, NULL, &startupP, &procP);
    CreateProcess(NULL, cmdS, NULL, NULL, TRUE, 0, NULL, NULL, &startupS, &procS);

    WaitForSingleObject(procM.hProcess, INFINITE);
    WaitForSingleObject(procA.hProcess, INFINITE);
    WaitForSingleObject(procP.hProcess, INFINITE);
    WaitForSingleObject(procS.hProcess, INFINITE);

    CloseHandle(procM.hProcess);
    CloseHandle(procA.hProcess);
    CloseHandle(procP.hProcess);
    CloseHandle(procS.hProcess);

    CloseHandle(pipe1Read);
    CloseHandle(pipe1Write);
    CloseHandle(pipe2Read);
    CloseHandle(pipe2Write);
    CloseHandle(pipe3Read);
    CloseHandle(pipe3Write);

    return 0;
}