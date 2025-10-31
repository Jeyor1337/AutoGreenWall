#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <stdarg.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define CHDIR(path) _chdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#define CHDIR(path) chdir(path)
#endif

int is_quiet = 1;

void log_msg(const char *format, ...);
void getUserSettings(int *d_mode, int *p, int *min_c, int *max_c);
int prompt_for_int(const char *prompt, int min, int max);
void exec_cmd(const char *command);
int path_exists(const char *path);
void set_git_env(const char *key, const char *value);

int main(int argc, char *argv[])
{
    if (argc > 1 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--verbose"))) {
        is_quiet = 0;
    }

    int date_mode, commit_prob, min_c, max_c;
    getUserSettings(&date_mode, &commit_prob, &min_c, &max_c);

    srand(time(NULL));

    if (!path_exists("./contribution-data-c")) {
        log_msg("Directory not found. Creating './contribution-data-c'...\n");
        if (MKDIR("./contribution-data-c") != 0) {
            fprintf(stderr, "[FATAL] Failed to create directory.\n"); return 1;
        }
    }
    if (CHDIR("./contribution-data-c") != 0) {
        fprintf(stderr, "[FATAL] Failed to change directory.\n"); return 1;
    }

    if (!path_exists(".git")) {
        log_msg("No git repo found. Initializing...\n");
        exec_cmd("git init");
    }

    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    int year = tm_now->tm_year + 1900;

    struct tm start_dt = {0};
    start_dt.tm_year = year - 1900;
    start_dt.tm_mon = 0;
    start_dt.tm_mday = 1;

    struct tm end_dt;
    if (date_mode == 1) {
        end_dt = (struct tm){.tm_year = year - 1900, .tm_mon = 11, .tm_mday = 31};
    } else {
        end_dt = *tm_now;
    }
    time_t end_time_t = mktime(&end_dt);

    char start_str[11], end_str[11];
    strftime(start_str, 11, "%Y-%m-%d", &start_dt);
    strftime(end_str, 11, "%Y-%m-%d", &end_dt);
    
    log_msg("\nStarting job.\n");
    log_msg("  - Date Range: %s to %s\n", start_str, end_str);

    struct tm current_dt = start_dt;
    long commits_done = 0;
    char cmd_buf[512], date_buf[64];

    while (mktime(&current_dt) <= end_time_t)
    {
        if ((rand() % 100) < commit_prob) {
            int commits_today = min_c + (rand() % (max_c - min_c + 1));
            strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", &current_dt);
            log_msg("Date %s -> %d commits\n", date_buf, commits_today);

            for (int i = 0; i < commits_today; i++) {
                struct tm commit_time = current_dt;
                commit_time.tm_hour = 9 + (rand() % 12);
                commit_time.tm_min = rand() % 60;
                commit_time.tm_sec = rand() % 60;
                strftime(date_buf, sizeof(date_buf), "%Y-%m-%dT%H:%M:%S", &commit_time);

                set_git_env("GIT_AUTHOR_DATE", date_buf);
                set_git_env("GIT_COMMITTER_DATE", date_buf);

                FILE *fp = fopen("activity.log", "w");
                if (fp) { fprintf(fp, "update %s", date_buf); fclose(fp); }

                exec_cmd("git add activity.log");
                snprintf(cmd_buf, 512, "git commit -m \"sync: task %d for %d-%02d-%02d\"", i + 1, commit_time.tm_year + 1900, commit_time.tm_mon + 1, commit_time.tm_mday);
                exec_cmd(cmd_buf);
                commits_done++;
            }
        }
        current_dt.tm_mday++;
        mktime(&current_dt);
    }
    
    printf("\nOperation completed.\n");
    printf("Total commits created: %ld\n", commits_done);
    printf("\nNext steps to push to remote:\n");
    printf("  1. cd ./contribution-data-c\n");
    printf("  2. git remote add origin <your-github-repo-url>\n");
    printf("  3. git branch -M main\n");
    printf("  4. git push -u origin main\n");
    return 0;
}

void log_msg(const char *format, ...) {
    if (is_quiet) return;
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void getUserSettings(int *d_mode, int *p, int *min_c, int *max_c) {
    printf("--- Step 1: Select Date Range Mode ---\n");
    *d_mode = prompt_for_int("Mode [1] Full Year, [2] Until Today: ", 1, 2);
    printf("\n--- Step 2: Custom Commit Configuration ---\n");
    *p = prompt_for_int("Commit probability per day (1-100): ", 1, 100);
    *min_c = prompt_for_int("Min commits on commit-day (1-5): ", 1, 5);
    *max_c = prompt_for_int("Max commits on commit-day (min-5): ", *min_c, 5);
    printf("------------------------------------------\n");
}

int prompt_for_int(const char *prompt, int min, int max) {
    int val; char buf[128];
    while (1) {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin) && sscanf(buf, "%d", &val) == 1 && val >= min && val <= max) {
            return val;
        }
        printf("  > Invalid input. Expecting %d-%d.\n", min, max);
    }
}

void exec_cmd(const char *command)
{
    char final_cmd[512];
    const char *redir = "";
    if (is_quiet) {
#ifdef _WIN32
        redir = " > NUL 2>&1";
#else
        redir = " > /dev/null 2>&1";
#endif
    }
    snprintf(final_cmd, sizeof(final_cmd), "%s%s", command, redir);
    if (system(final_cmd) != 0) {
        fprintf(stderr, "  [WARN] Command failed: %s\n", command);
    }
}

int path_exists(const char *path) {
    struct stat info;
    return stat(path, &info) == 0 && (info.st_mode & S_IFDIR);
}

void set_git_env(const char *key, const char *value) {
#ifdef _WIN32
    _putenv_s(key, value);
#else
    setenv(key, value, 1);
#endif
}
