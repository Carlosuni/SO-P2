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


extern int obtain_order();		/* See parser.y for description */
int mytime(char **mandato);
int mypwd(char **mandato);


int main(void){
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

	/* Eliminamos los biffer... */
	setbuf(stdout, NULL);	
	setbuf(stdin, NULL);	
			
	/* Hasta el infinito o hasta que apretemos Ctrl + C */
	while (1){	
		
		/* Prompt */
		fprintf(stderr, "%s", "msh> ");	
		
		/* Parseamos la entrada... */
		ret = obtain_order(&argvv, filev, &bg);
		
		/* Tratamiento de las SE�ALES */
		
		/* En background las se�ales se ignoras... */
		if (bg){
 			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
		}
		
		/* En foreground se tiene el comportamiento por defecto... */
		else{
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
		}			
		
		/* EOF */
		if (ret == 0) 
		  break;		
		  		  
		/* Syntax error */
		if (ret == -1) 
		  continue;	
		  
		/* N�mero de comandos en la entrada */
		argvc = ret - 1;		
		
		/* L�nea vac�a */
		if (argvc == 0) 
		  continue;	
		  		  
		/* Comprobamos si tenemos que abrir el fichero de entrada */  
		if (filev[0]!=NULL){
			
			fdIn = open (filev[0], O_RDONLY);			
			
			/* Si se produce un error al abrir el fichero de entrada */
			if (fdIn<0){
				printf ("Error al abrir el fichero de entrada!\n");			
				continue;
			}
		}
		
		/* Comprobamos si tenemos que abrir el fichero de salida */  
		if (filev[1]!=NULL){
			
			fdOut = creat (filev[1], 0666);
			
			/* Si se produce un error al abrir el fichero de salida */
			if (fdOut<0){
				printf ("Error al crear el fichero de salida!\n");			
				continue;
			}
		}
		
		/* Comprobamos si tenemos que abrir el fichero de errores */  
		if (filev[2]!=NULL){
			
			fdError = creat (filev[2], 0666);
			
			/* Si se produce un error al abrir el fichero de errores */
			if (fdError<0){
				printf ("Error al crear el fichero de errores!\n");			
				continue;
			}
		}
																		
		/* Creamos tantos procesos como comandos a ejecutar */
		for (i=0; i<argvc; i++){					
			
			/* Creamos el PIPE correspondiente... si procede */					
						
			/* Si somos un proceso par y quedan m�s procesos por crear */
			if ((i<(argvc-1)) && (i%2==0)){
				
				while (pipe(pipe1)==-1)
					printf("Error al crear el pipe1\n");
			}
			
			/* Si somos un proceso impar y quedan m�s procesos por crear */
			if ((i<(argvc-1)) && (i%2==1)){
				
				while (pipe(pipe2)==-1)
					printf("Error al crear el pipe2\n");
			}								


			/* Comprobamos si tenemos que ejecutar un MANDATO INTERNO */

			/* Si es mytime, no es en BG y es el �ltimo */
			if (((strcmp(argvv[i][0],"mytime")==0)&&(!bg)&&(argvc>1)&&(i==argvc-1)) ||
					((argvc==1)&&(strcmp(argvv[i][0],"mytime")==0)&&(!bg))){						
						
					/* Si hay que direccionar la salida */	
					if (filev[1]!=NULL){
						outPadre = dup (STDOUT_FILENO);
						close (STDOUT_FILENO);
						dup (fdOut);
						close (fdOut);						
					}

					/* Ejecutamos... */
					mytime(argvv[i]);

					/* Dejamos la salida como antes... */
					if (filev[1]!= NULL){
						close(STDOUT_FILENO);
						dup(outPadre);
					}
					
					continue;
			}


		   /* Si es mypwd, no es en BG y es el �ltimo */
			if (((strcmp(argvv[i][0],"mypwd")==0)&&(!bg)&&(argvc>1)&&(i==argvc-1)) ||
					((argvc==1)&&(strcmp(argvv[i][0],"mypwd")==0)&&(!bg))){						
						
					/* Si hay que direccionar la salida */	
					if (filev[1]!=NULL){
						outPadre = dup (STDOUT_FILENO);
						close (STDOUT_FILENO);
						dup (fdOut);
						close (fdOut);						
					}

					/* Ejecutamos... */
					mypwd(argvv[i]);

					/* Dejamos la salida como antes... */
					if (filev[1]!= NULL){
						close(STDOUT_FILENO);
						dup(outPadre);
					}
					
					continue;
			}
			
						
			/* Creamos un nuevo proceso... */
			pid = fork();
						
			/* Error al crear proceso hijo */
			if (pid==-1){
				printf ("Error al crear proceso hijo!\n");
				break;
			}
			
			/* Si soy el proceso hijo */			
			else if (pid==0){
				
				/* PRIMER MANDATO */
				if (i==0){
					
					/* Tiene que leer del fichero de entrada */
					if (filev[0]!=NULL){
						close (STDIN_FILENO);	// Cerramos IN est�ndar
						dup (fdIn);				// Fichero -> IN est�ndar
						close (fdIn);			// Cerramos fichero
					}					
					
					/* Si es el �nico mandato */
					if (argvc==1){
						
						/* Tiene que leer del fichero de salida */
						if (filev[1]!=NULL){					
							close (STDOUT_FILENO);
							dup (fdOut);
							close (fdOut);							
						}
						
						/* Tiene que leer del fichero de error */
						if (filev[2]!=NULL){
							close (STDERR_FILENO);
							dup (fdError);
							close (fdError);							
						}
					}
					
					/* Al menos hay 2 mandatos, redireccionamos pipes ... */
					else if (argvc>1){
						close (STDOUT_FILENO);	// Cerramos OUT est�ndar
						close (pipe1[0]);		// Cerramos pipe1[RD]
						dup (pipe1[1]);			// pipe1[WR] -> OUT est�ndar
						close (pipe1[1]);		// Cerramos pipe1[WR]						
					}
					
				} /* Primer mandato */	
														
				
				/* �LTIMO MANDATO */
				else if ((i==(argvc-1)) && (i!=0)){
					
					/* Tiene que leer del fichero de salida */
					if (filev[1]!=NULL){					
						close (STDOUT_FILENO);
						dup (fdOut);
						close (fdOut);							
					}
					
					/* Tiene que leer del fichero de error */
					if (filev[2]!=NULL){					
						close (STDERR_FILENO);
						dup (fdError);
						close (fdError);							
					}					
					
					/* Comprobamos si hay que redireccionar pipes */
					
					/* Si somos un proceso impar*/
					if (i%2==1){
						close (STDIN_FILENO);	// Cerramos IN est�ndar
						close (pipe1[1]);		// Cerramos pipe2[WR]
						dup (pipe1[0]);			// pipe2[RD] -> IN est�ndar
						close (pipe1[0]);		// Cerramos pipe2[RD]
					}
										
					/* Si somos un proceso par */
					else{
						close (STDIN_FILENO);	// Cerramos IN est�ndar
						close (pipe2[1]);		// Cerramos pipe1[WR]
						dup (pipe2[0]);			// pipe1[RD] -> IN est�ndar
						close (pipe2[0]);		// Cerramos pipe1[RD]
					}				
					
				} /* �ltimo mandato */				
				
				
								
				/* MANDATO INTERMEDIO */
				else{
					
					/* Es un mandato par */
					if (i%2==0){
						
						/* Redireccionamos la salida del pipe 1 */
						close (STDIN_FILENO);	// Cerramos IN est�ndar
						close (pipe2[1]);		// Cerramos pipe1[WR]
						dup (pipe2[0]);			// pipe1[RD] -> IN est�ndar
						close (pipe2[0]);		// Cerramos pipe1[RD]
						
						/* Redireccionamos la salida del pipe 2 */
						close (STDOUT_FILENO);	// Cerramos OUT est�ndar
						close (pipe1[0]);		// Cerramos pipe2[RD]
						dup (pipe1[1]);			// pipe2[WR] -> OUT est�ndar
						close (pipe1[1]);		// Cerramos pipe2[WR]						
					}
					
					
					/* Es un mandato impar*/
					else{
												
						/* Redireccionamos la salida del pipe 1 */
						close (STDIN_FILENO);	// Cerramos IN est�ndar
						close (pipe1[1]);		// Cerramos pipe1[WR]
						dup (pipe1[0]);			// pipe1[RD] -> IN est�ndar
						close (pipe1[0]);		// Cerramos pipe1[RD]
						
						/* Redireccionamos la salida del pipe 2 */
						close (STDOUT_FILENO);	// Cerramos OUT est�ndar
						close (pipe2[0]);		// Cerramos pipe2[RD]
						dup (pipe2[1]);			// pipe2[WR] -> OUT est�ndar
						close (pipe2[1]);		// Cerramos pipe2[WR]						
					}					
					
				} /* Mandato intermedio */
								
				
				/* Ejecutamos el comando */
				if ((strcmp(argvv[i][0],"mytime")==0)){
					mytime(argvv[i]);
					exit(0);
				}

				else 
				  if ((strcmp(argvv[i][0],"mypwd")==0)){
						mypwd(argvv[i]);
						exit(0);
		  		  }

				else							
	 				execvp (argvv[i][0], argvv[i]);						
				
			} /* Soy proceso hijo */
			
			
			
			/* Si soy el padre... */
			else{
				
				/* Se cierran los pipes correspondientes... */
				if ((i%2==0) && (i!=0)){
					close (pipe2[0]);
					close (pipe2[1]);
				}			
				
				else if (i%2==1){
					close (pipe1[0]);
					close (pipe1[1]);
				}			
				
				
				/* Hay que ejecutar en background... */
				if (bg)
  				  printf("[%d]\n", pid);
				
				/* Esperamos a que el �ltimo mandato termine... */
				else{
					if (i==(argvc-1)){
						while (pid!=wait(&status)) 
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

int mytime(char **mandato){
	
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

	/*
	int numMin, numMax;
	int i;

	// Validar cmd 
	if ((mandato[1]==NULL)||(mandato[2]==NULL)){
		printf("Usage: stop minPid maxPid");
		return(-1);
	}
	
	numMin=atoi(mandato[1]);
	numMax=atoi(mandato[2]);

	
	// Envio SIGKILL a un proceso 
	if (numMin>numMax){
		printf("Error, no existe el proceso\n");
		exit(0);
	}
										               
	for (i=numMin;i<=numMax;i++)	
		kill(i, SIGSTOP);
	*/
	return 0;
}
