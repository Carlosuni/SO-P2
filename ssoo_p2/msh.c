/*-
 * msh.c
 * Minishell C source
 * Show how to use "obtain_order" input interface function
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
				perror("Error al abrir el fichero de entrada!\n");			
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
				perror("Error al crear el fichero de salida!\n");			
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
				perror("Error al crear el fichero de errores!\n");			
				continue;
			}
		}
																		
		/* Creamos tantos procesos como comandos a ejecutar */
		for (i = 0; i < argvc; i++)
		{	
			/*COMPLEMENTARIO*/
			/* Pipe preparado para un número indeterminado e ilimitado de mandatos (pares e impares) */								
			/* Creamos el PIPE correspondiente... si procede */	
			/* Si somos un proceso par y quedan mas procesos por crear */
			if ((i < (argvc - 1)) && (i % 2 == 0))
			{
				while (pipe(pipe1) == -1)
				{
					perror("Error al crear el pipe1\n");
				}
			}			
			/* Si somos un proceso impar y quedan mas procesos por crear */
			if ((i < (argvc - 1)) && (i % 2 == 1))
			{
				while (pipe(pipe2) == -1)
				{
					perror("Error al crear el pipe2\n");
				}
			}								

			/* Comprobamos si tenemos que ejecutar un MANDATO INTERNO */
		  /* Si es mytime, no es en BG y es el último */
			if (((strcmp(argvv[i][0], "mytime") == 0) && (!bg) && (argvc > 1) && (i == argvc - 1)) ||
					((argvc == 1) && (strcmp(argvv[i][0], "mytime" )== 0) && (!bg))){						
						
					/* Si hay que direccionar la salida */	
					if (filev[1] != NULL){
						outPadre = dup(STDOUT_FILENO);
						close(STDOUT_FILENO);
						dup(fdOut);
						close(fdOut);						
					}

					/* Ejecutamos el mandato interno mytime*/
					mytime(argvv[i]);

					/* Dejamos la salida como antes */
					if (filev[1] != NULL)
					{
						close(STDOUT_FILENO);
						dup(outPadre);
					}					
					continue;
			}

			/* Si es mypwd, no es en BG y es el último */
			if (((strcmp(argvv[i][0], "mypwd") == 0) && (!bg) && (argvc > 1) && (i == argvc - 1)) ||
					((argvc == 1) && (strcmp(argvv[i][0], "mypwd") == 0) && (!bg))){						
						
					/* Si hay que direccionar la salida */	
					if (filev[1] != NULL){
						outPadre = dup(STDOUT_FILENO);
						close(STDOUT_FILENO);
						dup(fdOut);
						close(fdOut);						
					}

					/* Ejecutamos el mandato interno myowd*/
					mypwd(argvv[i]);

					/* Dejamos la salida como antes... */
					if (filev[1]!= NULL){
						close(STDOUT_FILENO);
						dup(outPadre);
					}				
					continue;
			}
								
			/* Creamos un nuevo proceso */
			pid = fork();
						
			/* Error al crear proceso hijo */
			if (pid == -1)
			{
				perror ("Error al crear proceso hijo!\n");
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
				  /* Esperamos a que el último mandato termine... */
				} else if (i == (argvc - 1))
				{
					while (pid != wait(&status)) 
					{
						continue;
					}
				}			
			} /* Soy proceso padre */						
		} /* For i=0 to numMandatos */	
	}
	exit(0);
	return 0;
}

/* Muestra la duración de un proceso */
/* Da error en el corrector a pesar de estar bien
	 y dar la salida esperada */
int mytime(char **mandato)
{	
	/* Datos para intepretar las palabras del mandato */
	int n_palabras = 0;
	char *argumentos[7];
	int mandato_size = sizeof(mandato);

	/* Recorremos todas las palabras del mandato */
	for (int j = 0; j < mandato_size; ++j)
	{
		/* Guardamos las palabras del submandato (dentro de mytime) */
		char *palabra = mandato[j];
		argumentos[j - 1] = palabra;

		/* Si es NULL, deja de contar y almacenar las palabras del mandato */
		if (palabra == NULL)
		{
			break;
		} 
		++n_palabras;	
	}

	/* Recogida de tiempo inicial */
	struct timeval tv_inicio, tv_fin;
	gettimeofday(&tv_inicio, NULL);
	double t_inicio = tv_inicio.tv_usec;
	long seconds_start = tv_inicio.tv_sec;

	/* Creamos un nuevo proceso... */
	int pid = fork(); 
	int status;
	int tpid = 0;

	/* Creamos proceso hijo para ejecutar submandato con execvp */
	switch (pid)
	{
		case -1: /* error */
			perror("Error en el fork");
			return -1;

		case 0: /* hijo */
			//sleep(2);
			if (execvp(argumentos[0], argumentos) < 0)
			{
				perror("Error ejecutando el mandato\n");
				return -1;
			}
			break;

		default: /* padre */
			do {
       	tpid = wait(&status);

				if(tpid == -1)
				{
					perror("Usage: mytime <command <args>>");
					return -1;	
				} else if(tpid == pid)
				{
					/* Obtención del tiempo de ejecucíon */
					gettimeofday(&tv_fin, NULL);
					double t_fin = tv_fin.tv_usec;
					long seconds_end = tv_fin.tv_sec;
					long seconds = seconds_end - seconds_start;
					long micros = ((seconds * 1000000) + t_fin) - (t_inicio);
					double t_ejec = ((double) micros) / 1000000;
					printf("​Time spent: %f secs.\n​", t_ejec);
					break;
				}   		
			} while(tpid != pid);
	}
	return 0;
}

/* Devuelve el directorio actual */
/* Da error en el corrector a pesar de estar bien
	 y dar la salida esperada */
int mypwd(char **mandato)
{
	int n_palabras = 0;
	int mandato_size = sizeof(mandato);

	/* Recorremos todas las palabras del mandato */
	for (int j = 0; j < mandato_size; ++j)
	{
		char *palabra = mandato[j];

		/* Si es NULL, deja de contar y almacenar las palabras del mandato */
		if (palabra == NULL)
		{
			break;
		} else if (j > 0)
		{
			perror("Mypwd error\n");
			return -1;
		}
		++n_palabras;	
	}

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