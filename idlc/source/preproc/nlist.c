/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

extern int Cplusplus;
Nlist *kwdefined;
char wd[128];

/* 
	ER: Tabelle extra gross gemacht, da es anscheinend ein Problem mit der
	der Verkettung gibt, irgendwann irgendwo wird mal ein nlist->next
	ueberschrieben, was in eineme SIGSEGV resultiert.
	Den GDB mit watchpoint hab ich aber nach 2 Tagen abgebrochen..
	so loeppt's jedenfalls erstmal..
 */
#define	NLSIZE 15000

static Nlist *nlist[NLSIZE];

struct kwtab
{
    char *kw;
    int val;
    int flag;
}   kwtab[] =

{
    { "if", KIF, ISKW },
    { "ifdef", KIFDEF, ISKW },
    { "ifndef", KIFNDEF, ISKW },
    { "elif", KELIF, ISKW },
    { "else", KELSE, ISKW },
    { "endif", KENDIF, ISKW },
    { "include", KINCLUDE, ISKW },
    { "include_next", KINCLUDENEXT, ISKW },
    { "import", KIMPORT, ISKW },
    { "define", KDEFINE, ISKW },
    { "undef", KUNDEF, ISKW },
    { "line", KLINE, ISKW },
    { "error", KERROR, ISKW },
    { "pragma", KPRAGMA, ISKW },
    { "ident", KIDENT, ISKW },
    { "eval", KEVAL, ISKW },
    { "defined", KDEFINED, ISDEFINED + ISUNCHANGE },
    { "machine", KMACHINE, ISDEFINED + ISUNCHANGE },
    { "__LINE__", KLINENO, ISMAC + ISUNCHANGE },
    { "__FILE__", KFILE, ISMAC + ISUNCHANGE },
    { "__DATE__", KDATE, ISMAC + ISUNCHANGE },
    { "__TIME__", KTIME, ISMAC + ISUNCHANGE },
    { "__STDC__", KSTDC, ISUNCHANGE },
    { NULL, 0, 0 }
};

unsigned long namebit[077 + 1];

void
    setup_kwtab(void)
{
    struct kwtab *kp;
    Nlist *np;
    Token t;
    static Token deftoken[1] = {{NAME, 0, 0, 7, (uchar *) "defined"}};
    static Tokenrow deftr = {deftoken, deftoken, deftoken + 1, 1};

    for (kp = kwtab; kp->kw; kp++)
    {
        t.t = (uchar *) kp->kw;
        t.len = strlen(kp->kw);
        np = lookup(&t, 1);
        np->flag = (char) kp->flag;
        np->val = (char) kp->val;
        if (np->val == KDEFINED)
        {
            kwdefined = np;
            np->val = NAME;
            np->vp = &deftr;
            np->ap = 0;
        }
    }
}

Nlist *
    lookup(Token * tp, int install)
{
    unsigned int h;
    Nlist *np;
    uchar *cp, *cpe;

    h = 0;
    for (cp = tp->t, cpe = cp + tp->len; cp < cpe;)
        h += *cp++;
    h %= NLSIZE;
    np = nlist[h];
    while (np)
    {
        if (*tp->t == *np->name && tp->len == (unsigned int)np->len
            && strncmp((char *)tp->t, (char *)np->name, tp->len) == 0)
            return np;
        np = np->next;
    }
    if (install)
    {
        np = new(Nlist);
        np->vp = NULL;
        np->ap = NULL;
        np->flag = 0;
        np->val = 0;
        np->len = tp->len;
        np->name = newstring(tp->t, tp->len, 0);
        np->next = nlist[h];
        nlist[h] = np;
        quickset(tp->t[0], tp->len > 1 ? tp->t[1] : 0);
        return np;
    }
    return NULL;
}
