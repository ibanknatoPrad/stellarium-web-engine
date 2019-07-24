/* Stellarium Web Engine - Copyright (c) 2019 - Noctua Software Ltd
 *
 * This program is licensed under the terms of the GNU AGPL v3, or
 * alternatively under a commercial licence.
 *
 * The terms of the AGPL v3 license can be found in the main directory of this
 * repository.
 */

#include "grid_cache.h"
#include "swe.h"
#include "utils/cache.h"

#include <stdlib.h>
#include <string.h>

#define CACHE_SIZE (2 * (1 << 20))

static cache_t *g_cache = NULL;

/*
 * Function: grid_cache_get
 * Return a pointer to an healpix grid for fast texture projection
 *
 * Parameters:
 *   order      - Healpix pixel order argument.
 *   pix        - Healpix pix.
 *   split      - Number of splits to use for the grid.
 *
 * Return:
 *   A (split + 1)^2 grid of 3d positions.
 */
const double (*grid_cache_get(int order, int pix,
                              int split))[4]
{
    int n = split + 1;
    int i, j;
    double (*grid)[4];
    struct {
        int nside;
        int pix;
        int split;
        int pad_;
    } key = { order, pix, split };
    uv_map_t map;
    double p[4];

    uv_map_init_healpix(&map, order, pix, true, true);

    _Static_assert(sizeof(key) == 16, "");
    if (!g_cache) g_cache = cache_create(CACHE_SIZE);
    grid = cache_get(g_cache, &key, sizeof(key));
    if (grid) return grid;
    grid = calloc(n * n, sizeof(*grid));
    for (i = 0; i < n; i++)
    for (j = 0; j < n; j++) {
        vec3_set(p, (double)j / split, (double)i / split, 1.0);
        uv_map(&map, p, p);
        grid[i * n + j][0] = p[0];
        grid[i * n + j][1] = p[1];
        grid[i * n + j][2] = p[2];
        grid[i * n + j][3] = p[3];
    }
    cache_add(g_cache, &key, sizeof(key), grid, sizeof(*grid) * n * n, NULL);
    return grid;
}
