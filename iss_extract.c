/*
* Copyright (c) 2014 Toni Spets <toni.spets@iki.fi>
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
 * compile: gcc -std=c99 -Wall -o iss_extract iss_extract.c
 * usage: ./iss_extract <l/x> <setup.exe> [files...]
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "pe.h"

#define CHUNK 16384

enum
{
    MODE_UNK,
    MODE_LIST,
    MODE_EXTRACT
};

int seek_and_read(FILE *fh, void *dst, int offset, int length)
{
    fseek(fh, offset, SEEK_SET);
    if (feof(fh)) return 0;
    return fread(dst, length, 1, fh) != 0;
}

char *_strdup(const char *i)
{
    char *ret = malloc(strlen(i) + 1);
    strcpy(ret, i);
    return ret;
}

char *read_string(FILE *fh)
{
    char buf[512];

    for (int i = 0; i <= sizeof buf; i++)
    {
        const char c = fgetc(fh);
        if (feof(fh)) return NULL;

        buf[i] = c;

        if (c == '\0')
            return _strdup(buf);
        else if (!isprint(c))
            return NULL;
    }

    return NULL;
}

int main(int argc, char **argv)
{
    int ret = 1;
    FILE *installer_fh = NULL, *file_fh = NULL;
    int mode = MODE_UNK;
    IMAGE_DOS_HEADER dos_hdr;
    IMAGE_NT_HEADERS nt_hdr;

    fprintf(stderr, "InstallShield Setup Extract / Copyright (c) 2014 Toni Spets <toni.spets@iki.fi>\n\n");

    if (argc < 3 || strlen(argv[1]) > 1)
        goto usage;

    switch (argv[1][0])
    {
        case 'l': mode = MODE_LIST; break;
        case 'x': mode = MODE_EXTRACT; break;
        default: mode = MODE_UNK;
    }

    if (mode == MODE_UNK)
        goto usage;

    installer_fh = fopen(argv[2], "r");
    if (installer_fh == NULL)
    {
        perror(argv[2]);
        goto cleanup;
    }


    if (!seek_and_read(installer_fh, &dos_hdr, 0, sizeof (dos_hdr)))
    {
        perror(argv[2]);
        goto cleanup;
    }

    if (dos_hdr.e_magic != IMAGE_DOS_SIGNATURE)
    {
        fprintf(stderr, "File DOS signature invalid.\n");
        goto cleanup;
    }

    if (!seek_and_read(installer_fh, &nt_hdr, dos_hdr.e_lfanew, sizeof (nt_hdr)))
    {
        perror(argv[2]);
        goto cleanup;
    }

    if (nt_hdr.Signature != IMAGE_NT_SIGNATURE)
    {
        fprintf(stderr, "File NT signature invalid.\n");
        goto cleanup;
    }

    int is_start = 0;

    for (int i = 0; i < nt_hdr.FileHeader.NumberOfSections; i++)
    {
        IMAGE_SECTION_HEADER cur_sct;
        DWORD off = (nt_hdr.FileHeader.SizeOfOptionalHeader + dos_hdr.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER)) + sizeof(IMAGE_SECTION_HEADER) * i;

        if (!seek_and_read(installer_fh, &cur_sct, off, sizeof(IMAGE_SECTION_HEADER)))
        {
            perror(argv[2]);
            goto cleanup;
        }

        if (cur_sct.PointerToRawData + cur_sct.SizeOfRawData > is_start)
            is_start = cur_sct.PointerToRawData + cur_sct.SizeOfRawData;
    }

    if (is_start == 0)
    {
        fprintf(stderr, "Could not find the start of InstallShield data.\n");
        goto cleanup;
    }

    if (fseek(installer_fh, is_start, SEEK_SET) != 0)
    {
        perror("fseek");
        goto cleanup;
    }


    if (mode == MODE_LIST)
    {
        printf("Length     Version          Name             Path\n");
        printf("----------------------------------------------------------------------\n");

        unsigned long total = 0;
        unsigned long files = 0;

        while (1)
        {
            char *name = read_string(installer_fh);
            if (name == NULL) break;
            char *path = read_string(installer_fh);
            char *version = read_string(installer_fh);
            char *length = read_string(installer_fh);
            int ilength = atoi(length);

            printf("%10d %-16s %-16s %s\n", ilength, version, name, path);

            fseek(installer_fh, ilength, SEEK_CUR);

            total += ilength;
            files++;

            free(name);
            free(path);
            free(version);
            free(length);
        }

        printf("----------------------------------------------------------------------\n");
        printf("%10lu                                   %lu files\n", total, files);
    }

    else if (mode == MODE_EXTRACT)
    {
        while(1)
        {
            char *name = read_string(installer_fh);
            if (name == NULL) break;
            char *path = read_string(installer_fh);
            char *version = read_string(installer_fh);
            char *length = read_string(installer_fh);
            int ilength = atoi(length);

            int do_extract = (argc == 3);

            if (argc > 3)
            {
                for (int j = 3; j < argc; j++)
                {
                    if (strcmp(argv[j], (const char *)name) == 0)
                    {
                        do_extract = 1;
                        break;
                    }
                }
            }

            if (do_extract)
            {
                fprintf(stderr, "Extracting %s...\n", name);

                file_fh = fopen((const char *)name, "w");
                if (!file_fh)
                {
                    perror((const char *)name);
                    goto cleanup;
                }

                unsigned long p = 0;
                int8_t buf[CHUNK];
                do {
                    unsigned long next = (p + CHUNK < ilength) ? CHUNK : ilength - p;

                    if (fread(buf, next, 1, installer_fh) != 1)
                    {
                        perror("fread");
                        goto cleanup;
                    }

                    if (fwrite(buf, next, 1, file_fh) != 1)
                    {
                        perror("fwrite");
                        goto cleanup;
                    }

                    p += next;
                } while (p < ilength);

                fclose(file_fh);
                file_fh = NULL;
            }
            else
            {
                fseek(installer_fh, ilength, SEEK_CUR);
            }

            free(name);
            free(path);
            free(version);
            free(length);
        }
    }

    ret = 0;
    goto cleanup;

usage:
    fprintf(stderr, "usage: %s <l/x> <setup.exe> [files...]\n", argv[0]);

cleanup:
    if (installer_fh) fclose(installer_fh);
    if (file_fh) fclose(file_fh);

    return ret;
}
