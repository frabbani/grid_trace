#pragma once

#include "grid.h"
#include <stdio.h>
#include <stdlib.h>

struct GridTr_shape_s {
  struct vec3_s *ps;
  struct ivec3_s *faces; // assumes triangle faces for now
  uint32 num_ps, num_faces;
};

void GridTr_load_shape_from_obj(struct GridTr_shape_s *shape,
                                const char *filename) {
  if (shape) {
    return;
  }
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    printf("<%s> - Failed to open file '%s'\n", __FUNCTION__, filename);
    return;
  }

  shape->ps = NULL;
  shape->faces = NULL;
  shape->num_ps = shape->num_faces = 0;
  char line[256];

  uint num_ps, num_faces;
  num_ps = num_faces = 0;
  // Read the OBJ file and populate the shape structure
  while (fgets(line, sizeof(line), fp)) {
    // Parse the line and extract vertex/face information
    if (line[0] == 'v' && line[1] == ' ') {
      ++num_ps;
      // ...
    } else if (line[0] == 'f' && line[1] == ' ') {
      ++num_faces;
    }
  }
  shape->ps = GridTr_new(num_ps * sizeof(struct vec3_s));
  shape->faces = GridTr_new(num_faces * sizeof(struct ivec3_s));
  fseek(fp, 0, SEEK_SET);
  while (fgets(line, sizeof(line), fp)) {
    // Parse the line and extract vertex/face information
    if (line[0] == 'v' && line[1] == ' ') {
      // Parse vertex position
      struct vec3_s pos;
      sscanf(line + 2, "%f %f %f", &pos.x, &pos.y, &pos.z);
      shape->ps[shape->num_ps++] = pos;
      // Add position to the shape structure
      // ...
    } else if (line[0] == 'f' && line[1] == ' ') {
      // Parse face
      struct ivec3_s face;
      sscanf(line + 2, "%d %d %d", &face.x, &face.y, &face.z);
      shape->faces[shape->num_faces++] = face;
    }
  }
  fclose(fp);
}

void GridTr_free_shape(struct GridTr_shape_s *shape) {
  if (shape) {
    GridTr_free(shape->ps);
    GridTr_free(shape->faces);
    shape->num_faces = shape->num_ps = 0;
  }
}

void GridTr_export_grid_to_obj(const struct GridTr_grid_s *grid,
                               const struct GridTr_shape_s *shape,
                               const char *filename) {
  if (!shape || !filename) {
    return;
  }
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    printf("<%s> - Failed to open file '%s'\n", __FUNCTION__, filename);
    return;
  }

  uint32 num_cells = 0;
  const void **cell_ptrs =
      GridTr_hash_table_get_all_ro(grid->cell_table, &num_cells);
  printf("<%s> - total %d cells\n", __FUNCTION__, num_cells);
  for (uint32 i = 0; i < num_cells; ++i) {
    const struct GridTr_grid_cell_s *cell =
        (const struct GridTr_grid_cell_s *)cell_ptrs[i];
    // Write cell info as comments
    printf("# Cell <%d, %d, %d>  %u colliders\n", cell->crl.x, cell->crl.y,
           cell->crl.z, cell->num_colliders);
  }
  void *p = (void *)cell_ptrs;
  GridTr_free(p);
  fclose(fp);
}