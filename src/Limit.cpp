#include "Limit.h"
#include "ConfigFile.h"
#include "helperFunctions.h"
#pragma warning(push, 0)
#include "../phnt/phnt.h"
#include "../phnt/phnt_windows.h"
#pragma warning(pop)

void Limit::ToggleWholeGameLimit(std::atomic<limit>* lim_game)
{
    limit temp_limit = lim_game->load();
    temp_limit.state = !temp_limit.state;
    lim_game->store(temp_limit);

    if (temp_limit.state) {
        ShellExecute(NULL, NULL, L"powershell.exe",
            L"-ExecutionPolicy bypass -noe -c New-NetQosPolicy -Name 'Destiny2-Limit' -AppPathNameMatchCondition 'destiny2.exe' -ThrottleRateActionBitsPerSecond 0.801KB", NULL,
            SW_HIDE);
    } else {
        ShellExecute(NULL, NULL, L"powershell.exe", L"-ExecutionPolicy bypass -c Remove-NetQosPolicy -Name 'Destiny2-Limit' -Confirm:$false", NULL, SW_HIDE);
    }
}


void Limit::ToggleSuspend(std::atomic<limit>* suspend)
{
    if (!Helper::D2Active()) { // prevents from pausing random stuff if running with debug
        MessageBox(NULL, L"failed to pause...\nd2 is not the active window", NULL, MB_OK | MB_ICONWARNING);
        return;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    suspend->load().state ? SuspendProcess(pid, false) : SuspendProcess(pid, true);

    limit temp_limit = suspend->load();
    temp_limit.state = !temp_limit.state;
    suspend->store(temp_limit);
}


void Limit::SuspendProcess(DWORD pid, bool suspend)
{
    if (pid == 0) {
        return;
    }

    HANDLE hProc = OpenProcess(PROCESS_SUSPEND_RESUME, 0, pid);

    if (hProc == NULL) {
        return;
    }
    suspend ? printf("suspending process\n") : printf("resuming process\n");
    suspend ? NtSuspendProcess(hProc) : NtResumeProcess(hProc);
    CloseHandle(hProc);
}
