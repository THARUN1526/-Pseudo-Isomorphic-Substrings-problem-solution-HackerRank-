#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct range_minimum_query {
    int n;
    int **t;
};

struct range_minimum_query *
create_range_minimum_query(
    int n,
    int *a
    )
{
    int i;
    int j;
    struct range_minimum_query *rmq;

    if (n < 1) {
        fprintf(stderr, "ERROR: n < 1\n");
        exit(EXIT_FAILURE);
    }

    rmq = malloc(sizeof(struct range_minimum_query));
    if (rmq == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    rmq->n = n;
    rmq->t = malloc(sizeof(int *) * n);
    if (rmq->t == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    j = 1;
    while ((1 << j) <= n) {
        j++;
    }

    for (i = 0; i < n; i++) {
        if ((i + (1 << (j - 1))) > n) {
            j--;
        }

        rmq->t[i] = malloc(sizeof(int) * j);
        if (rmq->t[i] == NULL) {
            fprintf(stderr, "ERROR: malloc failed.\n");
            exit(EXIT_FAILURE);
        }

        rmq->t[i][0] = a[i];
    }

    for (j = 1; (1 << j) <= n; j++) {
        for (i = 0; (i + (1 << j)) <= n; i++) {
            if (rmq->t[i][j - 1] <= rmq->t[i + (1 << (j - 1))][j - 1]) {
                rmq->t[i][j] = rmq->t[i][j - 1];
            } else {
                rmq->t[i][j] = rmq->t[i + (1 << (j - 1))][j - 1];
            }
        }
    }

    return rmq;
}

void
free_range_minimum_query(
    struct range_minimum_query *rmq
    )
{
    int i;

    for (i = 0; i < rmq->n; i++) {
        free(rmq->t[i]);
    }

    free(rmq->t);
    free(rmq);

    return;
}

int
min_int(
    int x,
    int y
    )
{
    return ((x <= y) ? x : y);
}

int
lookup_range_minimum_query(
    struct range_minimum_query *rmq,
    int i,
    int j
    )
{
    int k;

    if (i > j) {
        fprintf(stderr, "ERROR: i > j\n");
        exit(EXIT_FAILURE);
    }

    if (i == j) {
        return rmq->t[i][0];
    }

    k = 0;
    for (;;) {
        if ((i + (1 << k) - 1) == j) {
            return rmq->t[i][k];
        }

        if ((i + (1 << (k + 1)) - 1) > j) {
            return min_int(rmq->t[i][k], rmq->t[j - (1 << k) + 1][k]);
        }

        k++;
    }
}

#define ALPHABET_FIRST_CHAR 'a'
#define ALPHABET_LAST_CHAR  'z'
#define ALPHABET_SIZE       (ALPHABET_LAST_CHAR - ALPHABET_FIRST_CHAR + 1)

struct pseudo_isomorphic_suffix_array {
    int n;
    char *string;
    int *suffix_index;
    struct range_minimum_query *lcp_rmq;
};

struct spanning_previous_links {
    int count;
    int array[ALPHABET_SIZE];
};

struct character_ranks_for_suffix {
    int rank[ALPHABET_SIZE];
};

struct pseudo_isomorphic_suffix_array_context {
    int n;
    char *string;
    int *distance_from_previous;
    struct spanning_previous_links *spanning_previous_links;
    struct character_ranks_for_suffix *character_ranks_for_suffix;
    int *index;
    int *temp;
    int *rank;
    int *lcp;
    struct range_minimum_query *lcp_rmq;
    int *group;
    int iteration;
};

#if VERBOSE

void
print_int_array(
    char *prefix,
    int *a,
    int n
    )
{
    int i;

    fprintf(stderr, "%s", prefix);
    for (i = 0; i < n; i++) {
        fprintf(stderr, " %d", a[i]);
    }
    fprintf(stderr, "\n");

    return;
}

void
print_pseudo_isomorphic_suffix_array_context(
    struct pseudo_isomorphic_suffix_array_context *ctx
    )
{
    fprintf(stderr, "INFO: CONTEXT: Iteration %d\n", ctx->iteration);
    print_int_array("INFO: CONTEXT:   index:", ctx->index, ctx->n);
    print_int_array("INFO: CONTEXT:    rank:", ctx->rank, ctx->n);
    print_int_array("INFO: CONTEXT:     lcp:", ctx->lcp, ctx->n);
    print_int_array("INFO: CONTEXT:   group:", ctx->group, ctx->n);

    return;
}

#endif

void
initialize_pseudo_isomorphic_suffix_array_context(
    struct pseudo_isomorphic_suffix_array_context *ctx,
    char *s,
    int n
    )
{
    int i;
    int j;
    int k;
    int previous[ALPHABET_SIZE];
    int sorted_by_next[ALPHABET_SIZE];

#if VERBOSE

    fprintf(stderr, "INFO: string:");
    for (i = 0; i < n; i++) {
        fprintf(stderr, " %c", s[i]);
    }
    fprintf(stderr, "\n");

#endif

    ctx->n = n;
    ctx->string = malloc(sizeof(char) * (n + 1));
    if (ctx->string == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(ctx->string, s, sizeof(char) * n);
    ctx->string[n] = 0;

    ctx->distance_from_previous = malloc(sizeof(int) * n);
    if (ctx->distance_from_previous == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < ALPHABET_SIZE; i++) {
        previous[i] = -1;
    }

    for (i = 0; i < n; i++) {
        ctx->distance_from_previous[i] =
            i - previous[s[i] - ALPHABET_FIRST_CHAR];
        previous[s[i] - ALPHABET_FIRST_CHAR] = i;
    }

#if VERBOSE

    fprintf(stderr, "INFO: distance_from_previous:");
    for (i = 0; i < n; i++) {
        fprintf(stderr, " %d", ctx->distance_from_previous[i]);
    }
    fprintf(stderr, "\n");

#endif

    ctx->spanning_previous_links =
        malloc(sizeof(struct spanning_previous_links) * n);
    if (ctx->spanning_previous_links == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < n; i++) {
        ctx->spanning_previous_links[i].count = 0;
    }

    for (i = 0; i < n; i++) {
        for (j = i - ctx->distance_from_previous[i]; j < i; j++) {
            if (j >= 0) {
                ctx->spanning_previous_links[j]
                    .array[ctx->spanning_previous_links[j].count] = i;
                ctx->spanning_previous_links[j].count++;
            }
        }
    }

#if VERBOSE

    for (i = 0; i < n; i++) {
        fprintf(stderr, "INFO: spanning_previous_links[%d]:", i);
        for (j = 0; j < ctx->spanning_previous_links[i].count; j++) {
            fprintf(stderr, " %d", ctx->spanning_previous_links[i].array[j]);
        }
        fprintf(stderr, "\n");
    }

#endif

    ctx->character_ranks_for_suffix =
        malloc(sizeof(struct character_ranks_for_suffix) * n);
    if (ctx->character_ranks_for_suffix == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < ALPHABET_SIZE; i++) {
        sorted_by_next[i] = i;
    }

    for (i = n - 1; i >= 0; i--) {
        j = s[i] - ALPHABET_FIRST_CHAR;

        k = 0;
        while (sorted_by_next[k] != j) {
            k++;
        }

        while (k > 0) {
            sorted_by_next[k] = sorted_by_next[k - 1];
            k--;
        }

        sorted_by_next[0] = j;

        for (k = 0; k < ALPHABET_SIZE; k++) {
            ctx->character_ranks_for_suffix[i].rank[sorted_by_next[k]] = k;
        }
    }

#if VERBOSE

    for (i = 0; i < n; i++) {
        fprintf(stderr, "INFO: character_ranks_for_suffix[%d]:", i);
        for (j = 0; j < ALPHABET_SIZE; j++) {
            fprintf(stderr, " %d", ctx->character_ranks_for_suffix[i].rank[j]);
        }
        fprintf(stderr, "\n");
    }

#endif

    ctx->index = malloc(sizeof(int) * n);
    if (ctx->index == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    ctx->temp = malloc(sizeof(int) * n);
    if (ctx->temp == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    ctx->rank = malloc(sizeof(int) * n);
    if (ctx->rank == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    ctx->lcp = malloc(sizeof(int) * n);
    if (ctx->lcp == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    ctx->group = malloc(sizeof(int) * n);
    if (ctx->group == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < n; i++) {
        ctx->index[i] = i;
        ctx->rank[i] = i;
    }

    for (i = 1; i < n; i++) {
        ctx->lcp[i] = 1;
    }

    ctx->lcp_rmq = create_range_minimum_query(n, ctx->lcp);

    for (i = 0; i < n; i++) {
        ctx->group[i] = 0;
    }

    ctx->iteration = 0;

    return;
}

int
compute_lcp_in_pseudo_isomorphic_suffix_array_context(
    struct pseudo_isomorphic_suffix_array_context *ctx,
    int s1,
    int s2
    )
{
    int i;
    int j;
    struct spanning_previous_links *spl;
    int ss1;
    int ss2;
    int x;
    int y;

    if (s1 == s2) {
        fprintf(stderr, "ERROR: s1 == s2\n");
        exit(EXIT_FAILURE);
    }

    if (ctx->group[s1] != ctx->group[s2]) {
        if (ctx->rank[s1] < ctx->rank[s2]) {
            i = ctx->rank[s1] + 1;
            j = ctx->rank[s2];
        } else {
            i = ctx->rank[s2] + 1;
            j = ctx->rank[s1];
        }

        return lookup_range_minimum_query(ctx->lcp_rmq, i, j);
    }

    x = 1 << ctx->iteration;
    ss1 = s1 + x;
    ss2 = s2 + x;
    if ((ss1 >= ctx->n) || (ss2 >= ctx->n)) {
        return x;
    }

    if (ctx->group[ss1] == ctx->group[ss2]) {
        y = x + x;
    } else {
        if (ctx->rank[ss1] < ctx->rank[ss2]) {
            i = ctx->rank[ss1] + 1;
            j = ctx->rank[ss2];
        } else {
            i = ctx->rank[ss2] + 1;
            j = ctx->rank[ss1];
        }

        y = x + lookup_range_minimum_query(ctx->lcp_rmq, i, j);
    }

    spl = &ctx->spanning_previous_links[ss1 - 1];
    for (i = 0; i < spl->count; i++) {
        j = spl->array[i];
        if (j >= (s1 + y)) {
            break;
        }

        if (((j - ctx->distance_from_previous[j]) >= s1)
            &&
            (ctx->distance_from_previous[j] !=
             ctx->distance_from_previous[s2 + (j - s1)])
            ) {
            y = j - s1;
            break;
        }
    }

    spl = &ctx->spanning_previous_links[ss2 - 1];
    for (i = 0; i < spl->count; i++) {
        j = spl->array[i];
        if (j >= (s2 + y)) {
            break;
        }

        if (((j - ctx->distance_from_previous[j]) >= s2)
            &&
            (ctx->distance_from_previous[j] !=
             ctx->distance_from_previous[s1 + (j - s2)])
            ) {
            y = j - s2;
            break;
        }
    }

    return y;
}

int
compare_suffix_in_pseudo_isomorphic_suffix_array_context(
    struct pseudo_isomorphic_suffix_array_context *ctx,
    int s1,
    int s2
    )
{
    char c1;
    char c2;
    int lcp;
    int r1;
    int r2;
    int ss1;
    int ss2;

    if (s1 == s2) {
        fprintf(stderr, "ERROR: s1 == s2\n");
        exit(EXIT_FAILURE);
    }

    lcp = compute_lcp_in_pseudo_isomorphic_suffix_array_context(ctx, s1, s2);

    ss1 = s1 + lcp;
    ss2 = s2 + lcp;

    if (ss1 >= ctx->n) {
        return -1;
    }

    if (ss2 >= ctx->n) {
        return 1;
    }

    c1 = ctx->string[ss1];
    c2 = ctx->string[ss2];

    r1 = ctx->character_ranks_for_suffix[s1].rank[c1 - ALPHABET_FIRST_CHAR];
    r2 = ctx->character_ranks_for_suffix[s2].rank[c2 - ALPHABET_FIRST_CHAR];

    return r1 - r2;
}

void
run_merge_sort_in_pseudo_isomorphic_suffix_array_context(
    struct pseudo_isomorphic_suffix_array_context *ctx,
    int l,
    int r
    )
{
    int i;
    int j;
    int k;
    int m;
    int t;

    if (l > r) {
        fprintf(stderr, "ERROR: l > r\n");
        exit(EXIT_FAILURE);
    }

    if (l == r) {
        return;
    }

    if ((l + 1) == r) {
        if (compare_suffix_in_pseudo_isomorphic_suffix_array_context(
                ctx,
                ctx->index[l],
                ctx->index[r]) <= 0) {
            return;
        }

        t = ctx->index[l];
        ctx->index[l] = ctx->index[r];
        ctx->index[r] = t;
        return;
    }

    m = (l + r) >> 1;
    run_merge_sort_in_pseudo_isomorphic_suffix_array_context(ctx, l, m - 1);
    run_merge_sort_in_pseudo_isomorphic_suffix_array_context(ctx, m, r);

    i = l;
    j = m;
    k = l;

    while ((i < m) && (j <= r)) {
        if (compare_suffix_in_pseudo_isomorphic_suffix_array_context(
                ctx,
                ctx->index[i],
                ctx->index[j]) <= 0) {
            ctx->temp[k] = ctx->index[i];
            i++;
        } else {
            ctx->temp[k] = ctx->index[j];
            j++;
        }
        k++;
    }

    while (i < m) {
        ctx->temp[k] = ctx->index[i];
        i++;
        k++;
    }

    while (j <= r) {
        ctx->temp[k] = ctx->index[j];
        j++;
        k++;
    }

    for (k = l; k <= r; k++) {
        ctx->index[k] = ctx->temp[k];
    }

    return;
}

void
update_lcps_in_pseudo_isomorphic_suffix_array_context(
    struct pseudo_isomorphic_suffix_array_context *ctx
    )
{
    int i;

    for (i = 1; i < ctx->n; i++) {
        ctx->lcp[i] =
            compute_lcp_in_pseudo_isomorphic_suffix_array_context(
                ctx, ctx->index[i - 1], ctx->index[i]);
    }

    free_range_minimum_query(ctx->lcp_rmq);

    ctx->lcp_rmq = create_range_minimum_query(ctx->n, ctx->lcp);

    return;
}

void
update_ranks_in_pseudo_isomorphic_suffix_array_context(
    struct pseudo_isomorphic_suffix_array_context *ctx
    )
{
    int i;
    int j;

    for (i = 0; i < ctx->n; i++) {
        j = ctx->index[i];
        ctx->rank[j] = i;
    }

    return;
}

void
update_groups_in_pseudo_isomorphic_suffix_array_context(
    struct pseudo_isomorphic_suffix_array_context *ctx
    )
{
    int group_index;
    int i;
    int max_lcp;

    group_index = 0;
    max_lcp = 1 << ctx->iteration;

    ctx->group[ctx->index[0]] = group_index;
    for (i = 1; i < ctx->n; i++) {
        if (ctx->lcp[i] != max_lcp) {
            group_index++;
        }

        ctx->group[ctx->index[i]] = group_index;
    }

    return;
}

struct pseudo_isomorphic_suffix_array *
create_pseudo_isomorphic_suffix_array_from_context(
    struct pseudo_isomorphic_suffix_array_context *ctx
    )
{
    struct pseudo_isomorphic_suffix_array *pisa;

    pisa = malloc(sizeof(struct pseudo_isomorphic_suffix_array));
    if (pisa == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    pisa->n = ctx->n;
    pisa->string = ctx->string;
    pisa->suffix_index = ctx->index;
    pisa->lcp_rmq = ctx->lcp_rmq;

    free(ctx->distance_from_previous);
    free(ctx->spanning_previous_links);
    free(ctx->character_ranks_for_suffix);
    free(ctx->temp);
    free(ctx->rank);
    free(ctx->lcp);
    free(ctx->group);

    return pisa;
}

struct pseudo_isomorphic_suffix_array *
create_pseudo_isomorphic_suffix_array(
    char *s,
    int n
    )
{
    struct pseudo_isomorphic_suffix_array_context ctx;
    int i;

    if (n < 1) {
        fprintf(stderr, "ERROR: n < 1\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < n; i++) {
        if ((s[i] < ALPHABET_FIRST_CHAR) || (s[i] > ALPHABET_LAST_CHAR)){
            fprintf(stderr,
                    "ERROR: %c is not in the supported alphabet.\n",
                    s[i]);
            exit(EXIT_FAILURE);
        }
    }

    initialize_pseudo_isomorphic_suffix_array_context(&ctx, s, n);

#if VERBOSE

    print_pseudo_isomorphic_suffix_array_context(&ctx);

#endif

    while ((1 << ctx.iteration) < n) {
        run_merge_sort_in_pseudo_isomorphic_suffix_array_context(&ctx,
                                                                 0,
                                                                 n - 1);

        update_lcps_in_pseudo_isomorphic_suffix_array_context(&ctx);

        update_ranks_in_pseudo_isomorphic_suffix_array_context(&ctx);

        ctx.iteration++;

        update_groups_in_pseudo_isomorphic_suffix_array_context(&ctx);

#if VERBOSE

        print_pseudo_isomorphic_suffix_array_context(&ctx);

#endif

    }

    return create_pseudo_isomorphic_suffix_array_from_context(&ctx);
}

void
free_pseudo_isomorphic_suffix_array(
    struct pseudo_isomorphic_suffix_array *pisa
    )
{
    free(pisa->string);
    free(pisa->suffix_index);
    free_range_minimum_query(pisa->lcp_rmq);
    free(pisa);
    return;
}

long *
get_lcp_sums_by_suffix_length(
    char *s,
    int n
    )
{
    int i;
    int j;
    long *lcp_sum;
    int *next;
    struct pseudo_isomorphic_suffix_array *pisa;
    int *prev;
    int *rank;
    int sl;
    long x;
    int y;

    lcp_sum = malloc(sizeof(long) * n);
    if (lcp_sum == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    rank = malloc(sizeof(int) * n);
    if (rank == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    prev = malloc(sizeof(int) * n);
    if (prev == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    next = malloc(sizeof(int) * n);
    if (next == NULL) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(EXIT_FAILURE);
    }

    pisa = create_pseudo_isomorphic_suffix_array(s, n);

    for (i = 0; i < n; i++) {
        j = pisa->suffix_index[i];
        rank[j] = i;
        prev[j] = ((i == 0) ? -1 : pisa->suffix_index[i - 1]);
        next[j] = ((i == (n - 1)) ? -1 : pisa->suffix_index[i + 1]);
    }

    x = 0;
    for (i = 0; i < n; i++) {
        x += pisa->lcp_rmq->t[i][0];
    }

    lcp_sum[n - 1] = x;

    for (sl = n - 1; sl > 0; sl--) {
        i = n - 1 - sl;

        if (prev[i] != -1) {
            y = lookup_range_minimum_query(pisa->lcp_rmq,
                                           rank[prev[i]] + 1,
                                           rank[i]);
            x -= y;
        }

        if (next[i] != -1) {
            y = lookup_range_minimum_query(pisa->lcp_rmq,
                                           rank[i] + 1,
                                           rank[next[i]]);
            x -= y;
        }

        if ((prev[i] != -1) && (next[i] != -1)) {
            y = lookup_range_minimum_query(pisa->lcp_rmq,
                                           rank[prev[i]] + 1,
                                           rank[next[i]]);
            x += y;
        }

        if (prev[i] != -1) {
            next[prev[i]] = next[i];
        }

        if (next[i] != -1) {
            prev[next[i]] = prev[i];
        }

        lcp_sum[sl - 1] = x;
    }

    free_pseudo_isomorphic_suffix_array(pisa);
    free(rank);
    free(prev);
    free(next);

    return lcp_sum;
}

void
reverse(
    char *s,
    int n
    )
{
    int l;
    int r;
    char t;

    l = 0;
    r = n - 1;
    while (l < r) {
        t = s[l];
        s[l] = s[r];
        s[r] = t;
        l++;
        r--;
    }

    return;
}

void
run(
    char *s,
    int n
    )
{
    int l;
    long *lcp_sum;
    long x;

    reverse(s, n);

    lcp_sum = get_lcp_sums_by_suffix_length(s, n);

    x = 0;
    for (l = 1; l <= n; l++) {
        x += l;
        printf("%ld\n", x - lcp_sum[l - 1]);
    }

    return;
}

int
main(
    int argc,
    char **argv
    )
{
    char *s;

    s = malloc(sizeof(char) * (1024 * 1024));
    scanf("%s", s);
    run(s, strlen(s));
    return 0;
}
