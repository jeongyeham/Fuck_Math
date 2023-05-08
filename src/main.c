#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "hpdf.h"
#include "windows.h"
#include "ini.h"

#define getrandom(min, max) (SHORT)((rand() % (int)(((max) + 1) - \
                                                    (min))) +     \
                                    (min))

typedef enum
{
        and = 0,
        minus,
        times,
        divided,
        and_minus_mixed,
        times_divided_mixed,
        all_mixed,
} enum_operate;

struct init_para_t
{
    // topics settings
    int min_number;
    int max_number;
    enum_operate operation;
    int topics_depth;
    int topics_num;
    BOOL is_valid;

    // HPDF settings
    int paper_width;
    int paper_height;
    int font_size;
} _init_params;
typedef struct init_para_t *init_t;

struct ini_file_para_t
{
    int version;
    const char *name;
    const char *email;
} _ini_file_params;
typedef struct ini_file_para_t *ini_file_t;

jmp_buf env;
HANDLE mutex_handle;

const char *fname = "output.pdf";
const char *ttf_file_name = ".\\input.ttf";

DWORD WINAPI pdf_task(LPVOID lpParam);
DWORD WINAPI cli_task(LPVOID lpParam);

static int handler(void *user, const char *section, const char *name,
                   const char *value)
{
    ini_file_t pconfig = (ini_file_t)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("protocol", "version"))
    {
        pconfig->version = atoi(value);
    }
    else if (MATCH("user", "name"))
    {
        pconfig->name = strdup(value);
    }
    else if (MATCH("user", "email"))
    {
        pconfig->email = strdup(value);
    }
    else
    {
        return 0; /* unknown section/name, error */
    }
    return 1;
}

static void error_handler(HPDF_STATUS error_no,
                          HPDF_STATUS detail_no,
                          void *user_data)
{
    printf("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
           (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

int main(void)
{
    DWORD pdf_task_ID = -1;
    DWORD cli_task_ID = -1;
    init_t init_env = malloc(sizeof(_init_params));
    char ini_input_buff;

    printf("Has a ini file ? (Y or N) : ");
    scanf("%s", &ini_input_buff);

    if (strcmp(&ini_input_buff, "Y") == 0 || strcmp(&ini_input_buff, "y") == 0)
    {
        ini_file_t config = malloc(sizeof(_ini_file_params));
        if (ini_parse("test.ini", handler, config) < 0)
        {
            printf("Can't load 'test.ini'\n");
            goto HAS_NO_INI;
        }
    }
    else if (strcmp(&ini_input_buff, "N") == 0 || strcmp(&ini_input_buff, "n") == 0)
    {
        goto HAS_NO_INI;
    }
    else
    {
        goto HAS_SHIT;
    }

    goto MAIN_TASK;

HAS_NO_INI:
    printf("please input min_number\n");
    scanf("%d", &init_env->min_number);

    printf("please input max_number\n");
    scanf("%d", &init_env->max_number);

    printf("please input operation\n");
    printf("  and                    -----> 0\n \
              minus                  -----> 1\n \
              times                  -----> 2\n \
              divided                -----> 3\n \
              and_minus_mixed        -----> 4\n \
              times_divided_mixed    -----> 5\n \
              all_mixed              -----> 6\n");
    scanf("%d", &init_env->operation);

    printf("please input topics_depth\n");
    scanf("%d", &init_env->topics_depth);

    printf("please input topics_num\n");
    scanf("%d", &init_env->topics_num);

MAIN_TASK:
    mutex_handle = CreateMutex((LPSECURITY_ATTRIBUTES)NULL,
                               FALSE,
                               NULL);

    HANDLE pdf_task_handle = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                          (SIZE_T)0,
                                          (LPTHREAD_START_ROUTINE)&pdf_task,
                                          (LPVOID)init_env,
                                          (DWORD)0,
                                          (LPDWORD)&pdf_task_ID);

    HANDLE cli_task_handle = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                          (SIZE_T)0,
                                          (LPTHREAD_START_ROUTINE)&cli_task,
                                          (LPVOID)init_env,
                                          (DWORD)0,
                                          (LPDWORD)&cli_task_ID);
    system("pause");
    
HAS_SHIT:
    printf("Goodbye\n");
    for (int i = 5; i >= 0; i--)
    {
        Sleep(1000);
        printf("program will shutdown in %d s\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", i);
        fflush(stdout);
    }
    return 0;
};

DWORD WINAPI pdf_task(LPVOID lpParam)
{
    init_t initial = (init_t)lpParam;
    HPDF_Doc pdf;
    HPDF_Font font;
    HPDF_Page page;
    HPDF_Destination dst;
    const char *ret;

    pdf = HPDF_New(error_handler, NULL);
    if (!pdf)
    {
        printf("error: cannot create PdfDoc object\n");
        return 0;
    }

    /* error-handler */
    if (setjmp(env))
    {
        HPDF_Free(pdf);
        return 0;
    }

    /* create default-font */
    font = HPDF_GetFont(pdf, HPDF_LoadTTFontFromFile(pdf, ttf_file_name, HPDF_FALSE), NULL);

    /* add a new page object. */
    page = HPDF_AddPage(pdf);

    HPDF_Page_SetWidth(page, 650);
    HPDF_Page_SetHeight(page, 500);

    dst = HPDF_Page_CreateDestination(page);
    HPDF_Destination_SetXYZ(dst, 0, HPDF_Page_GetHeight(page), 1);
    HPDF_SetOpenAction(pdf, dst);

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, font, 20);
    HPDF_Page_MoveTextPos(page, 220, HPDF_Page_GetHeight(page) - 70);
    HPDF_Page_ShowText(page, "1+1=2");
    HPDF_Page_EndText(page);

    HPDF_Page_SetFontAndSize(page, font, 12);

    /* save the document to a file */
    HPDF_SaveToFile(pdf, fname);

    /* clean up */
    HPDF_Free(pdf);

    return 0;
}

DWORD WINAPI cli_task(LPVOID lpParam)
{
    init_t initial = (init_t)lpParam;
    return 0;
}