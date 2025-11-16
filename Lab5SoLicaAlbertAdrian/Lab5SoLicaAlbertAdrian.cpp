/* LINUX
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0) return n == 2;
    for (int d = 3; d * d <= n; d += 2)
        if (n % d == 0) return false;
    return true;
}

int worker_mode() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int start, end;
    if (!(cin >> start >> end)) return 1;
    for (int x = start; x <= end; ++x)
        if (is_prime(x)) cout << x << '\n';
    cout.flush();
    return 0;
}

int parent_mode(const char* progname) {
    const int NUM_WORKERS = 10;
    const int CHUNK = 1000;
    struct CP { int to_child[2]; int from_child[2]; pid_t pid; };
    vector<CP> w(NUM_WORKERS);

    for (int i = 0; i < NUM_WORKERS; ++i) {
        pipe(w[i].to_child);
        pipe(w[i].from_child);
        pid_t pid = fork();
        if (pid == 0) {
            close(w[i].to_child[1]);
            close(w[i].from_child[0]);
            dup2(w[i].to_child[0], STDIN_FILENO);
            dup2(w[i].from_child[1], STDOUT_FILENO);
            close(w[i].to_child[0]);
            close(w[i].from_child[1]);
            execl(progname, progname, "worker", (char*)NULL);
            exit(1);
        }
        else {
            w[i].pid = pid;
            close(w[i].to_child[0]);
            close(w[i].from_child[1]);
        }
    }

    for (int i = 0; i < NUM_WORKERS; ++i) {
        int start = i * CHUNK + 1;
        int end = (i + 1) * CHUNK;
        string msg = to_string(start) + " " + to_string(end) + "\n";
        write(w[i].to_child[1], msg.c_str(), msg.size());
        close(w[i].to_child[1]);
    }

    for (int i = 0; i < NUM_WORKERS; ++i) {
        cout << "Primes from worker " << i << ":\n";
        FILE* f = fdopen(w[i].from_child[0], "r");
        char buf[256];
        while (fgets(buf, sizeof(buf), f)) fputs(buf, stdout);
        fclose(f);
        cout << "----\n\n";
    }

    for (int i = 0; i < NUM_WORKERS; ++i) {
        int st;
        waitpid(w[i].pid, &st, 0);
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc >= 2 && string(argv[1]) == "worker")
        return worker_mode();
    return parent_mode(argv[0]);
}
*/ // LINUX 


// WINDOWS
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0) return n == 2;
    for (int d = 3; d * d <= n; d += 2)
        if (n % d == 0) return false;
    return true;
}

int worker_mode() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int start, end;
    if (!(cin >> start >> end)) return 1;
    for (int x = start; x <= end; ++x)
        if (is_prime(x)) cout << x << '\n';
    cout.flush();
    return 0;
}

string GetExePath() {
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (n == 0 || n == MAX_PATH) return "";
    return string(buf, n);
}

int parent_mode() {
    const int NUM_WORKERS = 10;
    const int CHUNK = 1000;
    string exe = GetExePath();

    for (int i = 0; i < NUM_WORKERS; ++i) {
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        HANDLE in_r, in_w, out_r, out_w;
        CreatePipe(&in_r, &in_w, &sa, 0);
        SetHandleInformation(in_w, HANDLE_FLAG_INHERIT, 0);
        CreatePipe(&out_r, &out_w, &sa, 0);
        SetHandleInformation(out_r, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdInput = in_r;
        si.hStdOutput = out_w;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        ZeroMemory(&pi, sizeof(pi));

        string cmd = "\"" + exe + "\" worker";
        BOOL ok = CreateProcessA(
            NULL,
            const_cast<char*>(cmd.c_str()),
            NULL, NULL,
            TRUE,
            0,
            NULL, NULL,
            &si,
            &pi
        );

        CloseHandle(in_r);
        CloseHandle(out_w);

        int start = i * CHUNK + 1;
        int end = (i + 1) * CHUNK;
        ostringstream oss;
        oss << start << " " << end << "\n";
        string msg = oss.str();
        DWORD written;
        WriteFile(in_w, msg.c_str(), msg.size(), &written, NULL);
        CloseHandle(in_w);

        cout << "Primes from worker " << i << ":\n";
        char buffer[4096];
        DWORD readBytes;
        while (ReadFile(out_r, buffer, 4096, &readBytes, NULL) && readBytes)
            cout.write(buffer, readBytes);
        CloseHandle(out_r);

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        cout << "----\n\n";
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc >= 2 && string(argv[1]) == "worker")
        return worker_mode();
    return parent_mode();
}
//WINDOWS