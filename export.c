#include "export.h"
#include "vec.inl"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void GridTr_load_shape_from_obj(struct GridTr_shape_s *shape,
                                const char *filename) {
  if (!shape) {
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
      char *toks[3];
      toks[0] = strtok(line + 2, " ");
      toks[1] = strtok(NULL, " ");
      toks[2] = strtok(NULL, " \n");
      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 16; j++) {
          if (toks[i][j] == '/') {
            toks[i][j] = '\0';
            break;
          }
        }
      }
      face.x = atoi(toks[0]) - 1;
      face.y = atoi(toks[1]) - 1;
      face.z = atoi(toks[2]) - 1;
      shape->faces[shape->num_faces++] = face;
    }
  }
  fclose(fp);
}

char *GridTr_export_shape_to_obj_str(const struct GridTr_shape_s *shape,
                                     struct vec3_s o, float scale,
                                     uint32 starting_vertex) {
  if (!shape) {
    return NULL;
  }

  uint est = shape->num_ps * 128 + shape->num_faces * 128;

  // Create a string buffer to hold the OBJ data
  char *buffer = (char *)GridTr_new(est * sizeof(char));
  if (!buffer) {
    return NULL;
  }

  buffer[0] = '\0'; // Initialize the buffer as an empty string
  char line[128];
  // Write vertex positions
  for (uint32 i = 0; i < shape->num_ps; i++) {
    struct vec3_s p = shape->ps[i];
    p = vec3_add(vec3_mul(p, scale), o);
    sprintf(line, "v %f %f %f\n", p.x, p.y, p.z);
    strcat(buffer, line);
  }

  // Write face indices
  for (uint32 i = 0; i < shape->num_faces; i++) {
    struct ivec3_s face = shape->faces[i];
    face.x += starting_vertex;
    face.y += starting_vertex;
    face.z += starting_vertex;
    // face indices are 1-based in OBJ format
    sprintf(line, "f %d %d %d\n", face.x + 1, face.y + 1, face.z + 1);
    strcat(buffer, line);
  }
  return buffer;
}

void GridTr_free_shape(struct GridTr_shape_s *shape) {
  if (shape) {
    GridTr_free(shape->ps);
    GridTr_free(shape->faces);
    shape->num_faces = shape->num_ps = 0;
  }
}

void GridTr_export_grid_boxes_to_obj(const struct GridTr_grid_s *grid,
                                     const char *filename) {
  if (!grid || !filename) {
    return;
  }
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    printf("<%s> - failed to open file '%s' for writing\n", __FUNCTION__,
           filename);
    return;
  }

  struct GridTr_shape_s cell_shape;
  GridTr_load_shape_from_obj(&cell_shape, "cube.obj");
  if (!cell_shape.ps || !cell_shape.faces) {
    printf("<%s> - failed to load shape from 'cube.obj'\n", __FUNCTION__);
    return;
  }

  uint32 num_cells = 0;
  uint32 vertex_start = 0;
  const void **cells = GridTr_grid_get_all_grid_cells(grid, &num_cells);
  for (uint32 i = 0; i < num_cells; i++) {
    const struct GridTr_grid_cell_s *cell =
        (const struct GridTr_grid_cell_s *)cells[i];
    struct GridTr_aabb_s aabb;
    GridTr_get_aabb_for_grid_cell(cell->crl, grid->cell_size, &aabb);
    char *str = GridTr_export_shape_to_obj_str(&cell_shape, aabb.o,
                                               grid->cell_size, vertex_start);
    if (str) {
      fprintf(fp, "%s", str);
      GridTr_free(str);
    } else {
      printf("<%s> - failed to export shape for cell <%d, %d, %d>\n",
             __FUNCTION__, cell->crl.x, cell->crl.y, cell->crl.z);
    }
    vertex_start += cell_shape.num_ps;
  }
  fclose(fp);
  GridTr_free_shape(&cell_shape);

  void *ptr = (void *)cells;
  GridTr_free(ptr);
}
