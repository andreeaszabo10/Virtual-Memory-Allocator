#include "vma.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// creez o noua lista si o initializez
list_t *create(unsigned int data_size)
{
	list_t *dll;

	dll = malloc(sizeof(*dll));

	dll->head = NULL;
	dll->data_size = data_size;
	dll->size = 0;

	return dll;
}

// creez arena si o initializez
arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena = malloc(sizeof(arena_t));
	
	// initializez si aloc lista de blocuri
	arena->alloc_list = malloc(sizeof(list_t));
	
	arena->alloc_list->data_size = sizeof(block_t);
	arena->alloc_list->size = 0;
	arena->alloc_list->head = NULL;
	arena->arena_size = size;
	arena->aloc = 0;
	arena->mini = 0;
	
	return arena;
}

// adaug un nou nod la pozitia n din lista
void *add_node(list_t *list, unsigned int n, const void *data)
{
	if (n < 0)
		return NULL;
	if (n > list->size)
		n = list->size;
	node *nodee = malloc(sizeof(node));
	nodee->data = malloc(list->data_size);
	memcpy(nodee->data, data, list->data_size);
	if (list->size != 0) {
		if (n != 0) {
			// parcurg lista pana ajung la nodul n
			node *p = list->head;
			for (int i = 0; i < n - 1; ++i)
				p = p->next;
			// modific legaturile pentru a adauga noul nod
			if (n == list->size) {
				nodee->next = NULL;
				nodee->prev = p;
			} else {
				nodee->next = p->next;
				nodee->prev = p;
			}
			p->next = nodee;
			list->size++;
			return nodee;
		}
		// daca adaug la inceputul listei
		nodee->next = list->head;
		nodee->prev = NULL;
		list->head = nodee;
		list->size++;
		return nodee;
	}
	// daca este primul nod adaugat
	list->head = nodee;
	list->head->next = NULL;
	list->head->prev = NULL;
	list->size++;
	return nodee;
}

// eliberez resursele
void dealloc_arena(arena_t *arena)
{
	// parcurg fiecare nod din lista incepand cu head-ul si il eliberez
	for (int i = 0; i < arena->alloc_list->size; ++i) {
		node *p = arena->alloc_list->head;
		// modific head-ul sa fie urmatorul nod
		arena->alloc_list->head = p->next;
		for (int i = 0; i < ((block_t *)p->data)->miniblock_list->size; ++i) {
			// parcurg fiecare minibloc pentru blocul curent si il eliberez
			node *q = ((block_t *)p->data)->miniblock_list->head;
			((block_t *)p->data)->miniblock_list->head = q->next;
			if (((miniblock_t *)q->data)->verify == 1)
				free(((miniblock_t *)q->data)->rw_buffer);
			free(q->data);
			free(q);
		}
		free(((block_t *)p->data)->miniblock_list);
		free(p->data);
		free(p);
	}
	// eliberez lista de blocuri si arena
	free(arena->alloc_list);
	free(arena);
}

//	verific daca exista un bloc adiacent adresei primite
node *findnode(arena_t *arena, const uint64_t address)
{
	node *a = arena->alloc_list->head;
	int ok = 1;
	while (a) {
		int start = ((block_t *)a->data)->start_address;
		if (start - 1 == address)
			return a;
		if (start + ((block_t *)a->data)->size == address)
			return a;
		a = a->next;
	}
	return a;
}

// pentru cazurile de eroare referitoare la o zona deja alocata
int error(arena_t *arena, uint64_t address, uint64_t size)
{
	node *p = arena->alloc_list->head;
	while (p) {
		int start = ((block_t *)p->data)->start_address;

		// adresa de inceput a noului bloc este mai mica, dar
		// adresa e de final acestuia intra peste o zona alocata
		if (start >= address && start < address + size) {
			printf("This zone was already allocated.\n");
			return 0;
		}

		// adresa de inceput a noului bloc e mai mare decat cea de start a
		// vechiului bloc, e mai mica decat cea de final a vechiului bloc
		if (address > start && address < start + ((block_t *)p->data)->size) {
			printf("This zone was already allocated.\n");
			return 0;
		}

		// dimensiunea noului bloc este mai mare decat cea a vechiului
		// bloc, cel nou incluzand in intregime zona alocata pentru cel vechi
		if (address < start && (address + size) > start) {
			printf("This zone was already allocated.\n");
			return 0;
		}
		p = p->next;
	}
	return 1;
}

void verify(arena_t *arena)
{
	node *ok = arena->alloc_list->head;
	int start, prevsize;
	while (ok->next) {
		start = ((block_t *)ok->data)->start_address;
		prevsize = ((block_t *)ok->data)->size;
		int nextsize = ((block_t *)ok->next->data)->size;
		if (prevsize + start == ((block_t *)ok->next->data)->start_address) {
			((block_t *)ok->data)->size = nextsize + prevsize;
			node *aux = ((block_t *)ok->data)->miniblock_list->head;
			while (aux->next)
				aux = aux->next;
			aux->next = ((block_t *)ok->next->data)->miniblock_list->head;
			((block_t *)ok->next->data)->miniblock_list->head->prev = aux;
			int nextminsize = ((block_t *)ok->next->data)->miniblock_list->size;
			((block_t *)ok->data)->miniblock_list->size += nextminsize;
			node *x = ok->next->next;
			if (x)
				x->prev = ok;
			free(((block_t *)ok->next->data)->miniblock_list);
			free(ok->next->data);
			free(ok->next);
			ok->next = x;
			arena->alloc_list->size--;
		} else {
			ok = ok->next;
		}
	}
}

void initializemini(const uint64_t address, const uint64_t size,
					miniblock_t *mini)
{
	mini->verify = 0;
	mini->start_address = address;
	mini->size = size;
	mini->perm = 6;
}

void initializelist(arena_t *arena, const uint64_t address, const uint64_t size)
{
	node *block = malloc(sizeof(node));
	block_t *new = malloc(sizeof(block_t));
	new->start_address = address;
	new->size = size;
	block->next = NULL;
	block->prev = NULL;
	arena->alloc_list->size++;
	new->miniblock_list = create(sizeof(miniblock_t));
	node *mini = malloc(sizeof(node));
	miniblock_t *aux = malloc(sizeof(miniblock_t));
	initializemini(address, size, aux);
	mini->next = NULL;
	mini->prev = NULL;
	new->miniblock_list->head = mini;
	new->miniblock_list->size++;
	block->data = (block_t *)new;
	mini->data = (miniblock_t *)aux;
	arena->alloc_list->head = block;
	arena->mini++;
	arena->aloc += size;
}

void solo_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	block_t *block = malloc(sizeof(block_t));
	node *prev = arena->alloc_list->head;
	int count = 0;
	while (prev) {
		count++;
		if (((block_t *)prev->data)->start_address < address && prev->next)
			prev = prev->next;
		else
			break;
	}
	if (prev->next && address > ((block_t *)prev->data)->start_address)
		prev = prev->prev;
	block->start_address = address;
	block->size = size;
	block->miniblock_list = create(sizeof(miniblock_t));
	miniblock_t *mini = malloc(sizeof(miniblock_t));
	initializemini(address, size, mini);
	node *newmini = add_node(block->miniblock_list, 0, (miniblock_t *)mini);
	block->miniblock_list->head = newmini;
	node *newblock;
	if (address > ((block_t *)prev->data)->start_address) {
		newblock = add_node(arena->alloc_list, count, (block_t *)block);
	} else {
		newblock = add_node(arena->alloc_list, count - 1, (block_t *)block);
		int var = ((block_t *)arena->alloc_list->head->data)->start_address;
		if (((block_t *)newblock->data)->start_address < var)
			arena->alloc_list->head = newblock;
	}
	arena->aloc += size;
	arena->mini++;
	free(mini);
	free(block);
}

void middle_block(arena_t *arena, const uint64_t address,
				  const uint64_t size, node *ok)
{
	miniblock_t *mini1 = malloc(sizeof(miniblock_t));
	initializemini(address, size, mini1);
	long s = ((block_t *)ok->data)->miniblock_list->size;
	node *mini;
	mini = add_node(((block_t *)ok->data)->miniblock_list, s, mini1);
	((block_t *)ok->next->data)->miniblock_list->head->prev = mini;
	mini->next = ((block_t *)ok->next->data)->miniblock_list->head;
	int prevsize1 = ((block_t *)ok->data)->size;
	int prevsize2 = ((block_t *)ok->next->data)->size;
	((block_t *)ok->data)->size = prevsize1 + size + prevsize2;
	long nxtmini = ((block_t *)ok->next->data)->miniblock_list->size;
	((block_t *)ok->data)->miniblock_list->size += nxtmini;
	node *nod = ok->next->next;
	if (nod)
		nod->prev = ok;
	free(((block_t *)ok->next->data)->miniblock_list);
	free(ok->next->data);
	free(ok->next);
	ok->next = nod;
	arena->alloc_list->size--;
	arena->aloc += size;
	arena->mini++;
	free(mini1);
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (arena->arena_size <= address) {
		printf("The allocated address is outside the size of arena\n");
		return;
	}
	if (arena->arena_size < address + size) {
		printf("The end address is past the size of the arena\n");
		return;
	}
	if (!arena->alloc_list->head) {
		initializelist(arena, address, size);
		return;
	}
	verify(arena);
	node *ok = findnode(arena, address);
	if (!ok) {
		int verify = error(arena, address, size);
		if (verify == 0)
			return;
		solo_block(arena, address, size);
		return;
	}
	if (ok->next) {
		long start = ((block_t *)ok->data)->start_address;
		long sz = ((block_t *)ok->data)->size;
		long nxt = ((block_t *)ok->next->data)->start_address;
		if (address == start + sz && arena->mini > 1 && address + size == nxt) {
			int verify = error(arena, address, size);
			if (verify == 0)
				return;
			middle_block(arena, address, size, ok);
			return;
		}
	}
	if (address > ((block_t *)ok->data)->start_address) {
		int verify = error(arena, address, size);
		if (verify == 0)
			return;
		arena->aloc += size;
		int start = ((block_t *)ok->data)->start_address;
		int prevsize = ((block_t *)ok->data)->size;
		((block_t *)ok->data)->size = size + prevsize;
		miniblock_t *mini1 = malloc(sizeof(miniblock_t));
		initializemini(address, size, mini1);
		long sz = ((block_t *)ok->data)->miniblock_list->size;
		node *p = add_node(((block_t *)ok->data)->miniblock_list, sz, mini1);
		arena->mini++;
		free(mini1);
		return;
	}
	if (address < ((block_t *)ok->data)->start_address) {
		int verify = error(arena, address, size);
		if (verify == 0)
			return;
		arena->aloc += size;
		int start = ((block_t *)ok->data)->start_address;
		int prevsize = ((block_t *)ok->data)->size;
		((block_t *)ok->data)->start_address = address;
		((block_t *)ok->data)->size = size + prevsize;
		miniblock_t *mini = malloc(sizeof(miniblock_t));
		initializemini(address, size, mini);
		node *p = add_node(((block_t *)ok->data)->miniblock_list, 0, mini);
		((block_t *)ok->data)->miniblock_list->head = p;
		arena->mini++;
		free(mini);
		return;
	}
}

void free_block(arena_t *arena, const uint64_t address)
{
	node *a = arena->alloc_list->head;
	node *aux;
	int count = 0, nrmini = 0;
	int ok = 1;
	while (a) {
		count++;
		nrmini = 0;
		aux = (((block_t *)a->data)->miniblock_list)->head;
		while (aux) {
			nrmini++;
			if (((miniblock_t *)aux->data)->start_address == address) {
				ok = 0;
				break;
			}
			aux = aux->next;
		}
		if (ok == 0)
			break;
		a = a->next;
	}
	if (ok == 1) {
		printf("Invalid address for free.\n");
		return;
	}
	long start = ((block_t *)a->data)->start_address;
	long sz = ((block_t *)a->data)->size;
	long minisz = ((miniblock_t *)aux->data)->size;
	if (((block_t *)a->data)->miniblock_list->size == 1) {
		arena->alloc_list->head = a->next;
		arena->alloc_list->size--;
		arena->aloc = arena->aloc - ((block_t *)a->data)->size;
		arena->mini--;
		free(aux->data);
		free(aux);
		free(((block_t *)a->data)->miniblock_list);
		free(a->data);
		free(a);
		return;
	}
	if (aux->next && aux->prev) {
		block_t *block = malloc(sizeof(block_t));
		block->miniblock_list = create(sizeof(miniblock_t));
		block->size = sz - address + start - minisz;
		block->start_address = address + ((miniblock_t *)aux->data)->size;
		block->miniblock_list->head = aux->next;
		long minilistsz = ((block_t *)a->data)->miniblock_list->size;
		block->miniblock_list->size = minilistsz - nrmini;
		node *p = add_node(arena->alloc_list, count, (block_t *)block);
		a->next = p;
		((block_t *)a->data)->size = address - start;
		((block_t *)a->data)->miniblock_list->size = nrmini - 1;
		arena->mini--;
		arena->aloc -= ((miniblock_t *)aux->data)->size;
		aux->prev->next = NULL;
		aux->next->prev = NULL;
		if (((block_t *)a->data)->miniblock_list->size == 1)
			((block_t *)a->data)->miniblock_list->head->next = NULL;
		free(block);
		free(aux->data);
		free(aux);
		return;
	}
	if (aux == ((block_t *)a->data)->miniblock_list->head)
		((block_t *)a->data)->miniblock_list->head = aux->next;
	((block_t *)a->data)->miniblock_list->size--;
	arena->mini--;
	if (aux->next) {
		aux->next->prev = NULL;
		((block_t *)a->data)->size = sz - minisz;
		((block_t *)a->data)->start_address = start + minisz;
	}
	if (aux->prev) {
		((block_t *)a->data)->size = sz - minisz;
		aux->prev->next = NULL;
	}
	arena->aloc -= ((miniblock_t *)aux->data)->size;
	free(aux->data);
	free(aux);
}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	node *a = arena->alloc_list->head;
	node *aux;
	int ok = 1;
	while (a) {
		aux = (((block_t *)a->data)->miniblock_list)->head;
		while (aux) {
			if (((miniblock_t *)aux->data)->start_address == address) {
				ok = 0;
				break;
			}
			aux = aux->next;
		}
		long start = ((block_t *)a->data)->start_address;
		if (start < address && address < start + ((block_t *)a->data)->size)
			ok = 0;
		if (ok == 0)
			break;
		a = a->next;
	}
	if (ok == 1) {
		printf("Invalid address for read.\n");
		return;
	}
	if (!aux)
		aux = ((block_t *)a->data)->miniblock_list->head;
	int perm = ((miniblock_t *)aux->data)->perm;
	if (perm == 0 || perm == 2 || perm == 1 || perm == 3) {
		printf("Invalid permissions for read.\n");
		return;
	}
	node *x = ((block_t *)a->data)->miniblock_list->head;
	int count = address + size;
	while (((miniblock_t *)x->data)->start_address < count) {
		int perm = ((miniblock_t *)x->data)->perm;
		if (perm == 0 || perm == 2 || perm == 1 || perm == 3) {
			printf("Invalid permissions for read.\n");
			return;
		}
		if (!x->next)
			break;
		x = x->next;
	}
	if (size > ((block_t *)a->data)->size) {
		printf("Warning: size was bigger than the block size. ");
		printf("Reading %lu characters.\n", ((miniblock_t *)aux->data)->size);
		for (int i = 0; i < ((miniblock_t *)aux->data)->size; ++i)
			printf("%c", ((char *)((miniblock_t *)aux->data)->rw_buffer)[i]);
		printf("\n");
		return;
	}
	long start = ((block_t *)a->data)->start_address;
	for (int i = 0  + address - start; i < size  + address - start; ++i)
		printf("%c", ((char *)((miniblock_t *)aux->data)->rw_buffer)[i]);
	printf("\n");
}

void write(arena_t *arena, const uint64_t address,
		   const uint64_t size, int8_t *data)
{
	node *a = arena->alloc_list->head;
	node *aux;
	int ok = 1, verif = 1;
	while (a) {
		long start = ((block_t *)a->data)->start_address;
		if (start <= address && ((block_t *)a->data)->size + start > address)
			verif = 0;
		a = a->next;
	}
	if (verif == 1) {
		printf("Invalid address for write.\n");
		return;
	}
	a = arena->alloc_list->head;
	while (a) {
		aux = (((block_t *)a->data)->miniblock_list)->head;
		while (aux) {
			long start = ((miniblock_t *)aux->data)->start_address;
			long minisize = ((miniblock_t *)aux->data)->size;
			if (start <= address && start + minisize > address) {
				ok = 0;
				break;
			}
			aux = aux->next;
		}
		if (ok == 0)
			break;
		a = a->next;
	}
	if (ok == 1 || !arena->alloc_list->head) {
		printf("Invalid address for write.\n");
		return;
	}
	int perm = ((miniblock_t *)aux->data)->perm;
	if (perm == 0 || perm == 4 || perm == 1 || perm == 5) {
		printf("Invalid permissions for write.\n");
		return;
	}
	node *x = ((block_t *)a->data)->miniblock_list->head;
	int count = address + size;
	while (((miniblock_t *)x->data)->start_address < count) {
		int perm = ((miniblock_t *)x->data)->perm;
		if (perm == 0 || perm == 4 || perm == 1 || perm == 5) {
			printf("Invalid permissions for write.\n");
			return;
		}
		if (!x->next)
			break;
		x = x->next;
	}
	char *word = malloc(size * sizeof(char));
	long blsz = ((block_t *)a->data)->size;
	long addr = ((block_t *)a->data)->start_address;
	if (size > blsz + addr - address) {
		printf("Warning: size was bigger than the block size. Writing ");
		printf("%lu characters.\n", blsz + addr - address);
		for (int i = 0; i < blsz + addr - address; ++i)
			word[i] = data[i];
		while (aux) {
			((miniblock_t *)aux->data)->rw_buffer = malloc(size * sizeof(char));
			long wordsize = blsz + addr - address;
			memcpy(((miniblock_t *)aux->data)->rw_buffer, word, wordsize);
			((miniblock_t *)aux->data)->verify = 1;
			aux = aux->next;
		}
		free(word);
		return;
	}
	for (int i = 0; i < size + 0; ++i)
		word[i] = data[i];
	((miniblock_t *)aux->data)->rw_buffer = malloc(size * sizeof(char));
	memcpy(((miniblock_t *)aux->data)->rw_buffer, word, size);
	((miniblock_t *)aux->data)->verify = 1;
	free(word);
}

void pmap(const arena_t *arena)
{
	int freemem = arena->arena_size - arena->aloc;
	printf("Total memory: 0x%lX bytes\n", arena->arena_size);
	printf("Free memory: 0x%lX bytes\n", arena->arena_size - arena->aloc);
	printf("Number of allocated blocks: %d\n", arena->alloc_list->size);
	printf("Number of allocated miniblocks: %ld\n", arena->mini);
	node *a = arena->alloc_list->head;
	for (int i = 0; i < arena->alloc_list->size; ++i) {
		long st = ((block_t *)a->data)->start_address;
		printf("\nBlock %d begin\n", i + 1);
		printf("Zone: 0x%lX - 0x%lX\n", st, st + ((block_t *)a->data)->size);
		node *x = ((block_t *)(a->data))->miniblock_list->head;
		for (int j = 0; j < ((block_t *)a->data)->miniblock_list->size; ++j) {
			long sm = ((miniblock_t *)x->data)->start_address;
			if (((miniblock_t *)x->data)->perm == 0) {
				printf("Miniblock %d:", j + 1);
				printf("\t\t0x%lX\t\t-\t\t", sm);
				printf("0x%lX\t\t| ---\n", sm + ((miniblock_t *)x->data)->size);
			}
			if (((miniblock_t *)x->data)->perm == 6) {
				printf("Miniblock %d:", j + 1);
				printf("\t\t0x%lX\t\t-\t\t", sm);
				printf("0x%lX\t\t| RW-\n", sm + ((miniblock_t *)x->data)->size);
			}
			if (((miniblock_t *)x->data)->perm == 7) {
				printf("Miniblock %d:", j + 1);
				printf("\t\t0x%lX\t\t-\t\t", sm);
				printf("0x%lX\t\t| RWX\n", sm + ((miniblock_t *)x->data)->size);
			}
			x = x->next;
		}
		printf("Block %d end\n", i + 1);
		a = a->next;
	}
}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	node *a = arena->alloc_list->head;
	node *aux;
	int ok = 1;
	while (a) {
		aux = (((block_t *)a->data)->miniblock_list)->head;
		while (aux) {
			if (((miniblock_t *)aux->data)->start_address == address) {
				ok = 0;
				break;
			}
			aux = aux->next;
		}
		if (ok == 0)
			break;
		a = a->next;
	}
	if (ok == 1) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	((miniblock_t *)aux->data)->perm = *permission;
}