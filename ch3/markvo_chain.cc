#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

const int kPrefNum = 2;

const int kHashNum = 4093;

const int kMaxGen = 10000;

template<typename Val>
struct ListItem {
    Val val;
    ListItem *next;
};

typedef ListItem<char *> Suffix;

struct StateImp {
    char *pref[kPrefNum];
    Suffix *suf;
};

typedef ListItem<StateImp> State;

State *statetab[kHashNum];

Suffix *suffixtab[kHashNum];

void build(char *prefix[kPrefNum], FILE *f);

void add(char *prefix[kPrefNum], char *suffix);

void generate(int num);

void addsuffix(State *sp, char *suf);

State *lookup(char *pref[kPrefNum], int create);

unsigned int hash(char *pref) {
    unsigned int h;
    unsigned char *p;
    h = 0;
    for ((p = (unsigned char *)pref); *p != '\0'; ++p ) {
        h = h * 31 + *p;
    }
    return h % kHashNum;
}

unsigned int hash(char *pref[kPrefNum]) {
    return (((uintptr_t(pref[0]) >> 1) + (uintptr_t(pref[1]) >> 1)) % kHashNum);
}

Suffix *lookup_suffix(char *suf, int create) {
    Suffix *sp;
    int h;
    h = hash(suf);
    for (sp = suffixtab[h]; sp != NULL; sp = sp->next) {
        if (strcmp(sp->val, suf) == 0) {
            return sp;
        }
    }
    if (create) {
        sp = (Suffix *) malloc(sizeof(Suffix));
        sp->val = strdup(suf);
        sp->next = suffixtab[h];
        suffixtab[h] = sp;
    }
    return sp;
}

void build(char *prefix[kPrefNum], FILE *f) {
    char buf[100], fmt[10];
    sprintf(fmt, "%%%lds", sizeof(buf)-1);
    while (fscanf(f, fmt, buf) != EOF) {
        add(prefix, lookup_suffix(buf, 1)->val);
    }
}

void add(char *prefix[kPrefNum], char *suffix) {
    State *sp;    
    sp = lookup(prefix, 1);
    addsuffix(sp, suffix);
    memmove(prefix, prefix + 1, sizeof(prefix[0])*(kPrefNum-1));
    prefix[kPrefNum-1] = suffix;
}

State *lookup(char *pref[kPrefNum], int create) {
    State *sp;
    int h, i;
    h = hash(pref);
    for (sp = statetab[h]; sp != NULL; sp = sp->next) {
        for (i = 0; i < kPrefNum; ++i) {
            if (sp->val.pref[i] != pref[i]) {
                break;
            }
        }
        if (i == kPrefNum) {
            return sp;
        }
    }
    if (create) {
        sp = (State *) malloc(sizeof(State));
        for (i = 0; i < kPrefNum; ++i) {
            sp->val.pref[i] = pref[i];
        }
        sp->val.suf = NULL;
        sp->next = statetab[h];
        statetab[h] = sp;
    }
    return sp;
}

void addsuffix(State *sp, char *suf) {
    Suffix *s;
    s = (Suffix *) malloc(sizeof(Suffix));
    s->val = suf;
    s->next = sp->val.suf;
    sp->val.suf = s;
}

char NONWORD[] = "\n";

void generate(int nwords) {
    State *sp;
    Suffix *suf;
    char *prefix[kPrefNum], *w;
    int i, nmatch;
    char *nwp = lookup_suffix(NONWORD, 0)->val;
    if (!nwp) return;
    for (i = 0; i < kPrefNum; ++i) {
        prefix[i] = nwp;
    }
    unsigned int seed = (unsigned int) time(NULL);
    srand(seed);
    for (i = 0; i < nwords; ++i) {
        sp = lookup(prefix, 0);
        nmatch = 0;
        if (!sp) break;
        for (suf = sp->val.suf; suf != NULL; suf = suf->next) {
            if (rand() % ++nmatch == 0) {
                w = suf->val;
            }
        }
        if (nwp == w) {
            break;
        }
        printf("%s ", w);
        memmove(prefix, prefix + 1, (kPrefNum - 1)*sizeof(prefix[0]));
        prefix[kPrefNum-1] = w;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) return 1;
    int i, nwords = kMaxGen;
    char *prefix[kPrefNum];
    char *nwp = lookup_suffix(NONWORD, 1)->val;
    for (i = 0; i < kPrefNum; ++i) {
        prefix[i] = nwp;
    }
    build(prefix, fopen(argv[1], "r"));
    add(prefix, NONWORD);
    State *sp;
    for (int j = 0; j < kHashNum; ++j) {
        for (sp = statetab[j]; sp != NULL; sp = sp->next) {
            printf("state: %d->{ ", j);
            for (int k = 0; k < kPrefNum; ++k) {
                printf("pref[%d]: %p:%s ", k, sp->val.pref[k], sp->val.pref[k]);
            }
            printf("}\n");
        }
    }
    generate(nwords);
    return 0;
}
