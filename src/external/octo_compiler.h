#pragma once

/**
*
*  octo_compiler.h
*
*  a compiler for Octo CHIP-8 assembly language,
*  suitable for embedding in other tools and environments.
*  depends only upon the C standard library.
*
*  the public interface is octo_compile_str(char*);
*  the result will contain a 64k ROM image in the
*  'rom' field of the returned octo_program.
*  octo_free_program can clean up the entire structure
*  when a consumer is finished using it.
*
*  https://github.com/JohnEarnest/c-octo/blob/main/src/octo_compiler.h
**/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <map>
#include <vector>
#include <string>
/**
*
*  Fundamental Data Structures
*
**/

#define OCTO_LIST_BLOCK_SIZE 16
#define OCTO_RAM_MAX         (64*1024)
#define OCTO_INTERN_MAX      (64*1024)
#define OCTO_ERR_MAX         4096
#define OCTO_DESTRUCTOR(x) ((void(*)(void*))x)

double octo_sign(double x);

double octo_max(double x, double y);

double octo_min(double x, double y);

typedef struct {
  int count, space;
  void **data;
} octo_list;

// linear access should be good enough for a start;
// octo programs rarely have more than a few thousand constants:
typedef struct {
  octo_list keys, values;
} octo_map;

typedef struct {
  double value;
  char is_mutable;
} octo_const;

typedef struct {
  int value;
} octo_reg;

typedef struct {
  int value;
  char is_long;
} octo_pref;

typedef struct {
  int line, pos;
  octo_list addrs;
} octo_proto;

typedef struct {
  int calls;
  octo_list args, body;
} octo_macro;

typedef struct {
  int calls;
  char values[256];
  octo_macro *modes[256];
} octo_smode;

typedef struct {
  int addr, line, pos;
  char *type;
} octo_flow;

typedef struct {
  int type, base, len;
  char *format;
} octo_mon;

#define OCTO_TOK_STR 0
#define OCTO_TOK_NUM 1
#define OCTO_TOK_EOF 2

typedef struct {
  int type;
  int line;
  int pos;
  union {
    char *str_value;
    double num_value;
  };
} octo_tok;

// I could just use lists directly, but this abstraction clarifies intent:
typedef struct {
  octo_list values;
} octo_stack;

struct octo_listing_item {
  int line;
  int pos;
  int addr;
  uint16_t data;
};

typedef struct {
  // string interning table
  size_t strings_used;
  char strings[OCTO_INTERN_MAX];

  // tokenizer
  char *source;
  char *source_root;
  int source_line;
  int source_pos;
  octo_list tokens;

  octo_tok *current_token;

  // compiler
  char has_main;    // do we need a trampoline for 'main'?
  int here;
  int length;
  char rom[OCTO_RAM_MAX];
  char used[OCTO_RAM_MAX];
  octo_map constants;   // name -> octo_const
  octo_map aliases;     // name -> octo_reg
  octo_map protos;      // name -> octo_proto
  octo_map macros;      // name -> octo_macro
  octo_map stringmodes; // name -> octo_smode
  octo_stack loops;       // [octo_flow]
  octo_stack branches;    // [octo_flow]
  octo_stack whiles;      // [octo_flow], value=-1 indicates a marker

  // debugging
  char *breakpoints[OCTO_RAM_MAX];
  octo_map monitors; // name -> octo_mon

  // error reporting
  char is_error;
  char error[OCTO_ERR_MAX];
  int error_line;
  int error_pos;

  // Listing
  std::map<uint16_t, octo_listing_item> *listing;

} octo_program;

void octo_free_program(octo_program *p);

octo_program *octo_compile_str(char *text);

void *octo_map_get(octo_map *map, char *key);

void *octo_map_set(octo_map *map, char *key, void *value);