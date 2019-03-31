/*-
 * msh.c
 *
 * Minishell C source
 * Show how to use "obtain_order" input interface function
 *
 * THIS FILE IS TO BE MODIFIED
 */

#include <stddef.h>			/* NULL */
#include <stdio.h>			/* setbuf, printf */
#include <stdlib.h>			
#include <unistd.h>		
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>


extern int obtain_order();		/* See parser.y for description */
int mytime(char **mandato);
int mypwd(char **mandato);


int main(void)
{
	char ***argvv;
	int argvc;
	//char **argv;
	//int argc;
	char *filev[3];
	int bg;
	int ret;

	/* Nuevas variables */
	int pid;		/* ID del proceso creado */
	int i;			/* Auxiliar */
	int fdIn;		/* Descriptor de fichero ENTRADA */
	int fdOut;		/* Descriptor de fichero SALIDA */
	int fdError;	/* Descriptor de fichero ERROR */
	int pipe1[2];	/* Primer pipe */
	int pipe2[2];   /* Segundo pipe */
	int status;		/* Estado del wait */
	int outPadre;	/* Redireccionar salida del padre */

	/* Limpiamos los buffers... */
	setbuf(stdout, NULL);	
	setbuf(stdin, NULL);	
			
	/* Hasta el infinito o hasta que apretemos Ctrl + C */
	while (1)
	{		
		/* Prompt */
		fprintf(stderr, "%s", "msh> ");	
		
		/* Parseamos la entrada... */
		ret = obtain_order(&argvv, filev, &bg);

		/*size_t n_mandatos = sizeof(argvv)/sizeof(argvv[0]);
		printf("\nn_mandatos = %lu\n", n_mandatos);
		printf("Primer primer flag de primer mandato = %s\n", argvv[0][1]);*/


		//size_t size_caracter = sizeof(argvv[0][0][0]);
		//printf("Tamaño de caracter = %lu\n", size_caracter);

		//size_t size_palabra = sizeof(argvv[0][0])/size_caracter;
		//printf("Tamaño de palabra = %lu\n", size_palabra);

		//size_t size_mandato = sizeof(argvv[0])/size_palabra;
		//printf("Tamaño de mandato = %lu\n", size_mandato);

		//size_t size_entrada = sizeof(argvv)/size_mandato;
		//printf("Tamaño de entrada = %lu\n", size_entrada);
		//printf("Tamaño de entrada predefinida = %d\n", argvc);


		//printf("Tamaño de todo = %lu\n", sizeof(argvv));
		//printf("Tamaño de mandato = %lu\n", sizeof(argvv[0]));
		//printf("Tamaño de palabra = %lu\n", sizeof(argvv[0][1]));
		
		/*size_t n_palabras = sizeof(argvv[0])/sizeof(argvv[0][0]);
		printf("\n_palabras = %lu\n", n_palabras);
		printf("Primer mandato = %s\n", argvv[0][1]);*/

		/* Tratamiento de las SEÑALES */
		/* En background las señales se ignoran... */
		if (bg)
		{
 			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
		} else
		{
			/* En foreground se tiene el comportamiento por defecto... */
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
		}			
		
		/* EOF */
		if (ret == 0)
		{ 
		  break;		
		}
		  		  
		/* Syntax error */
		if (ret == -1) 
		{
		  continue;	
		}
		  
		/* Número de comandos en la entrada */
		argvc = ret - 1;		
		
		/* Línea vacía */
		if (argvc == 0) 
		{
		  continue;	
		}
		  		  
		/* Comprobamos si tenemos que abrir el fichero de entrada */  
		if (filev[0] != NULL)
		{	
			fdIn = open(filev[0], O_RDONLY);			
			
			/* Si se produce un error al abrir el fichero de entrada */
			if (fdIn < 0)
			{
				printf("Error al abrir el fichero de entrada!\n");			
				continue;
			}
		}
		
		/* Comprobamos si tenemos que abrir el fichero de salida */  
		if (filev[1] != NULL)
		{
			fdOut = creat(filev[1], 0666);
			
			/* Si se produce un error al abrir el fichero de salida */
			if (fdOut < 0)
			{
				printf("Error al crear el fichero de salida!\n");			
				continue;
			}
		}
		
		/* Comprobamos si tenemos que abrir el fichero de errores */  
		if (filev[2] != NULL)
		{		
			fdError = creat(filev[2], 0666);
			
			/* Si se produce un error al abrir el fichero de errores */
			if (fdError < 0)
			{
				printf("Error al crear el fichero de errores!\n");			
				continue;
			}
		}
																		
		/* Creamos tantos procesos como comandos a ejecutar */
		for (i = 0; i < argvc; i++)
		{						
			/* Creamos el PIPE correspondiente... si procede */	/* TODO */										
			/* Si somos un proceso par y quedan mas procesos por crear */
			if ((i < (argvc - 1)) && (i % 2 == 0))
			{
				while (pipe(pipe1) == -1)
				{
					printf("Error al crear el pipe1\n");
				}
			}			
			/* Si somos un proceso impar y quedan mas procesos por crear */
			if ((i < (argvc - 1)) && (i % 2 == 1))
			{
				while (pipe(pipe2) == -1)
				{
					printf("Error al crear el pipe2\n");
				}
			}								

			/* Comprobamos si tenemos que ejecutar un MANDATO INTERNO */

			/* Si es mytime, no es en BG y es el último */
			if ((strcmp(argvv[i][0], "mytime") == 0)){
				if (((!bg) && (argvc == 2) && (i == argvc - 1)) ||
						((argvc == 1) && (strcmp(argvv[i][0], "mytime") == 0) && (!bg)))
				{								
						/* Si hay que direccionar la salida */	
						if (filev[1] != NULL)
						{
							outPadre = dup(STDOUT_FILENO);
							close(STDOUT_FILENO);
							dup(fdOut);
							close(fdOut);						
						}

						/* Ejecutamos mytime */
						if (mytime(argvv[i]) == -1)
						{
							continue;
						}
						

						/* Dejamos la salida como antes... */
						if (filev[1] != NULL)
						{
							close(STDOUT_FILENO);
							dup(outPadre);
						}
						
						continue;
				} else
				{
					printf("Con mytime sólo se puede introducir un mandato");
					continue;
				}
				
			}

		   /* Si es mypwd, no es en BG y es el �ltimo */
			if (((strcmp(argvv[i][0], "mypwd") == 0) && (!bg) && (argvc > 1) && (i == argvc - 1)) ||
					((argvc == 1) && (strcmp(argvv[i][0], "mypwd") == 0) && (!bg)))
			{								
					/* Si hay que direccionar la salida */	
					if (filev[1] != NULL)
					{
						outPadre = dup(STDOUT_FILENO);
						close(STDOUT_FILENO);
						dup(fdOut);
						close(fdOut);						
					}

					/* Ejecutamos... */
					mypwd(argvv[i]);

					/* Dejamos la salida como antes... */
					if (filev[1] != NULL)
					{
						close(STDOUT_FILENO);
						dup(outPadre);
					}
					
					continue;
			}
								
			/* Creamos un nuevo proceso... */
			pid = fork();
						
			/* Error al crear proceso hijo */
			if (pid == -1)
			{
				printf ("Error al crear proceso hijo!\n");
				break;
			} else if (pid == 0)
			{
				/* Si soy el proceso hijo */							
				/* PRIMER MANDATO */
				if (i == 0)
				{			
					/* Tiene que leer del fichero de entrada */
					if (filev[0] != NULL)
					{
						close(STDIN_FILENO);	// Cerramos IN estándar
						dup(fdIn);				// Fichero -> IN estándar
						close(fdIn);			// Cerramos fichero
					}					
					
					/* Si es el único mandato */
					if (argvc == 1)
					{
						/* Tiene que leer del fichero de salida */
						if (filev[1] != NULL)
						{					
							close(STDOUT_FILENO);
							dup(fdOut);
							close(fdOut);							
						}
						
						/* Tiene que leer del fichero de error */
						if (filev[2] != NULL)
						{
							close(STDERR_FILENO);
							dup(fdError);
							close(fdError);							
						}
					} else if (argvc > 1)
					{
						/* Al menos hay 2 mandatos, redireccionamos pipes ... */
						close(STDOUT_FILENO);	// Cerramos OUT estándar
						close(pipe1[0]);		// Cerramos pipe1[RD]
						dup(pipe1[1]);			// pipe1[WR] -> OUT estándar
						close(pipe1[1]);		// Cerramos pipe1[WR]						
					}
					/* Primer mandato */	
				} else if ((i == (argvc - 1)) && (i != 0))
				{
					/* ÚLTIMO MANDATO */
					/* Tiene que escribir al fichero de salida */
					if (filev[1] != NULL)
					{					
						close(STDOUT_FILENO);
						dup(fdOut);
						close(fdOut);							
					}
					
					/* Tiene que leer del fichero de error */
					if (filev[2] != NULL)
					{					
						close(STDERR_FILENO);
						dup(fdError);
						close(fdError);							
					}					
					
					/* Comprobamos si hay que redireccionar pipes */
					/* Si somos un proceso impar*/
					if (i%2 == 1)
					{
						close(STDIN_FILENO);	// Cerramos IN est�ndar
						close(pipe1[1]);		// Cerramos pipe2[WR]
						dup(pipe1[0]);			// pipe2[RD] -> IN est�ndar
						close(pipe1[0]);		// Cerramos pipe2[RD]
					} else
					{
						/* Si somos un proceso par */
						close(STDIN_FILENO);	// Cerramos IN est�ndar
						close(pipe2[1]);		// Cerramos pipe1[WR]
						dup(pipe2[0]);			// pipe1[RD] -> IN est�ndar
						close(pipe2[0]);		// Cerramos pipe1[RD]
					}				
					/* Último mandato */
				} else
				{
					/* MANDATO INTERMEDIO */
					/* Es un mandato par */
					if (i%2 == 0)
					{
						/* Redireccionamos la salida del pipe 1 */
						close(STDIN_FILENO);	// Cerramos IN est�ndar
						close(pipe2[1]);		// Cerramos pipe1[WR]
						dup(pipe2[0]);			// pipe1[RD] -> IN est�ndar
						close(pipe2[0]);		// Cerramos pipe1[RD]
						
						/* Redireccionamos la salida del pipe 2 */
						close(STDOUT_FILENO);	// Cerramos OUT est�ndar
						close(pipe1[0]);		// Cerramos pipe2[RD]
						dup(pipe1[1]);			// pipe2[WR] -> OUT est�ndar
						close(pipe1[1]);		// Cerramos pipe2[WR]	
						/* Es un mandato impar*/					
					}else
					{									
						/* Redireccionamos la salida del pipe 1 */
						close(STDIN_FILENO);	// Cerramos IN est�ndar
						close(pipe1[1]);		// Cerramos pipe1[WR]
						dup(pipe1[0]);			// pipe1[RD] -> IN est�ndar
						close(pipe1[0]);		// Cerramos pipe1[RD]
						
						/* Redireccionamos la salida del pipe 2 */
						close(STDOUT_FILENO);	// Cerramos OUT est�ndar
						close(pipe2[0]);		// Cerramos pipe2[RD]
						dup(pipe2[1]);			// pipe2[WR] -> OUT est�ndar
						close(pipe2[1]);		// Cerramos pipe2[WR]						
					}						
				} /* Mandato intermedio */
									
				/* Ejecutamos el comando */
				if ((strcmp(argvv[i][0], "mytime") == 0))
				{
					mytime(argvv[i]);
					exit(0);
				} else if ((strcmp(argvv[i][0], "mypwd") == 0))
				{
					mypwd(argvv[i]);
					exit(0);
		  		} else 
				{							
	 				execvp(argvv[i][0], argvv[i]);						
				}
			/* Soy proceso hijo */	
			} else
			{
				/* Si soy el padre... */
				/* Se cierran los pipes correspondientes... */
				if ((i%2 == 0) && (i != 0))
				{
					close(pipe2[0]);
					close(pipe2[1]);
				} else if (i % 2 == 1)
				{
					close(pipe1[0]);
					close(pipe1[1]);
				}			
						
				/* Hay que ejecutar en background... */
				if (bg)
				{
  				  printf("[%d]\n", pid);
				  /* Esperamos a que el �ltimo mandato termine... */
				} else if (i == (argvc - 1))
				{
					while (pid != wait(&status)) 
					{
						continue;
					}
				}			
			} /* Soy proceso padre */
						
		} /* For i=0 to numMandatos */
			
/*
 * LAS LINEAS QUE A CONTINUACION SE PRESENTAN SON SOLO
 * PARA DAR UNA IDEA DE COMO UTILIZAR LAS ESTRUCTURAS
 * argvv Y filev. ESTAS LINEAS DEBERAN SER ELIMINADAS.
 */
 
//		for (argvc = 0; (argv = argvv[argvc]); argvc++) {
//			for (argc = 0; argv[argc]; argc++)
//				printf("%s ", argv[argc]);
//			printf("\n");
//		}
//		if (filev[0]) printf("< %s\n", filev[0]);/* IN */
//		if (filev[1]) printf("> %s\n", filev[1]);/* OUT */
//		if (filev[2]) printf(">& %s\n", filev[2]);/* ERR */
//		if (bg) printf("&\n");
/*
 * FIN DE LA PARTE A ELIMINAR
 */
	
	}
	exit(0);
	return 0;
}

int mytime(char **mandato)
{	

		/* size_t size_caracter = sizeof(mandato[0][0][0]);
		printf("Tamaño de caracter = %lu\n", size_caracter);

		size_t size_palabra = sizeof(argvv[0][0])/size_caracter;
		printf("Tamaño de palabra = %lu\n", size_palabra);

		size_t size_mandato = sizeof(argvv[0])/size_palabra;
		printf("Tamaño de mandato = %lu\n", size_mandato);

		size_t size_entrada = sizeof(argvv)/size_mandato;
		printf("Tamaño de entrada = %lu\n", size_entrada);
		printf("Tamaño de entrada predefinida = %d\n", argvc); */

	//char *comando = mandato[0];
	//size_t n_palabras = sizeof(mandato)/sizeof(mandato[0]);
	//char **flags[mandato.]
	//printf("\n%lu\n", n_palabras);
	
	//printf("Tamaño mandato = %lu\n", sizeof(mandato));

	int n_palabras = 0;
	//char *comando = NULL;
	//char *flags[6];

	char *argumentos[7];

	int mandato_size = sizeof(mandato);

	/* Recorremos todas las palabras del mandato */
	for (int j = 0; j < mandato_size; ++j)
	{
		//printf("Palabra del mandato = %s\n", mandato[j]);
		/* Si es la segunda, es el comando del proceso hijo */
/* 		if (j == 1)
		{
			comando = mandato[j];
			printf("Guardando '%s' como comando\n", comando);
		}
		 */
		/* De la segunda para adelante,son los flags */
		/* if (j > 1) { */
		char *palabra = mandato[j];

		argumentos[j - 1] = palabra;
		//strcpy(flags[j - 2], mandato[j]);
		//printf("Guardando '%s' como argumento\n", argumentos[j - 1]);

		/* Si es NULL, deja de contar y almacenar las palabras del mandato */
		if (palabra == NULL)
		{
			//printf("Numero de palabras = %d\n", n_palabras);
			//printf("FIN DE MANDATO\n");
			break;
		} 

	/* 	} */

		++n_palabras;	
	}

	struct timeval tv_inicio, tv_fin;
	gettimeofday(&tv_inicio, NULL);
	double t_inicio = tv_inicio.tv_usec;
	//printf("tiempo inicio = %f\n", t_inicio);
	long seconds_start = tv_inicio.tv_sec;

	//time_t start, end;
  //double dif;
	//time(&start);

	//clock_t t; 
    //t = clock(); 

	/* Creamos un nuevo proceso... */
	int pid = fork(); 
	int status;
	int tpid = 0;


	switch(pid)
	{
		case -1: /* error */
			perror("Error en el fork");
			return -1;

		case 0: /* hijo */
			//sleep(2);
			if (execvp(argumentos[0], argumentos) < 0)
			{
				int errnum = errno;
				fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
				//printf("Error en el exec. Si todo ha ido bien esto nunca debería ejecutarse.\nError: %s\n", strerror(errno)); 
				return -1;
			}
			break;

		default: /* padre */
			//printf("Soy el proceso padre\n");
/* 			while (wait(&status) != pid)
			{
				printf("“​Time spent: secs.\n​");
				if (status == 0)
				{
					printf("Ejecución normal del hijo\n");
				} else 
				{
					printf("Ejecución anormal del hijo \n");
					perror("Error en el wait");
					printf("Fin del proceso PADRE\n");
					return -1;
				}
			}
			if (wait(&status) == pid)
			{
				gettimeofday(&tv_fin, NULL);
				double t_fin = tv_fin.tv_usec;
				double t_ejec = t_fin - t_inicio;
				printf("“​Time spent: %f secs.\n​", t_ejec);
			} */
			do {
       	tpid = wait(&status);

				if(tpid == -1)
				{
					//printf("Ejecución anormal del hijo \n");
					//perror("Error en el wait");
					//printf("Fin del proceso PADRE\n");
					printf("Usage: mytime <command <args>>");
					return -1;	
				} else if(tpid == pid)
				{
					gettimeofday(&tv_fin, NULL);
					double t_fin = tv_fin.tv_usec;
					//printf("tiempo final = %f\n", t_fin);
					//double t_ejec = t_fin - t_inicio;
					

					long seconds_end = tv_fin.tv_sec;
					long seconds = seconds_end - seconds_start;
					//long seconds = (end.tv_sec - start.tv_sec);
					long micros = ((seconds * 1000000) + t_fin) - (t_inicio);
					//printf("Time elpased is %d seconds and %d micros\n", seconds, micros);
					double t_ejec = ((double) micros) / 1000000;
					printf("​Time spent: %f secs.\n​", t_ejec);

					//time(&end);
					//dif = difftime(end, start);
					//printf("Your calculations took %f seconds to run.\n", dif );  

					//t = clock() - t; 
		  			//double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
					//printf("fun() took %f seconds to execute \n", time_taken); 

					break;
				}   		
			} while(tpid != pid);

	}
			

/* 	printf("Mandato entero = %s\n", mandato[1]);
	size_t n_chars = sizeof(mandato[0][0]);
	printf("Tamaño chars mandato = %lu\n", n_chars); */
	//printf("llego\n");
	//printf("tamaño mandato = %lu", sizeof(mandato));

	/* char **p_mandato = &mandato; */

	/* for(int j = 0; j < sizeof(mandato); ++j)
	{
		printf("%c", *(p_mandato+j));
		if (mandato[0][j] == '\0'){
			printf("\nEncontrado fin de palabra\n");
		}
	} */


	/*
	FILE *df;
	char linea[MAX];
	char cpu[MAX];
	int FIN=0;


	if ((df=fopen("/proc/cpuinfo", "r"))==NULL){
		printf("Error al abrir el fichero");
		return -1;
	}

	// No hay par�metros... 
	if (mandato[1]==NULL){
		while ((fgets(linea, MAX, df))&&(!FIN))	{
			if (strstr(linea, "model name")!=NULL){
				strcpy(cpu, strstr(linea, ": ")+2);
				printf("%s", cpu );
				FIN=1;
			}
		}
	}
	
	// Sacamos toda la informaci�n 
	else if (strcmp(mandato[1], "-all")==0){
		
		while ((fgets(linea, MAX, df))&&(!FIN)){
			
			if (strstr(linea, "model name")!=NULL){
				strcpy(cpu, strstr(linea, ": ")+2);
				printf(" %s ", cpu );
			}
			
			if (strstr(linea, "vendor_id")!=NULL){
				strcpy(cpu, strstr(linea, ": ")+2);
				printf("de %s ", cpu );
			}
			
			if (strstr(linea, "cpu MHz")!=NULL){
				strcpy(cpu, strstr(linea, ": ")+2);
				printf("a %s ", cpu );
			}
			
			if (strstr(linea, "cache size")!=NULL){
				strcpy(cpu, strstr(linea, ": ")+2);
				printf("%s de cache\n" , cpu );
				FIN=1;
			}
			
			if (strstr(linea, "processor")!=NULL){
				strcpy(cpu, strstr(linea, ": ")+2);
				printf("%s procesador ", cpu );
			}
				
			bzero(linea, MAX);
			bzero(cpu, MAX);
		}
	}
	
	// Par�metro incorrecto
	else{
		printf ("Par�metro incorrecto -> %s\n", mandato[1]);
	}
*/

	return 0;
}

int mypwd(char **mandato)
{

	int n_palabras = 0;
	//char *comando = NULL;
	//char *flags[6];

	int mandato_size = sizeof(mandato);

	/* Recorremos todas las palabras del mandato */
	for (int j = 0; j < mandato_size; ++j)
	{
		//printf("Palabra del mandato = %s\n", mandato[j]);
		char *palabra = mandato[j];
		//strcpy(flags[j - 2], mandato[j]);
		//printf("Guardando '%s' como argumento\n", argumentos[j - 1]);

		/* Si es NULL, deja de contar y almacenar las palabras del mandato */
		if (palabra == NULL)
		{
			//printf("Numero de palabras = %d\n", n_palabras);
			//printf("FIN DE MANDATO\n");
			break;
		} else if (j > 0)
		{
			perror("Mypwd error\n");
			return -1;
		}

		++n_palabras;	
	}


	/* FALLA EN EL CORRECTOR INESPERADAMENTE
		DALA SALIDA ESPERADA*/
	char cwd[256];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
      	perror("getcwd() error");
	  	return -1;
	} else {
     	printf("Current dir: %s\n", cwd);
	}
	

	return 0;
}