#include <dirent.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


typedef struct tipo_leituras
{
    int lat;
    int valores[360];

} Leituras;


typedef struct tipo_leitura_data
{
	int data;
	Leituras latitudes[180];
} LeituraData;

typedef struct tipo_quinzena
{
	int idQuinz;
	int[] lat;
	int[] lon;
	int[] qtdDiasValidos;
	int[] somaMedicao;
} Quinzena;



void bzero(void *s, size_t n);
FILE *fopen(const char *filename, const char *mode);
int fclose(FILE *a_file);
size_t getline(char **lineptr, size_t *n, FILE *stream);
char *strptime(const char *buf, const char *format, struct tm *tm);
void lerArquivo(LeituraData* leituraData, char file[]);
void agruparQuinzena(LeituraData leituras[]);





int main(int argc, char const *argv[])
{

	// DIR  *d;
 //  	struct dirent *dir;
 //  	d = opendir(".");
 //  	if (d)
 //  	{
 //    	while ((dir = readdir(d)) != NULL){
 //      		printf("%s\n", dir->d_name);
 //    	}

 //    	closedir(d);
 //  	}
    
//    char file[] = "";

	LeituraData leiturasPorData[2];

    lerArquivo(&leiturasPorData[0], "/mnt/695f96e3-9d30-4f93-a1a5-2f9262536915/omi/L3_ozone_omi_20130507.txt");
    lerArquivo(&leiturasPorData[1], "/mnt/695f96e3-9d30-4f93-a1a5-2f9262536915/omi/L3_ozone_omi_20130508.txt");

    for (int i = 0; i < 2; ++i)
    {
    	for (int j = 0; j < 180; ++j)
	    {
	        Leituras leitura = leiturasPorData[i].latitudes[j];
	        printf("data: %i\n", leiturasPorData[i].data);
	        printf("Latitude: %i\n", leitura.lat);
	    }
    }
    
    /* code */
    return 0;
}


void lerArquivo(LeituraData* leiturasPorData, char file[]){
	char *buffer = NULL;
    size_t nBuffer = 0;
    size_t nRead;
	FILE *in = fopen(file, "r");
    int nLinha = 0;

    
    int qtdLatitudes = 0;



    printf("Teste s: %i\n", sizeof(buffer[0]));
    int comecoLinha = 0;
    char medicao[3] = "";
    int nCharMedicao = 0;
    int qtdLeitura = 0;
    while((nRead = getline(&buffer, &nBuffer, in)) > 0 && nRead < 100)
    {
        if (nLinha++ < 3)
        {
        	if (nLinha == 1)
        	{	
        		int nDia = 0;
        		char dia[3];
        		while(nDia < 3){
        			dia[nDia] = buffer[6 + nDia];
        			nDia++;
        		}
    			leiturasPorData->data = atoi(dia);
        		
        	}
            continue;
        }
        if (nLinha == 2704)
        {
            printf("abacate");
        }
        printf("n/B:%i/%s", nLinha, buffer);
        

        for (int i = 1; i < nRead; i++)
        {
     
            char c = buffer[i];
            if (qtdLeitura == 359)
            {
                int j = i + 1;
                while((c = buffer[j++]) != 'l');
                if (c == 'l')
                {
                    char lat[5];
                    int nCharLat = 0;
                    j = j + 3;
                    while((c = buffer[j++]) != '=');
                    while((c = buffer[j++]) == ' ');
                    do
                    {
                        lat[nCharLat++] = c;
                    }
                    while((c = buffer[j++])!=' ' && c != '\n');
                    printf("Ql%i\n", qtdLatitudes);
                    leiturasPorData->latitudes[qtdLatitudes].lat=atoi(lat);
                    qtdLeitura = 0;
                    qtdLatitudes++;
                    break;
                }
            }

            medicao[nCharMedicao++] = c;

            if (nCharMedicao == 3)
            {
                int m = atoi(medicao);
                leiturasPorData->latitudes[qtdLatitudes].valores[qtdLeitura++] = m;
                bzero(medicao, 3);
                nCharMedicao = 0;
            }

        }

    }
    fclose(in);


// void agruparQuinzena(LeituraData leituras[], int qtdDias){
// 	Quinzena quinzenas[25];

// 	for (int i = 0; i < qtdDias; ++i)
// 	{
// 		LeituraData leitura = leituras[i];
// 		int qz = leitura.dia % 15;
// 		Quinzena quinzena = quinzenas[qz];
// 		for (int j = 0; j < 180; ++j)
// 		{
// 			quinzena.lat[j] = leitura.latitudes[j].lat;
// 			int lon = -180;
// 			for (int k = 0; k < 360; ++k)
// 			{
// 				quinzena.lon[k] = 
// 			}			
// 		}


// 	}
// }