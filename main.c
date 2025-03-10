#include "vma.c"
#include <string.h>

// verific daca a fost citita o comanda invalida
void mesaj(char *op)
{
if (strcmp(op, "ALLOC_ARENA") != 0  && strcmp(op, "ALLOC_BLOCK") != 0)
	if (strcmp(op, "PMAP") != 0 && strcmp(op, "DEALLOC_ARENA") != 0)
		if (strcmp(op, "FREE_BLOCK") != 0 && strcmp(op, "WRITE") != 0)
			if (strcmp(op, "READ") != 0 && strcmp(op, "MPROTECT") != 0)
				printf("Invalid command. Please try again.\n");
}

//in functie de ce cuvant este citit modific permisiunile
void permissions(char *cuv, int8_t *permission)
{
	if (strcmp(cuv, "PROT_WRITE") == 0)
		*permission += 2;
	if (strcmp(cuv, "PROT_READ") == 0)
		*permission += 4;
	if (strcmp(cuv, "PROT_EXEC") == 0)
		*permission += 1;
}

int main(void)
{
	char *op = malloc(100 * sizeof(char));
	arena_t *arena;
	uint64_t address, size;
	while (scanf("%s", op)) {
		mesaj(op);
		// ii dau arenei dimensiunea citita
		if (strcmp(op, "ALLOC_ARENA") == 0) {
			scanf("%lu", &size);
			arena = alloc_arena(size);
		}
		// aloc blocul cu dimensiunile date
		if (strcmp(op, "ALLOC_BLOCK") == 0) {
			scanf("%lu", &address);
			scanf("%lu", &size);
			alloc_block(arena, address, size);
		}
		if (strcmp(op, "WRITE") == 0) {
			scanf("%lu", &address);
			scanf("%lu", &size);
			char ch = fgetc(stdin);
			char *string = malloc((size + 1) * sizeof(char));
			// citesc de la stdin un numar de caractere egal cu size
			for (int i = 0; i < size; ++i)
				string[i] = fgetc(stdin);
			write(arena, address, size, string);
			free(string);
		}
		if (strcmp(op, "READ") == 0) {
			scanf("%lu", &address);
			scanf("%lu", &size);
			read(arena, address, size);
		}
		if (strcmp(op, "MPROTECT") == 0) {
			char *cuv = malloc(100 * sizeof(char));
			char a;
			int8_t permission = 0;
			scanf("%lu", &address);
			scanf("%s", cuv);
			permissions(cuv, &permission);
			a = fgetc(stdin);
			// daca a e spatiu, urmeaza alta permisiune
			if (a == ' ') {
				// citesc caracterul "|"
				scanf("%s", cuv);
				// citesc permisiunea
				scanf("%s", cuv);
				permissions(cuv, &permission);
				a = fgetc(stdin);
					if (a == ' ') {
						scanf("%s", cuv);
						scanf("%s", cuv);
						permissions(cuv, &permission);
					}
			}
			mprotect(arena, address, &permission);
			free(cuv);
		}
		if (strcmp(op, "PMAP") == 0)
			pmap(arena);
		if (strcmp(op, "FREE_BLOCK") == 0) {
			scanf("%lu", &address);
			free_block(arena, address);
		}
		if (strcmp(op, "DEALLOC_ARENA") == 0) {
			// eliberez string-ul pentru cititul comenzilor
			free(op);
			dealloc_arena(arena);
			// ies din program
			break;
		}
	}
}