/* ggen - your goggle lives here, edit and run make */
#include "../tools/gen.h"

static const Meta meta = {
	.name        = "Linux docs",
	.description = "Boost primary docs, push down content farms",
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

	{ BOOST,    3, "wiki.archlinux.org man7.org docs.kernel.org kernel.org" },
	{ BOOST,    2, "github.com gitlab.com stackoverflow.com" },
	{ BOOST,    2, "/man/@man.archlinux.org" },
	{ BOOST,    1, "*systemd*@freedesktop.org" },
	{ DOWNRANK, 2, "medium.com geeksforgeeks.org" },
	{ DISCARD,  0, "pinterest.com quora.com" },
};

int
main(int argc, char *argv[])
{
	return gen(argc, argv, &meta, rules, LEN(rules));
}
