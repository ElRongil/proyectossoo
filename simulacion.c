#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabecera.h"

#define LONGITUD_COMANDO 100


int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre);

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main(int argc , char** argv) {
	char comando[LONGITUD_COMANDO];
	char orden[LONGITUD_COMANDO];
	char argumento1[LONGITUD_COMANDO];
	char argumento2[LONGITUD_COMANDO];
	 
	int i,j;
	unsigned long int m;
	EXT_SIMPLE_SUPERBLOCK ext_superblock;
	EXT_BYTE_MAPS ext_bytemaps;
	EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int entradadir;
    int grabardatos;
    FILE *fent;
	
    // Lectura del fichero completo de una sola vez
    fent = fopen("particion.bin","r+b");
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);     
    memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
    // Bucle de tratamiento de comandos
	for (;;) {
    	do {
        	printf(">> ");
        	fflush(stdin);
        	fgets(comando, LONGITUD_COMANDO, stdin);

        // Eliminar el salto de línea (\n) al final de la entrada
        	comando[strcspn(comando, "\n")] = '\0';

    	} while (ComprobarComando(comando, orden, argumento1, argumento2) == 0);

    if (strcmp(orden, "dir") == 0) {
        Directorio(&directorio, &ext_blq_inodos);
        continue;
    }

    // Escritura de metadatos en comandos rename, remove, copy     
    Grabarinodosydirectorio(&directorio, &ext_blq_inodos, fent);
    GrabarByteMaps(&ext_bytemaps, fent);
    GrabarSuperBloque(&ext_superblock, fent);

   	 if (grabardatos)
       		 GrabarDatos(&memdatos, fent);
   	 grabardatos = 0;

    // Si el comando es "salir", escribir los datos y cerrar
    		if (strcmp(orden, "salir") == 0) {
        		GrabarDatos(&memdatos, fent);
       			fclose(fent);
        		return 0;
    		}
	}
}

int ComprobarComando(char* strcomando, char* orden, char* argumento1, char* argumento2) {
    int esComandoValido = 0;

    // Eliminar salto de línea al final de strcomando
    strcomando[strcspn(strcomando, "\n")] = '\0';

    // Separar comando y argumentos
    int numArgs = sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);

    // Validar cada comando según el número de argumentos esperado
    if (strcmp(orden, "info") == 0) {
        if (numArgs == 1) {
            esComandoValido = 1;
        } else {
            printf("ERROR: El comando '%s' no necesita argumentos.\n", orden);
        }
    } else if (strcmp(orden, "dir") == 0) {
        if (numArgs == 1) {
            esComandoValido = 1;
        } else {
            printf("ERROR: El comando '%s' no necesita argumentos.\n", orden);
        }
    } else if (strcmp(orden, "rename") == 0) {
        if (numArgs == 3) {
            esComandoValido = 1;
        } else if (numArgs > 3) {
            printf("ERROR: El comando '%s' tiene demasiados argumentos.\n", orden);
        } else {
            printf("ERROR: El comando '%s' necesita dos argumentos.\n", orden);
        }
    } else if (strcmp(orden, "imprimir") == 0 || strcmp(orden, "remove") == 0) {
        if (numArgs == 2) {
            esComandoValido = 1;
        } else if (numArgs > 2) {
            printf("ERROR: El comando '%s' tiene demasiados argumentos.\n", orden);
        } else {
            printf("ERROR: El comando '%s' necesita un argumento.\n", orden);
        }
    } else if (strcmp(orden, "copy") == 0) {
        if (numArgs == 3) {
            esComandoValido = 1;
        } else if (numArgs > 3) {
            printf("ERROR: El comando '%s' tiene demasiados argumentos.\n", orden);
        } else {
            printf("ERROR: El comando '%s' necesita dos argumentos.\n", orden);
        }
    } else if (strcmp(orden, "salir") == 0) {
        if (numArgs == 1) {
            esComandoValido = 1;
        } else {
            printf("ERROR: El comando '%s' no necesita argumentos.\n", orden);
        }
    } else {
        printf("ERROR: Comando desconocido '%s'.\n", orden);
    }

    return esComandoValido;
}


void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Bytemap de bloques:\n");
    for (int i = 0; i < 25; i++) {
        printf("%u ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\nBytemap de inodos:\n");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%u ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {
    printf("Nº total de inodos: %u\n", psup->s_inodes_count);
    printf("Nº total de bloques: %u\n", psup->s_blocks_count);
    printf("Nº de bloques libres: %u\n", psup->s_free_blocks_count);
    printf("Nº de inodos libres: %u\n", psup->s_free_inodes_count);
    printf("Primer bloque de datos: %u\n", psup->s_first_data_block);
    printf("Tamaño de bloque: %u bytes\n", psup->s_block_size);
}


int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
    // Recorrer todas las entradas del directorio
	EXT_SIMPLE_INODE **inodo_resultado=NULL;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        // Verificar si la entrada está vacía
        if (directorio[i].dir_inodo == NULL_INODO) {
            continue; // Saltar entradas vacías
        }
        // Comparar el nombre del archivo buscado con el de la entrada actual
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            // Verificar que el número de inodo sea válido
            if (directorio[i].dir_inodo < MAX_INODOS) {
                *inodo_resultado = &inodos->blq_inodos[directorio[i].dir_inodo];
                return i; // Archivo encontrado, retornar índice
            } else {
                printf("Inodo inválido en la entrada del directorio.\n");
                return -2; // Código de error para inodo inválido
            }
        }
    }
    // Si no se encuentra el archivo, retornar -1
    return -1;
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    for (int i = 1; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            EXT_SIMPLE_INODE *inode = &inodos->blq_inodos[directorio[i].dir_inodo];
            printf("Nombre: %s, Tamaño: %u, Inodo: %u, Bloques: ",
                   directorio[i].dir_nfich, inode->size_fichero, directorio[i].dir_inodo);
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inode->i_nbloque[j] != NULL_BLOQUE) {
                    printf("%u ", inode->i_nbloque[j]);
                }
            }
            printf("\n");
        }
    }
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {
    // Buscar la posición del fichero a renombrar
    int posFichero = BuscaFich(directorio, inodos, nombreantiguo);
    if (posFichero == -1) {
        printf("ERROR: No se ha encontrado el fichero %s\n", nombreantiguo);
        return -1; // Error: archivo no encontrado
    }

    // Verificar si el nuevo nombre ya existe
    if (BuscaFich(directorio, inodos, nombrenuevo) != -1) {
        printf("ERROR: El fichero %s ya existe\n", nombrenuevo);
        return -1; // Error: el nuevo nombre ya está en uso
    }

    // Verificar que el nuevo nombre no exceda la longitud máxima
    if (strlen(nombrenuevo) >= LEN_NFICH) {
        printf("ERROR: El nuevo nombre es demasiado largo (máximo %d caracteres)\n", LEN_NFICH - 1);
        return -1; // Error: nombre demasiado largo
    }

    // Renombrar el fichero
    strncpy(directorio[posFichero].dir_nfich, nombrenuevo, LEN_NFICH - 1);
    directorio[posFichero].dir_nfich[LEN_NFICH - 1] = '\0'; // Asegurar terminación nula

    printf("Fichero %s renombrado a %s con éxito\n", nombreantiguo, nombrenuevo);
    return 0; // Éxito
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre) {
    // 1. Buscar el archivo en el directorio
    int posFichero = BuscaFich(directorio, inodos, nombre);
    if (posFichero == -1) {
        printf("ERROR: El fichero %s no existe\n", nombre);
        return -1;
    }

    // 2. Obtener el inodo correspondiente
    int numInodo = directorio[posFichero].dir_inodo;
    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[numInodo];

    if (inodo->size_fichero == 0) {
        printf("El fichero %s está vacío\n", nombre);
        return 0;
    }

    // 3. Recorrer los bloques y mostrar el contenido
    printf("Contenido del fichero %s:\n", nombre);

    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo->i_nbloque[i] != NULL_BLOQUE) {
            unsigned char *bloque = memdatos[inodo->i_nbloque[i]].dato;
            printf("%.*s", SIZE_BLOQUE, bloque);
        }
    }
    printf("\n");
    return 0;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre, FILE *fich) {
    // 1. Buscar el archivo en el directorio
    int posFichero = BuscaFich(directorio, inodos, nombre);
    if (posFichero == -1) {
        printf("ERROR: El fichero '%s' no existe.\n", nombre);
        return -1;
    }

    // 2. Obtener el inodo asociado
    int numInodo = directorio[posFichero].dir_inodo;
    if (numInodo == NULL_INODO || numInodo >= MAX_INODOS) {
        printf("ERROR: Inodo inválido para el fichero '%s'.\n", nombre);
        return -1;
    }
    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[numInodo];

    // 3. Liberar los bloques de datos
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo->i_nbloque[i] != NULL_BLOQUE) {
            // Liberar bloque en el bytemap
            ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0;
            ext_superblock->s_free_blocks_count++;
            inodo->i_nbloque[i] = NULL_BLOQUE; // Resetear puntero a bloque
        }
    }

    // 4. Liberar el inodo
    inodo->size_fichero = 0;
    ext_bytemaps->bmap_inodos[numInodo] = 0; // Marcar inodo como libre
    ext_superblock->s_free_inodes_count++;

    // 5. Eliminar la entrada del directorio
    memset(directorio[posFichero].dir_nfich, 0, LEN_NFICH); // Limpiar nombre
    directorio[posFichero].dir_inodo = NULL_INODO;

    // 6. Actualizar los cambios en el archivo binario
    Grabarinodosydirectorio(directorio, inodos, fich);
    GrabarByteMaps(ext_bytemaps, fich);
    GrabarSuperBloque(ext_superblock, fich);

    printf("El fichero '%s' ha sido eliminado con éxito.\n", nombre);
    return 0;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {
    // 1. Buscar el archivo origen
    int posOrigen = BuscaFich(directorio, inodos, nombreorigen);
    if (posOrigen == -1) {
        printf("ERROR: El archivo origen '%s' no existe.\n", nombreorigen);
        return -1;
    }

    // 2. Verificar si el archivo destino ya existe
    if (BuscaFich(directorio, inodos, nombredestino) != -1) {
        printf("ERROR: El archivo destino '%s' ya existe.\n", nombredestino);
        return -1;
    }

    // 3. Buscar un inodo libre
    int nuevoInodo = -1;
    for (int i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) { // Inodo libre
            nuevoInodo = i;
            ext_bytemaps->bmap_inodos[i] = 1; // Marcar como ocupado
            ext_superblock->s_free_inodes_count--;
            break;
        }
    }
    if (nuevoInodo == -1) {
        printf("ERROR: No hay inodos libres disponibles.\n");
        return -1;
    }

    // 4. Crear una nueva entrada en el directorio
    int nuevaEntrada = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) { // Entrada libre
            nuevaEntrada = i;
            strncpy(directorio[i].dir_nfich, nombredestino, LEN_NFICH - 1);
            directorio[i].dir_nfich[LEN_NFICH - 1] = '\0';
            directorio[i].dir_inodo = nuevoInodo;
            break;
        }
    }
    if (nuevaEntrada == -1) {
        printf("ERROR: No hay espacio en el directorio.\n");
        return -1;
    }

    // 5. Asignar bloques libres y copiar datos
    EXT_SIMPLE_INODE *inodoOrigen = &inodos->blq_inodos[directorio[posOrigen].dir_inodo];
    EXT_SIMPLE_INODE *inodoDestino = &inodos->blq_inodos[nuevoInodo];
    inodoDestino->size_fichero = inodoOrigen->size_fichero;

    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodoOrigen->i_nbloque[i] != NULL_BLOQUE) {
            // Buscar un bloque libre
            int nuevoBloque = -1;
            for (int j = PRIM_BLOQUE_DATOS; j < MAX_BLOQUES_PARTICION; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) { // Bloque libre
                    nuevoBloque = j;
                    ext_bytemaps->bmap_bloques[j] = 1; // Marcar como ocupado
                    ext_superblock->s_free_blocks_count--;
                    break;
                }
            }
            if (nuevoBloque == -1) {
                printf("ERROR: No hay bloques libres disponibles.\n");
                return -1;
            }

            // Asignar el bloque y copiar datos
            inodoDestino->i_nbloque[i] = nuevoBloque;
            memcpy(memdatos[nuevoBloque].dato, memdatos[inodoOrigen->i_nbloque[i]].dato, SIZE_BLOQUE);
        } else {
            inodoDestino->i_nbloque[i] = NULL_BLOQUE;
        }
    }

    // 6. Escribir los cambios en el archivo binario
    Grabarinodosydirectorio(directorio, inodos, fich);
    GrabarByteMaps(ext_bytemaps, fich);
    GrabarSuperBloque(ext_superblock, fich);

    printf("El archivo '%s' ha sido copiado a '%s' con éxito.\n", nombreorigen, nombredestino);
    return 0;
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    // Posicionar el puntero en el bloque 2 (inodos)
    fseek(fich, 2 * SIZE_BLOQUE, SEEK_SET);

    // Escribir los inodos en el archivo
    if (fwrite(inodos, SIZE_BLOQUE, 1, fich) != 1) {
        printf("ERROR: No se pudo escribir la lista de inodos en el archivo.\n");
        return;
    }

    // Posicionar el puntero en el bloque 3 (directorio)
    fseek(fich, 3 * SIZE_BLOQUE, SEEK_SET);

    // Escribir el directorio en el archivo
    if (fwrite(directorio, SIZE_BLOQUE, 1, fich) != 1) {
        printf("ERROR: No se pudo escribir el directorio en el archivo.\n");
        return;
    }

    // Asegurar que los datos se escriben en el disco
    fflush(fich);

    printf("Los inodos y el directorio han sido grabados correctamente.\n");
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    // Posicionar el puntero en el inicio del bloque 1 (bytemaps)
    fseek(fich, SIZE_BLOQUE, SEEK_SET);

    // Escribir los bytemaps en el archivo
    if (fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich) != 1) {
        printf("ERROR: No se pudo escribir los bytemaps en el archivo.\n");
        return;
    }

    // Asegurar que los datos se escriben en el disco
    fflush(fich);

    printf("Los bytemaps han sido grabados correctamente.\n");
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    fseek(fich, 0, SEEK_SET);
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    // Posicionar el puntero del archivo en el primer bloque de datos
    fseek(fich, PRIM_BLOQUE_DATOS * SIZE_BLOQUE, SEEK_SET);

    // Escribir los bloques de datos al archivo
    for (int i = 0; i < MAX_BLOQUES_DATOS; i++) {
        fwrite(&memdatos[i], SIZE_BLOQUE, 1, fich);
    }

    // Asegurar que los datos se escriben en disco
    fflush(fich);

    printf("Los bloques de datos han sido grabados correctamente en la partición.\n");
}
