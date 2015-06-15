#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <verbiste/c-api.h>


int main(int argc, char *argv[])
{
    Verbiste_ModeTensePersonNumber *vec;
    size_t i;
    const char *libdatadir;
    char conjFN[512], verbsFN[512];

    if (argc < 2)
    {
        printf("deconjugator.c: demo of the C API of Verbiste\n");
        printf("Usage: deconjugator VERB\n");
        printf("Note: this program expects UTF-8 and writes UTF-8.\n");
        return EXIT_FAILURE;
    }

    setlocale(LC_CTYPE, "");  // necessary on Solaris
    libdatadir = getenv("LIBDATADIR");
    if (libdatadir == NULL)
        libdatadir = LIBDATADIR;

    snprintf(conjFN, sizeof(conjFN), "%s/conjugation-fr.xml", libdatadir);
    snprintf(verbsFN, sizeof(verbsFN), "%s/verbs-fr.xml", libdatadir);

    if (verbiste_init(conjFN, verbsFN, "fr") != 0)
    {
        printf("deconjugator.c: failed to initialize Verbiste.\n");
        return EXIT_FAILURE;
    }

    vec = verbiste_deconjugate(argv[1]);

    if (vec == NULL)
    {
        fprintf(stderr, "Internal error in libverbiste.\n");
        return EXIT_FAILURE;
    }


    for (i = 0; vec[i].infinitive_verb != NULL; i++)
    {
        printf("%s, %s, %s, %d, %s\n",
                    vec[i].infinitive_verb,
                    (const char*) verbiste_get_mode_name(vec[i].mode),
                    (const char*) verbiste_get_tense_name(vec[i].tense),
                    vec[i].person,
                    vec[i].plural ? "plural" : "singular");
    }
    printf("\n");

    verbiste_free_mtpn_array(vec);

    verbiste_close();

    return EXIT_SUCCESS;
}
