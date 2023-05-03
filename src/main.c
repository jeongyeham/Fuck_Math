#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "hpdf.h"
#include "windows.h"

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

typedef struct _init
{
    // topics settings
    int min_number;
    int max_number;
    enum_operate operation;
    int bit_depth;
    int topics_num;
    BOOL is_valid;

    // HPDF settings
    int paper_width;
    int paper_height;
    int font_size;
} init_t;

jmp_buf env;
HANDLE mutex_handle;

const char *fname = "output.pdf";
const char *ttf_file_name = ".\\input.ttf";

DWORD WINAPI pdf_task(LPVOID lpParam);
DWORD WINAPI cli_task(LPVOID lpParam);

void error_handler(HPDF_STATUS error_no,
                   HPDF_STATUS detail_no,
                   void *user_data)
{
    printf("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
           (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

int main(void)
{
    init_t init_env = {0};
    char ini_input_buff;
    DWORD pdf_task_ID = -1;
    DWORD cli_task_ID = -1;

    printf("Has a ini file ? (Y or N) : ");
    scanf("%s", &ini_input_buff);

    if (strcmp(&ini_input_buff, "Y") == 0 || strcmp(&ini_input_buff, "Y") == 0)
    {
        goto HAS_INI;
    }
    else if (strcmp(&ini_input_buff, "N") == 0)
    {
        goto NOT_HAS_INI;
    }
    else
    {
        goto HAS_SHIT;
    }

NOT_HAS_INI:
    printf("please input min_number\n");
    scanf("%d", &init_env.min_number);

    printf("please input max_number\n");
    scanf("%d", &init_env.max_number);

    printf("please input operation\n");
    printf("  and                    -----> 0\n \
              minus                  -----> 1\n \
              times                  -----> 2\n \
              divided                -----> 3\n \
              and_minus_mixed        -----> 4\n \
              times_divided_mixed    -----> 5\n \
              all_mixed              -----> 6\n");
    scanf("%d", &init_env.operation);

    printf("please input min_number\n");
    scanf("%d", &init_env.min_number);

    printf("please input min_number\n");
    scanf("%d", &init_env.min_number);


HAS_INI:
    printf("okk\n");
    
    mutex_handle = CreateMutex((LPSECURITY_ATTRIBUTES)NULL,
                               FALSE,
                               NULL);

    HANDLE pdf_task_handle = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                          (SIZE_T)0,
                                          (LPTHREAD_START_ROUTINE)&pdf_task,
                                          (LPVOID)&init_env,
                                          (DWORD)0,
                                          (LPDWORD)&pdf_task_ID);

    HANDLE cli_task_handle = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                          (SIZE_T)0,
                                          (LPTHREAD_START_ROUTINE)&cli_task,
                                          (LPVOID)&init_env,
                                          (DWORD)0,
                                          (LPDWORD)&cli_task_ID);

HAS_SHIT:
    printf("Goodbye\n");
    system("pause");

    return 0;
};

DWORD WINAPI pdf_task(LPVOID lpParam)
{
    init_t *initial = (init_t *)lpParam;
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
    init_t *initial = (init_t *)lpParam;

    return 0;
}