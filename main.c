#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

/*
pub fn using() {
    println!("Ospm Usage:");
    println!("init - create ospm config");
    println!("repo add <url> - add repo");
    println!("repo list - show repo list");
    println!("repo del <title> - del repo from list");
    println!("add <repo> <package> - install package");
    println!("version - show ospm version");
    println!("help - show this text");
    stdout().flush().unwrap();
}

pub fn get_config_path() -> String {
    let home_dir = home::home_dir().unwrap().as_os_str().to_str().unwrap().to_owned();
    home_dir + "/ospm-config"
}

pub fn check_config_exists() -> bool {
    fs::exists(get_config_path()).unwrap()
}
*/

#define eprint(msg) fprintf(stderr, msg)
char* get_config_path() {
    char* home = getenv("HOME");
    int home_size = strlen(home);
    char* o = malloc(home_size+10);
    sprintf(o, "%s/ospmc", home);
    return o;}char** strsplit(char* a) {
    int l = strlen(a);
    char* f = malloc(l);
    char* s = malloc(l);
    int p1 = 0, p2 = 0;
    for (int i = 0; i < l; i++) {
        if (a[i] == ':' && a[i+1] == ' ')
            break;
        f[p1] = a[i];
        p1++;
    }
    f[p1] = 0;
    for (int i = p1+2; i < l; i++) {
        s[p2] = a[i];
        p2++;
    }
    s[p2] = 0;
    char** arr = malloc(sizeof(char**)*2);
    arr[0] = f;
    arr[1] = s;
    return arr;}void free_strlist(char** a, int l) {
    for (int i = 0; i < l; i++)
        free(a[i]);
    free(a);}void using() {
    printf("ospm usage:\n");
    printf("init - install config\n");
    printf("check <repo/package> (optional: repo name) - get list of <repo/packages in repo>");
    printf("repo add <url> - add repo\n");
    printf("add <repo> <package name> - install package\n");}char* curl_get(char* url) {
    char* c = malloc(strlen(url) + 30);
    sprintf(c, "curl %s -o /tmp/curltmp", url);
    system(c);
    free(c);
    FILE* f;
    f = fopen("/tmp/curltmp", "rb");
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);
    char* result = malloc(file_size+1);
    fread(result, 1, file_size, f);
    fclose(f);
    return result;}


   
int c_init() {
    char* c = get_config_path();
    int result = mkdir(c, 0644);
    free(c);
    return result;
}


int c_repo(int argc, char** argv) {
    if (argc == 2) {
        printf("repo add <url> - add repo\n");
        return 1;
    }
    if (!strcmp(argv[2], "add")) {
        if (argc != 4) {
            printf("usage %s repo add <url>", argv[0]);
            return 1;
        }
        char* confp = get_config_path();
        char* repo = curl_get(argv[3]);
        if (repo) {
            int repol = strlen(repo);
            char* title;

            char* line = malloc(100);
            int linep = 0;
            for (int i = 0; i < repol; i++) {
                linep = 0;
                while (repo[i] != '\n' && repo[i] != 0) {
                    line[linep] = repo[i];
                    i++;
                    linep++;
                }
                line[linep] = 0;
                if (strlen(line) == 0) 
                    continue;
                char** sp = strsplit(line);
                if (!strcmp(sp[0], "title")) {
                    title = sp[1];
                    break;
                }
                free_strlist(sp, 2);
            }
            char anwser;
            char* cpath = malloc(strlen(confp)+strlen(title)+5);
            sprintf(cpath, "%s/%s", confp, title);
            printf("%s\n", cpath);
            free(confp);
            printf("add repo \"%s\"?(Y/N)\n", title);fflush(stdout);
            for(;;) {
                printf(":");fflush(stdout);
                read(0, &anwser, 1);
                if (anwser == 'Y' || anwser == 'y')
                    break;
                if (anwser == 'N' || anwser == 'n') {
                    free(title);
                    free(cpath);
                    free(repo);
                    return 1;
                }
                printf("\n");fflush(stdout);
            }
            FILE *file = fopen(cpath, "w");
            fprintf(file, "%s", repo);
            fclose(file);
            free(title);
            free(cpath);
            free(repo);
            return 0;
        }
        else {
            free(confp);
            return 1;
        }
    }
    return 0;
}


int c_add(int argc, char** argv) {
    if (argc != 4) {
        printf("add <repo> <package name> - install package\n");
        return 2;
    }
    char* package = argv[3];
    char* title = argv[2];
    char* confp = get_config_path();
    char* cpath = malloc(strlen(confp)+strlen(title)+5);
    sprintf(cpath, "%s/%s", confp, title);
    printf("%s\n", cpath);
    free(confp);
    FILE *fptr = fopen(cpath, "r");
    fseek(fptr, 0, SEEK_END);
    long file_size = ftell(fptr);
    rewind(fptr);
    char* repo = malloc(file_size+1);
    fread(repo, 1, file_size, fptr);
    int repol = strlen(repo);
    char* line = malloc(100);
    int linep = 0;
    for (int i = 0; i < repol; i++) {
        linep = 0;
        while (repo[i] != '\n' && repo[i] != 0) {
            line[linep] = repo[i];
            i++;
            linep++;
        }
        line[linep] = 0;
        if (strlen(line) == 0) 
            continue;
        char** sp = strsplit(line);
        if (!strcmp(sp[0], package)) {
            curl_get(sp[1]);
            system("chmod +x /tmp/curltmp");
            system("/tmp/curltmp");
            free_strlist(sp, 2);
            return 0;
        }
        free_strlist(sp, 2);
    }
    printf("cant find %s:%s\n", cpath, package);fflush(stdout);
    return 3;
}

int c_check(int argc, char** argv) {
    if (argc < 3) {
        printf("usage: %s check <repo/package> (optional: repo name)\n", argv[0]);return 1;
    }
    if (!strcmp(argv[2], "repo")) {
        char* p = get_config_path();
        char* cmd = malloc(6 + strlen(p));
        sprintf(cmd, "ls %s", p);
        printf("repo list:\n\x1b[32m");fflush(stdout);
        system(cmd);
        printf("\x1b[0m");fflush(stdout);
        return 0;
    }
    if (!strcmp(argv[2], "package")) {
        if (argc != 4) {
            printf("usage: %s check <repo/package> (optional: repo name)\n", argv[0]);return 1;
        }
        char* p = get_config_path();
        char* cpath = malloc(2 + strlen(p) + strlen(argv[3]));
        sprintf(cpath, "%s/%s", p, argv[3]);
        printf("%s\n", cpath);
        free(p);
        FILE *fptr = fopen(cpath, "r");
        fseek(fptr, 0, SEEK_END);
        long file_size = ftell(fptr);
        rewind(fptr);
        char* repo = malloc(file_size+1);
        fread(repo, 1, file_size, fptr);
        int repol = strlen(repo);
        char* line = malloc(100);
        int linep = 0;
        printf("packages in %s list:\n\x1b[32m", argv[3]);fflush(stdout);
        for (int i = 0; i < repol; i++) {
            linep = 0;
            while (repo[i] != '\n' && repo[i] != 0) {
                line[linep] = repo[i];
                i++;
                linep++;
            }
            line[linep] = 0;
            if (strlen(line) == 0) 
                continue;
            char** sp = strsplit(line);
            if (strcmp(sp[0], "title")) {
                printf("%s\n", sp[0]);
            }
            free_strlist(sp, 2);
        }
        printf("\x1b[0m");fflush(stdout);
        return 0;
    }
    return 3;
}


#ifndef _WIN32
int main(int argc, char** argv) {
    if (argc == 1 || !strcmp(argv[1], "help")) {
        using();return 0;
    }
    if (!strcmp(argv[1], "init")) {
        int result = c_init();
        if (result) {printf("ERROR\n");} else {printf("SUCCESS\n");}
        return result;
    }
    if (!strcmp(argv[1], "repo")) {
        int result = c_repo(argc, argv);
        if (result) {printf("ERROR\n");} else {printf("SUCCESS\n");}
        return result;
    }
    if (!strcmp(argv[1], "add")) {
        int result = c_add(argc, argv);
        if (result) {printf("ERROR\n");} else {printf("SUCCESS\n");}
        return result;
    }
    if (!strcmp(argv[1], "check")) {
        int result = c_check(argc, argv);
        if (result) {printf("ERROR\n");} else {printf("SUCCESS\n");}
        return result;
    }
    return 5;
}
#else
int main() {
    printf("windows isnt support\n");
    return 5;
}
#endif
