/* ggen - your goggle lives here, edit and run make */
#include "../tools/gen.h"

static const Meta meta = {
	.name        = "Movies",
	.description = "Wanna See Free Movies",
	.public      = 0,
	.author      = "Himadri",
};

static const Rule rules[] = {
	{ BOOST,    3,  "fmovies.org "
                    "fmovies.tips "
                    "fmovies4k.org "
                    "fmovies-4u.com "
                    "fmovies-to.pro "
    },
	{ BOOST,    2,  "1show.org "
                    "pahe.in "
    },
	{ DOWNRANK, 2, "imdb.com" },
	{ DISCARD,  0, "" },
};

int
main(int argc, char *argv[])
{
	return gen(argc, argv, &meta, rules, LEN(rules));
}
