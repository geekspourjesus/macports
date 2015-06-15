#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <verbiste/c-api.h>


static
int
demo(const char *utf8_infinitive)
{
    int k;

    for (k = 0;
        verbiste_valid_modes_and_tenses[k].mode != VERBISTE_INVALID_MODE;
        k++)
    {
        Verbiste_Mode mode = verbiste_valid_modes_and_tenses[k].mode;
        Verbiste_Tense tense = verbiste_valid_modes_and_tenses[k].tense;
        Verbiste_PersonArray person_array;
        Verbiste_TemplateArray template_array;
        size_t t, i;

        template_array = verbiste_get_verb_template_array(utf8_infinitive);
        if (template_array == NULL)
        {
            printf("Unknown infinitive.\n");
            return EXIT_FAILURE;  // caution: this lets arrays leak
        }

        for (t = 0; template_array[t] != NULL; ++t)
        {
            person_array = verbiste_conjugate(utf8_infinitive, template_array[t], mode, tense, 0);

            if (person_array == NULL)
            {
                printf("Unknown infinitive.\n");
                return EXIT_FAILURE;  // caution: this lets arrays leak
            }

            printf("- %s %s:\n",
                (const char *) verbiste_get_mode_name(mode),
                (const char *) verbiste_get_tense_name(tense));

            for (i = 0; person_array[i] != NULL; i++)
            {
                size_t j;

                for (j = 0; person_array[i][j] != NULL; j++)
                {
                    if (j != 0)
                        printf(", ");
                    printf("%s", person_array[i][j]);
                }
                printf("\n");
            }

            verbiste_free_person_array(person_array);
        }

        verbiste_free_verb_template_array(template_array);
    }
    printf("-\n");
    return EXIT_SUCCESS;
}


int main( int argc, char *argv[])
{
    const char *libdatadir;
    char conjFN[512], verbsFN[512];
    int exit_status;

    if (argc < 2)
    {
        printf("conjugator.c: demo of the C API of Verbiste\n");
        printf("Usage: conjugator VERB\n");
        printf("Note: this program expects Latin-1 and writes Latin-1.\n");
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
        printf("conjugator.c: failed to initialize Verbiste.\n");
        return EXIT_FAILURE;
    }

    exit_status = demo(argv[1]);

    verbiste_close();

    return exit_status;
}
