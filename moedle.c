// In case of running under Windows, include header file
#if _MSC_VER
#include <Windows.h>
#endif

// load std header files for I/O, string and bool operations etc.
#include <ctype.h>       // tolower()
#include <stdbool.h>     // bool, true und false
#include <stdio.h>       // stdin, getchar(), fgets()
#include <stdlib.h>      // atoi(), srand(), rand()
#include <string.h>      // strncmp(), strchr()
#include <time.h>        // time()

// include word list
#include "words.h"

#define WORD_BUF_LEN (WORD_LENGTH + 1)
#define MAX_TRIES (6)

// each letter of the entered word will be marked correspondingly:
// `NOT_PRESENT` - letter is not present in the word we look for
// `PRESENT` - letter is present but in the wrong position
// `CORRECT` - letter is at the correct position
enum status
{
    UNMARKED,     // 0
    NOT_PRESENT,  // 1
    PRESENT,      // 2
    CORRECT       // 3
};

typedef enum status state_t;

// this struct contains current state of our round in the game
typedef struct
{
    // pointer to our target word in the word list
    const char* word;
    // currently entered word
    char guess[WORD_BUF_LEN];
    // marks for the letters and corresponding positions
    state_t result[WORD_LENGTH];
    // mark whether a letter has already been used
    bool used[WORD_LENGTH];
    // number of trial
    int n_tries;
} game_state;

// checks whether entered word exists in the word list
bool word_is_allowed(const char* word)
{
    // sequential search for the word in the list
    for (int i = 0; words[i] != NULL; ++i)
    {
        if (strncmp(word, words[i], WORD_LENGTH) == 0)
            return true;
    }
    return false;
}

// check whether letter of the trial word exists in the target word
// and, if so, mark correspondingly returning true (false otherwise)
bool is_character_unmarked(game_state* state, char c)
{
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        if (state->word[i] == c && state->used[i] == false) {
            state->used[i] = true;
            return true;
        }
    }
    return false;
}

// den Zustand der laufenden Raterunde aktualisieren
void update_state(game_state *state)
{
    // Jedes Zeichen als unmarkiert kennzeichnen
    for (int i = 0; i < WORD_LENGTH; ++i) {
        state->result[i] = UNMARKED;
        state->used[i] = false;
    }

    // korrekt platzierte Zeichen finden und als solche markieren
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        if (state->guess[i] == state->word[i]) {
            state->result[i] = CORRECT;
            state->used[i] = true;
        }
    }

    // noch mal alle Zeichen durchgehen und die als
    // vorhanden, aber fehlplatziert markieren, die nicht
    // bereits markiert sind
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        // Zeichen ist als korrekt markiert, also überspringen
        if (state->result[i] == CORRECT)
            continue;
        state->result[i] = is_character_unmarked(state, state->guess[i])
            ? PRESENT : NOT_PRESENT;
    }
}

void get_input(game_state* state)
{
    // solange eine Eingabe anfordern, bis sie gültig ist
    bool bad_word;
    do
    {
        printf("\nAttempt %d: ", state->n_tries);
        // Eingabe lesen
        bad_word = false;
        for (int i = 0; i < WORD_LENGTH; i++) {
            state->guess[i] = getchar();
            if (state->guess[i] == '\n') {
                state->guess[i] = '\0';
                bad_word = true;
                break;
            }
        }
        // überflüssige Zeichen verwerfen
        if (!bad_word)
            while (getchar() != '\n')
                ;
        // nach dem 5. Zeichen abschneiden
        state->guess[WORD_LENGTH] = '\0';
#ifdef DEBUG
        printf("Eingabe: '%s'\n", state->guess);
#endif
        if (bad_word) {
            printf("Please enter %d letters.\n", WORD_LENGTH);
	} else {
	    bad_word = !word_is_allowed(state->guess);
	    if (bad_word)
		    printf("This word is not in our word list.\n");
	}

    }
    while (bad_word);
}

// dem User Feedback geben, wie gut sein Rateversuch war
void print_result(const game_state* state)
{
    // Das Ergebnis ein bisschen hübsch aufbereiten
    printf("! ");
    for (int i = 0; i < WORD_LENGTH; ++i)
    {
        switch (state->result[i])
        {
        case CORRECT:
            // korrekt platzierte Buchstaben erscheinen auf
            // grünem Hintergrund
            printf("\033[37;42;1m");
            break;
        case PRESENT:
            // vorhandene, aber fehlplatzierte Buchstaben erscheinen auf
            // gelbem Hintergrund
            printf("\x1b[37;43;1m");
            break;
        case NOT_PRESENT:
            // fall-through
        default:
            // nicht vorhandene Buchstaben erscheinen auf
            // rotem Hintergrund
            printf("\033[37;41;1m");
            break;
        }
        printf("%c", state->guess[i]);
    }
    // Schrift- und Hintergrundfarbe auf Defaults zurücksetzen
    printf("\033[0m\n");
}

bool another_round(void)
{
    printf("Wanna play another round? (Y/n) ");
    char answer = (char)tolower(getchar());
    // überflüssige Zeichen verwerfen
    if (answer != '\n')
        while (getchar() != '\n')
            ;
    bool yes = answer == 'y' || answer == '\n';
    if (yes)
        printf("\nAlright! Let's go ...\n");
    return yes;
}

// Einstieg ins Programm
int main(int argc, char* argv[])
{
    unsigned int seed = (argc > 1)
        ? (unsigned int)atoi(argv[1])
        : (unsigned int)time(NULL);
    srand(seed);
    printf("\nMOEDLE - a wordle clone\n\n"
           "Try to find the work with %d letters in maximum %d attempts.\n"
           "(Cancel with Ctrl+C)\n",
           WORD_LENGTH, MAX_TRIES);

    bool keepRunning = true;
    while (keepRunning)
    {
        game_state state;
        int num_words;
        // Wörter zählen
        for (num_words = 0; words[num_words] != NULL;  num_words++)
           ;
        // ein Wort daraus zufällig auswählen
        state.word = words[rand() % num_words];
#ifdef DEBUG
        state.word = "cebit";
        printf("Hint: %s\n", state.word);
#endif
        // eine Raterunde läuft über maximal 6 (`MAX_TRIES`) Versuche
        bool doRestart = false;
        for (state.n_tries = 1;
             state.n_tries <= MAX_TRIES && !doRestart;
             ++state.n_tries)
        {
            // User raten lassen
            get_input(&state);
            // geratenes Wort auswerten
            update_state(&state);
            // Feedback geben
            print_result(&state);

            // geratenes Wort mit dem gesuchten vergleichen, wenn identisch,
            // hat der User das Spiel gewonnen
            if (strncmp(state.guess, state.word, WORD_LENGTH) == 0)
            {
                printf("\nCongrats, you got the word in %d. attempts!\n",
                       state.n_tries);
                doRestart = true;
                keepRunning = another_round();
            }
            else if (state.n_tries == MAX_TRIES)
            {
                printf("\nGame over!!!\n"
                       "The word we looked for is: %s.\n",
                       state.word);
                keepRunning = another_round();
            }
        }
    }
    printf("\nBye ...\n\n");
    return EXIT_SUCCESS;
}
