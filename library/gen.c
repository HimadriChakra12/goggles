/* ggen - your goggle lives here, edit and run make */
#include "../tools/gen.h"

static const Meta meta = {
	.name        = "Library",
	.description = "Curated Library for free reading",
	.public      = 0,
	.author      = "yourname",
	.avatar      = "#1793d1",
};

/*
 * action: BOOST, DOWNRANK, DISCARD or RAW
 * strength: 1-10 (0 = default)
 * targets: space separated
 *   example.com          site rule
 *   /path/ *foo* |http   url pattern, anything with / * ^ |
 *   pattern@example.com  pattern limited to one site
 *   @list.txt            read targets from a file, # starts a comment
 * DISCARD with no targets drops everything not matched
 * RAW writes the string as is
 */
static const Rule rules[] = {
	/* { DISCARD, 0, "" }, */

	{ BOOST,    3,  "annas-archive.gl annas-archive.pk annas-archive.gd "
                    "libgen.li libgen.gl libgen.bz libgen.vg "
                    "www.gutenberg.org "
                    "oceanofpdf.com"},
	{ BOOST,    2,  "archive.org/details/texts" },
	{ BOOST,    1,  "z-lib.gd z-library.sk 1lib.sk z-lib.fm" },
};

int
main(int argc, char *argv[])
{
	return gen(argc, argv, &meta, rules, LEN(rules));
}
