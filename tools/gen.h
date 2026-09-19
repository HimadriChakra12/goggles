/* gen.h - engine for gen, checks and prints a Brave Search goggle */
#ifndef GEN_H
#define GEN_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEN(a)  (sizeof(a) / sizeof((a)[0]))
#define MAXLINE 4096
#define MAXINS  500
#define MAXNUM  100000
#define MAXSIZE (2 * 1024 * 1024)
#define MAXSTR  10

enum { BOOST, DOWNRANK, DISCARD, RAW };

typedef struct {
	const char *name;
	const char *description;
	const char *author;
	int public;
	const char *avatar;
	const char *homepage;
	const char *issues;
	const char *license;
	const char *transferred_to;
} Meta;

typedef struct {
	int action;
	int strength;
	const char *targets;
} Rule;

static const char *actions[] = { "boost", "downrank", "discard", "raw" };
static FILE *body;
static int ninst;
static const char *curfile;
static int curline;
static int currule;

static void
die(const char *fmt, ...)
{
	va_list ap;

	fputs("gen: ", stderr);
	if (curfile)
		fprintf(stderr, "%s:%d: ", curfile, curline);
	else if (currule)
		fprintf(stderr, "rule %d: ", currule);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	exit(1);
}

static char *
next(char **s)
{
	char *t;

	while (isspace((unsigned char)**s))
		(*s)++;
	if (!**s)
		return NULL;
	t = *s;
	while (**s && !isspace((unsigned char)**s))
		(*s)++;
	if (**s)
		*(*s)++ = '\0';
	return t;
}

static int
count(const char *s, int c)
{
	int n = 0;

	for (; *s; s++)
		n += *s == c;
	return n;
}

static int
hexcolor(const char *s)
{
	if (*s++ != '#' || strlen(s) != 6)
		return 0;
	for (; *s; s++)
		if (!isxdigit((unsigned char)*s))
			return 0;
	return 1;
}

static void
put(const char *ins)
{
	if (strlen(ins) > MAXINS)
		die("instruction over %d chars: %.40s...", MAXINS, ins);
	if (count(ins, '*') > 2)
		die("more than 2 '*' in: %s", ins);
	if (count(ins, '^') > 2)
		die("more than 2 '^' in: %s", ins);
	if (++ninst > MAXNUM)
		die("more than %d instructions", MAXNUM);
	fprintf(body, "%s\n", ins);
}

static void
target(const char *act, int str, char *tok)
{
	char ins[MAXLINE + 128], opt[32];
	char *at, *site = NULL;

	if (strspn(tok, "0123456789") == strlen(tok))
		die("'%s' is a bare number, put the strength in the rule", tok);
	if (strchr(tok, '$'))
		die("'$' not allowed in '%s', use RAW", tok);

	if (str > 1)
		snprintf(opt, sizeof opt, "%s=%d", act, str);
	else
		snprintf(opt, sizeof opt, "%s", act);

	at = strrchr(tok, '@');
	if (at && at != tok && !strpbrk(at + 1, "/*^|") && strchr(at + 1, '.')) {
		*at = '\0';
		site = at + 1;
	}

	if (site)
		snprintf(ins, sizeof ins, "%s$%s,site=%s", tok, opt, site);
	else if (strpbrk(tok, "/*^|"))
		snprintf(ins, sizeof ins, "%s$%s", tok, opt);
	else
		snprintf(ins, sizeof ins, "$%s,site=%s", opt, tok);
	put(ins);
}

static void
include(const char *path, const char *act, int str)
{
	FILE *f;
	char line[MAXLINE], *p, *t;

	if (!(f = fopen(path, "r")))
		die("cannot open %s: %s", path, strerror(errno));
	curfile = path;
	curline = 0;
	while (fgets(line, sizeof line, f)) {
		curline++;
		if (!strchr(line, '\n') && !feof(f))
			die("line too long");
		p = line;
		while ((t = next(&p)) && *t != '#')
			target(act, str, t);
	}
	fclose(f);
	curfile = NULL;
}

static void
addrule(const Rule *r)
{
	const char *act = actions[r->action];
	char *buf, *p, *t;
	int str = r->strength ? r->strength : 1;

	if (r->action == RAW) {
		put(r->targets);
		return;
	}
	if (r->action == DISCARD && r->strength)
		die("discard takes no strength");
	if (str < 1 || str > MAXSTR)
		die("strength must be 1-%d", MAXSTR);

	if (!(buf = strdup(r->targets ? r->targets : "")))
		die("out of memory");
	p = buf;
	if (!(t = next(&p))) {
		if (r->action != DISCARD)
			die("%s needs at least one target", act);
		put("$discard");
	}
	for (; t; t = next(&p)) {
		if (t[0] == '@' && t[1])
			include(t + 1, act, str);
		else
			target(act, str, t);
	}
	free(buf);
}

static void
hdr(FILE *f, const char *key, const char *val)
{
	if (val)
		fprintf(f, "! %s: %s\n", key, val);
}

static void
shareurl(const char *u)
{
	fputs("https://search.brave.com/goggles?goggles_id=", stdout);
	for (; *u; u++) {
		if (isalnum((unsigned char)*u) || strchr("-._~", *u))
			putchar(*u);
		else
			printf("%%%02X", (unsigned char)*u);
	}
	putchar('\n');
	fputs("gen: submit the url at https://search.brave.com/goggles/create first\n", stderr);
}

static int
gen(int argc, char *argv[], const Meta *m, const Rule *r, size_t n)
{
	FILE *head;
	char *hbuf, *bbuf;
	size_t hlen, blen, i;

	if (argc == 3 && !strcmp(argv[1], "-u")) {
		shareurl(argv[2]);
		return 0;
	}
	if (argc != 1) {
		fprintf(stderr, "usage: %s > out.goggle\n"
		        "       %s -u raw-goggle-url\n", argv[0], argv[0]);
		return 1;
	}
	if (!m->name || !m->description || !m->author)
		die("name, description and author are required");
	if (m->avatar && !hexcolor(m->avatar))
		die("avatar must look like #rrggbb");

	if (!(body = open_memstream(&bbuf, &blen)))
		die("open_memstream: %s", strerror(errno));
	for (i = 0; i < n; i++) {
		currule = i + 1;
		addrule(&r[i]);
	}
	currule = 0;
	fclose(body);
	if (!ninst)
		die("no rules");

	if (!(head = open_memstream(&hbuf, &hlen)))
		die("open_memstream: %s", strerror(errno));
	hdr(head, "name", m->name);
	hdr(head, "description", m->description);
	hdr(head, "public", m->public ? "true" : "false");
	hdr(head, "author", m->author);
	hdr(head, "avatar", m->avatar);
	hdr(head, "homepage", m->homepage);
	hdr(head, "issues", m->issues);
	hdr(head, "license", m->license);
	hdr(head, "transferred_to", m->transferred_to);
	fputc('\n', head);
	fclose(head);

	if (hlen + blen > MAXSIZE)
		die("goggle is %zu bytes, limit is %d", hlen + blen, MAXSIZE);
	fwrite(hbuf, 1, hlen, stdout);
	fwrite(bbuf, 1, blen, stdout);
	fprintf(stderr, "gen: %d instructions, %zu bytes\n", ninst, hlen + blen);
	return 0;
}

#endif
