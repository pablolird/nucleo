#include "song_list.h"
#include "miichannel.h"
#include "songofstorms.h"
#include "nokia.h"
#include "keyboardcat.h"
#include "minuetg.h"
#include "furelise.h"
#include "greenhill.h"
#include "professorlayton.h"
#include "asabranca.h"
#include "doom.h"
#include "odetojoy.h"
#include "supermariobros.h"
#include "greensleeves.h"
#include "cannonind.h"
#include "pacman.h"
#include "pulodagaita.h"
#include "tetris.h"
#include "zeldatheme.h"
#include "thebadinerie.h"
#include "silentnight.h"
#include "thelick.h"
#include "startrekintro.h"
#include "thegodfather.h"
#include "pinkpanther.h"
#include "takeonme.h"
#include "cantinaband.h"
#include "babyelephantwalk.h"
#include "vampirekiller.h"
#include "princeigor.h"
#include "brahmslullaby.h"
#include "zeldaslullaby.h"
#include "imperialmarch.h"
#include "harrypotter.h"
#include "jigglypuffsong.h"
#include "happybirthday.h"
#include "bloodytears.h"
#include "gameofthrones.h"
#include "merrychristmas.h"
#include "starwars.h"
#include "nevergonnagiveyouup.h"
#include "thelionsleepstonight.h"

const Song* all_songs[] = {
    &miichannel_song,
    &songofstorms_song,
    &nokia_song,
    &keyboardcat_song,
    &minuetg_song,
    &furelise_song,
    &greenhill_song,
    &professorlayton_song,
    &asabranca_song,
    &doom_song,
    &odetojoy_song,
    &supermariobros_song,
    &greensleeves_song,
    &cannonind_song,
    &pacman_song,
    &pulodagaita_song,
    &tetris_song,
    &zeldatheme_song,
    &thebadinerie_song,
    &silentnight_song,
    &thelick_song,
    &startrekintro_song,
    &thegodfather_song,
    &pinkpanther_song,
    &takeonme_song,
    &cantinaband_song,
    &babyelephantwalk_song,
    &vampirekiller_song,
    &princeigor_song,
    &brahmslullaby_song,
    &zeldaslullaby_song,
    &imperialmarch_song,
    &harrypotter_song,
    &jigglypuffsong_song,
    &happybirthday_song,
    &bloodytears_song,
    &gameofthrones_song,
    &merrychristmas_song,
    &starwars_song,
    &nevergonnagiveyouup_song,
    &thelionsleepstonight_song
};

const unsigned int song_count = sizeof(all_songs) / sizeof(all_songs[0]);
