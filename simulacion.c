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
    int grabardatos=1;
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
                //printf("Comando escrito: %s\n", comando);
    	} while (ComprobarComando(comando, orden, argumento1, argumento2) == 0);


    if (strcmp(orden, "dir") == 0) {
        Directorio(directorio, &ext_blq_inodos);
        continue;
    }

    if (strcmp(orden, "bytemaps") == 0) {
        printf("El comando es bytemaps");
         Printbytemaps(&ext_bytemaps);
	 continue;
   }

    if (strcmp(orden, "copy") == 0) {
          Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent);
       continue;
    }
	
    if (strcmp(orden, "info") == 0) {
        LeeSuperBloque(&ext_superblock);
        continue;
     } 
 

    if (strcmp(orden, "rename") == 0) {
        Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
       continue;
    }

    if (strcmp(orden, "remove") == 0) {
        Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);
        continue;
    }
    /*
	  // Escritura de metadatos en comandos rename, remove, copy     
    Grabarinodosydirectorio(&directorio, &ext_blq_inodos, fent);
    GrabarByteMaps(&ext_bytemaps, fent);
    GrabarSuperBloque(&ext_superblock, fent);

         if (grabardatos)
                 GrabarDatos(memdatos, fent);
         grabardatos = 0;

	*/	

	if (strcmp(orden, "imprimir") == 0) {
        Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
        continue;
     }




    // Si el comando es "salir", escribir los datos y cerrar
    		if (strcmp(orden, "salir") == 0) {
        		GrabarDatos(memdatos, fent);
       			fclose(fent);
        		return 0;
    		}
	}
}

int ComprobarComando(char* strcomando, char* orden, char* argumento1, char* argumento2) {
    int esComandoValido = 0;

    // Separar comando y argumentos
    int numArgs = sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);
	//printf("Orden: %s, Argumento1: %s, argumento2: %s Numero de Argumentos: %d \n", orden, argumento1,argumento2, numArgs);
    // Validar cada comando según el número de argumentos esperado

    if(numArgs==1){
	argumento1="/0";
	argumento2="/0";
    }
    if (strcmp(orden, "info") == 0) {
        if (numArgs == 1) {
            esComandoValido = 1;
        } else {
            printf("ERROR: El comando '%s' no necesita argumentos.\n", orden);
        }
    } else if((strcmp(orden, "dir") == 0 ) || (strcmp(orden, "bytemaps") == 0)) {
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
        printf("ERROR: Comando ilegal[bytemaps,copy,dir,info,imprimir,rename,remove,salir]\n");
    }

    return esComandoValido;
}


void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps)
{
    printf("Bytemap de bloques:\n");
    for (int i = 0; i < 25; i++)
    {
        printf("%d", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\nBytemap de inodos:\n");
    for (int i = 0; i < MAX_INODOS; i++)
    {
        printf("%d", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup)
{
    printf("Inodos: %u\n", psup->s_inodes_count);
    printf("Bloques: %u\n", psup->s_blocks_count);
    printf("Bloques libres: %u\n", psup->s_free_blocks_count);
    printf("Inodos libres: %u\n", psup->s_free_inodes_count);
    printf("Primer bloque de datos: %u\n", psup->s_first_data_block);
    printf("Tamaño del bloque: %u\n", psup->s_block_size);
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
    // Recorrer todas las entradas del directorio
	int buscar = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        // Comparar el nombre del archivo buscado con el de la entrada actual
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
           buscar = i;
        }
    }
    // Si no se encuentra el archivo, retornar -1
    return buscar;
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

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo)
{
    for (int i = 0; i < MAX_FICHEROS; i++)
    {
        if (strcmp(directorio[i].dir_nfich, nombrenuevo) == 0)
        {
            return -1; // Nuevo nombre ya existe
        }
    }
    for (int i = 0; i < MAX_FICHEROS; i++)
    {
        if (strcmp(directorio[i].dir_nfich, nombreantiguo) == 0)
        {
            strncpy(directorio[i].dir_nfich, nombrenuevo, LEN_NFICH - 1);
            directorio[i].dir_nfich[LEN_NFICH - 1] = '\0';
            return 0;
        }
    }
    return -1; // Nombre antiguo no encontrado
}


// Imprime el contenido de un archivo
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    int indice = BuscaFich(directorio, inodos, nombre);
    if (indice != -1) {
        printf("Contenido de %s:\n", nombre);
        for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
            if(inodos->blq_inodos[directorio[indice].dir_inodo].i_nbloque[i] != NULL_BLOQUE){
                printf("%s", memdatos[inodos->blq_inodos[directorio[indice].dir_inodo].i_nbloque[i] - PRIM_BLOQUE_DATOS].dato);
            }
        }
    }
    else{
        printf("ERROR: Archivo no encontrado.");
    }
    printf("\n");
    return 0;
}




int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich)
{
    EXT_ENTRADA_DIR *entrada = BuscaFich(directorio, inodos, nombre);
    if (entrada == NULL) // Verificamos si el archivo fue encontrado
    {
        printf("Error: Archivo no encontrado.\n");
        return -1;
    }

    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[entrada->dir_inodo];
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO && inodo->i_nbloque[i] != NULL_BLOQUE; i++)
    {
        ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0; // Liberar el bloque
        inodo->i_nbloque[i] = NULL_BLOQUE;
    }

    inodo->size_fichero = 0;
    ext_bytemaps->bmap_inodos[entrada->dir_inodo] = 0; // Liberar el inodo

    // Limpiar la entrada de directorio
    memset(entrada->dir_nfich, 0, LEN_NFICH);
    entrada->dir_inodo = NULL_INODO;

    ext_superblock->s_free_blocks_count++;
    ext_superblock->s_free_inodes_count++;
    return 0;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich)
{
    int inodo_libre = -1;
    int indice_origen = BuscaFich(directorio, inodos, nombreorigen); 
    if (indice_origen == -1)
    {
        printf("Error: Archivo origen no encontrado.\n");
        return -1;
    }

    // Verificar si el archivo destino ya existe
    if (BuscaFich(directorio, inodos, nombredestino) == -1)
    {
        // Si no existe, asignar un inodo libre
        for (int j = 0; j < MAX_INODOS; j++)
        {
            if (ext_bytemaps->bmap_inodos[j] == 0)
            {
                inodo_libre = j;
                ext_bytemaps->bmap_inodos[j] = 1; // Marcar inodo como ocupado
                ext_superblock->s_free_inodes_count--;
                break;
            }
        }

        if (inodo_libre == -1)
        {
            printf("Error: No hay inodos libres disponibles.\n");
            return -1;
        }

        // Copiar datos desde el origen al destino
        EXT_SIMPLE_INODE *inodo_origen = &inodos->blq_inodos[directorio[indice_origen].dir_inodo];
        EXT_SIMPLE_INODE *inodo_destino = &inodos->blq_inodos[inodo_libre];
        memcpy(inodo_destino, inodo_origen, sizeof(EXT_SIMPLE_INODE));

        for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++)
        {
            // Verificar si el bloque en el inodo de origen es válido
            if (inodo_origen->i_nbloque[i] != NULL_BLOQUE && 
                inodo_origen->i_nbloque[i] >= PRIM_BLOQUE_DATOS &&
                inodo_origen->i_nbloque[i] < PRIM_BLOQUE_DATOS + MAX_BLOQUES_DATOS)
            {
                int bloque_asignado = -1;
                for (int j = 0; j < MAX_BLOQUES_DATOS; j++)
                {
                    if (ext_bytemaps->bmap_bloques[j] == 0)
                    {
                        ext_bytemaps->bmap_bloques[j] = 1; // Marcar bloque como ocupado
                        bloque_asignado = j;
                        inodo_destino->i_nbloque[i] = j + PRIM_BLOQUE_DATOS;
                        ext_superblock->s_free_blocks_count--;
                        memcpy(memdatos[j].dato, memdatos[inodo_origen->i_nbloque[i] - PRIM_BLOQUE_DATOS].dato, SIZE_BLOQUE);
                        break;
                    }
                }

                if (bloque_asignado == -1)
                {
                    printf("Error: No hay bloques libres disponibles para copiar el archivo.\n");
                    return -1;
                }
            }
            else
            {
                // Si el bloque del inodo de origen no es válido, detener el proceso
                inodo_destino->i_nbloque[i] = NULL_BLOQUE;
            }
        }

        // Actualizar el directorio con el nuevo archivo
        for (int i = 0; i < MAX_FICHEROS; i++) 
        {
            if (directorio[i].dir_inodo == NULL_INODO) 
            {
                directorio[i].dir_inodo = inodo_libre;
                strncpy(directorio[i].dir_nfich, nombredestino, LEN_NFICH);
                break;
            }
        }
    }
    else
    {
        printf("El fichero %s ya existe. No se puede sobrescribir.\n", nombredestino);
    }
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
