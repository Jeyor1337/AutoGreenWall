#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <git2.h>

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
int path_exists(const char *path);
int create_commit(git_repository *repo, const char *content, const char *message,
                  const char *author_name, const char *author_email, time_t commit_time);

int main(int argc, char *argv[])
{
    if (argc > 1 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--verbose"))) {
        is_quiet = 0;
    }

    git_libgit2_init();

    int date_mode, commit_prob, min_c, max_c;
    getUserSettings(&date_mode, &commit_prob, &min_c, &max_c);

    srand(time(NULL));

    if (!path_exists("./contribution-data-c")) {
        log_msg("Directory not found. Creating './contribution-data-c'...\n");
        if (MKDIR("./contribution-data-c") != 0) {
            fprintf(stderr, "[FATAL] Failed to create directory.\n");
            git_libgit2_shutdown();
            return 1;
        }
    }
    if (CHDIR("./contribution-data-c") != 0) {
        fprintf(stderr, "[FATAL] Failed to change directory.\n");
        git_libgit2_shutdown();
        return 1;
    }

    git_repository *repo = NULL;
    if (!path_exists(".git")) {
        log_msg("No git repo found. Initializing...\n");
        if (git_repository_init(&repo, ".", 0) != 0) {
            fprintf(stderr, "[FATAL] Failed to initialize repository: %s\n", git_error_last()->message);
            git_libgit2_shutdown();
            return 1;
        }
    } else {
        if (git_repository_open(&repo, ".") != 0) {
            fprintf(stderr, "[FATAL] Failed to open repository: %s\n", git_error_last()->message);
            git_libgit2_shutdown();
            return 1;
        }
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
    char msg_buf[256], date_buf[64], content_buf[128];

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
                time_t commit_timestamp = mktime(&commit_time);

                strftime(date_buf, sizeof(date_buf), "%Y-%m-%dT%H:%M:%S", &commit_time);
                snprintf(content_buf, sizeof(content_buf), "update %s", date_buf);
                snprintf(msg_buf, sizeof(msg_buf), "sync: task %d for %d-%02d-%02d",
                         i + 1, commit_time.tm_year + 1900, commit_time.tm_mon + 1, commit_time.tm_mday);

                if (create_commit(repo, content_buf, msg_buf, "AutoGreenWall", "auto@greenwall.local", commit_timestamp) != 0) {
                    fprintf(stderr, "[WARN] Failed to create commit for %s\n", date_buf);
                    continue;
                }
                commits_done++;
            }
        }
        current_dt.tm_mday++;
        mktime(&current_dt);
    }

    git_repository_free(repo);
    
    printf("\nOperation completed.\n");
    printf("Total commits created: %ld\n", commits_done);
    printf("\nNext steps to push to remote:\n");
    printf("  1. cd ./contribution-data-c\n");
    printf("  2. git remote add origin <your-github-repo-url>\n");
    printf("  3. git branch -M main\n");
    printf("  4. git push -u origin main\n");

    git_libgit2_shutdown();
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

int path_exists(const char *path) {
    struct stat info;
    return stat(path, &info) == 0 && (info.st_mode & S_IFDIR);
}

int create_commit(git_repository *repo, const char *content, const char *message,
                  const char *author_name, const char *author_email, time_t commit_time)
{
    int error = 0;
    git_oid tree_id, commit_id, blob_id;
    git_tree *tree = NULL;
    git_signature *sig = NULL;
    git_index *index = NULL;
    git_reference *head_ref = NULL;
    git_commit *parent_commit = NULL;
    const git_commit *parents[1];
    size_t parent_count = 0;

    // Create signature with custom timestamp
    if ((error = git_signature_new(&sig, author_name, author_email, commit_time, 0)) < 0) {
        goto cleanup;
    }

    // Create blob from content
    if ((error = git_blob_create_frombuffer(&blob_id, repo, content, strlen(content))) < 0) {
        goto cleanup;
    }

    // Get index and add blob to it
    if ((error = git_repository_index(&index, repo)) < 0) {
        goto cleanup;
    }

    git_index_entry entry = {0};
    entry.mode = GIT_FILEMODE_BLOB;
    entry.id = blob_id;
    entry.path = "activity.log";

    if ((error = git_index_add(index, &entry)) < 0) {
        goto cleanup;
    }

    // Write tree
    if ((error = git_index_write_tree(&tree_id, index)) < 0) {
        goto cleanup;
    }

    if ((error = git_tree_lookup(&tree, repo, &tree_id)) < 0) {
        goto cleanup;
    }

    // Get parent commit if exists
    error = git_reference_name_to_id(&commit_id, repo, "HEAD");
    if (error == 0) {
        if ((error = git_commit_lookup(&parent_commit, repo, &commit_id)) == 0) {
            parents[0] = parent_commit;
            parent_count = 1;
        }
    }
    error = 0;  // Reset error as HEAD might not exist on first commit

    // Create commit
    if ((error = git_commit_create(&commit_id, repo, "HEAD", sig, sig, NULL,
                                    message, tree, parent_count, parents)) < 0) {
        goto cleanup;
    }

    // Write index to disk
    git_index_write(index);

cleanup:
    if (parent_commit) git_commit_free(parent_commit);
    if (tree) git_tree_free(tree);
    if (index) git_index_free(index);
    if (head_ref) git_reference_free(head_ref);
    if (sig) git_signature_free(sig);

    if (error < 0) {
        const git_error *e = git_error_last();
        log_msg("  [ERROR] %s\n", e ? e->message : "unknown error");
    }

    return error;
}
