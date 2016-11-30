#include "common_impl.h"

int creer_socket(int prop, int *port_num) 
{
   int fd = 0;
   
   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   /* renvoie le numero de descripteur */
   /* et modifie le parametre port_num */
   
   return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */


char **set_data_from_file(char* path, char** machine,int *num){

	int i=0;
	FILE* fp;
	char buffer[SIZE_MSG];
	memset(buffer,0,SIZE_MSG);

	fp = fopen(path, "r");

	while(fgets(buffer, SIZE_MSG,fp)) {
		(*num)++;
	}

//fseek(fp, 0, SEEK_SET);
	rewind(fp);
	machine = malloc(*num * sizeof(char*));

	while(fgets(buffer, SIZE_MSG, fp)) {
		*(machine + i) = malloc (SIZE_MSG * sizeof(char));
		sprintf(*(machine + i), "%s", buffer);
		i++;
	}

	fclose(fp);
	return machine;

}
