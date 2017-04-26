#include <dirent.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <sys/time.h>

typedef struct tipo_problema {
    int tId;
    
    int data;

    int lat;
    int lon;
    int medidaAtual;
    
    float media;
    float desvio;
    int qtdValores;
    int valores[15];

    struct tipo_problema *anterior;
    struct tipo_problema *proximo;

} Problema;

typedef struct tipo_leituras {
    int lat;
    int valores[360];

} Leituras;

typedef struct tipo_leitura_data {
    int ano;
    int dia;
    Leituras latitudes[180];
} LeituraData;

typedef struct tipo_posicao {
    int medicoes[366];
} Posicao;

typedef struct tipo_ano {
    int ano;
    struct tipo_ano *anterior;
    struct tipo_ano *proximo;
    Posicao* posicoes[64800];
} Ano;

typedef struct tipo_lista_arquivos {
    char *nome;
    struct tipo_lista_arquivos* proximo;
} ListaArquivos;




int gettimeofday(struct timeval *restrict tp, void *restrict tzp);
double pow(double n, double e);
double sqrt(double n);
void bzero(void *s, size_t n);
FILE *fopen(const char *filename, const char *mode);
int fclose(FILE *a_file);
int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);
size_t getline(char **lineptr, size_t *n, FILE *stream);
char *strptime(const char *buf, const char *format, struct tm *tm);
void lerArquivo(Ano* leituraData, char file[]);
void agruparQuinzena(LeituraData leituras[], int nroDias, Ano *ano);
int calcularMedia(int somaMedicao, int nroDias, int somaQuadrados, float* media, float* desvioPadrao);
int listarArquivos(ListaArquivos* pastaBase);
void sequencial(int argc, char const *argv[]);
void paralelo(int argc, char const *argv[]);
int problemaComparador(Problema* prob1, Problema* prob2);

int main(int argc, char const *argv[]) {


    paralelo(argc, argv);
    //    sequencial(argc, argv);

    return 0;
}

void paralelo(int argc, char const *argv[]) {
    omp_set_num_threads(1);
    //    int latIni = -40, latFin = -15, lonIni = -70, lonFin = -50, diaIni = 1, diaFin = 366;
    int latIni = -90, latFin = 90, lonIni = -180, lonFin = 180, diaIni = 1, diaFin = 366;
    printf("Arc: %i", argc);
    //    if (argc == 7) {
    //        printf("Arv: %s %s %s %s %s %s\n", argv[1],  argv[2], argv[3], argv[4], argv[5], argv[6]);
    //        latIni = atoi(argv[1]);
    //        latFin = atoi(argv[2]) + 90;
    //        lonIni = atoi(argv[3]) + 180;
    //        lonFin = atoi(argv[4]) + 180;
    //        diaIni = atoi(argv[5]) - 1;
    //        diaFin = atoi(argv[6]);
    //    }
    latIni += 90;
    latFin += 90;
    lonIni += 180;
    lonFin += 180;
    diaIni -= 1;


    ListaArquivos pastaBase;
    pastaBase.nome = "/mnt/695f96e3-9d30-4f93-a1a5-2f9262536915/omi/2012/";
    int nrArquivos = listarArquivos(&pastaBase);

    // Inicializa lista de problemas
    Problema problemas;
    problemas.data = 0;
    problemas.proximo = NULL;
    problemas.anterior = NULL;

    Problema* ultProblema = &problemas;

    // Inicializa lista de anos/medidas
    Ano primeiro;
    primeiro.ano = 0;
    primeiro.proximo = NULL;
    primeiro.anterior = NULL;
    ListaArquivos* arquivo = pastaBase.proximo;
    while (arquivo) {
        lerArquivo(&primeiro, arquivo->nome);
        arquivo = arquivo->proximo;
    }
    struct timeval tempoInicial;
    struct timeval tempoFinal;
    gettimeofday(&tempoInicial, NULL);
    

    int myId, prop, nthreads;
    Ano* anoAux = &primeiro;
    while ((anoAux = anoAux->proximo) != NULL) {
#pragma omp parallel private(myId) shared(nthreads, prop, ultProblema)
        {
#pragma omp single
            {
                nthreads = omp_get_num_threads();
                prop = (latFin - latIni) / nthreads;
            }
            myId = omp_get_thread_num();
            int j = latIni + (prop * myId);
            int jF;
            if (myId == nthreads - 1) {
                jF = latFin;

            } else {
                jF = j + prop;
            }
            for (; j < jF; ++j) {
                for (int k = lonIni; k < lonFin; ++k) {
                    Posicao* posicao = anoAux->posicoes[j * 180 + k];
                    if (posicao == NULL) {
                        continue;
                    }
                    for (int d = diaIni; d < diaFin; d++) {
                        int med = posicao->medicoes[d];

                        if (med > 0) {
                            int somaMedida = med;
                            int somaQuadrados = pow(med, 2);
                            int diasPraTras = 1;
                            int diaAux = d;
                            int ignorados = 0;
                            int diasUsados[15];
                            diasUsados[0] = med;
                            while (diaAux + ignorados > d - 15 && (d - diasPraTras) >= 0) {
                                int medicao = posicao->medicoes[d - diasPraTras];
                                //                            printf("Dia %i, medição %i\n", d, medicao);
                                if (medicao > 0) {
                                    somaMedida += medicao;
                                    somaQuadrados += pow(medicao, 2);
                                    diasUsados[diasPraTras - ignorados] = medicao;
                                } else {
                                    ignorados++;
                                }
                                diaAux--;
                                diasPraTras++;

                            }
                            float media;
                            float desvioPadrao;
                            calcularMedia(somaMedida, diasPraTras - ignorados, somaQuadrados, (float*) &media, (float*) &desvioPadrao);


                            if (posicao->medicoes[d] < media - (3.5 * desvioPadrao)) {
                                ultProblema->proximo = malloc(sizeof (Problema));
                                ultProblema->proximo->anterior = ultProblema;
                                ultProblema = ultProblema->proximo;
                                ultProblema->data = anoAux->ano * 1000 + d;
                                ultProblema->desvio = desvioPadrao;
                                ultProblema->media = media;
                                ultProblema->lat = j - 90;
                                ultProblema->lat = k - 180;
                                ultProblema->medidaAtual = posicao->medicoes[d];
                                ultProblema->tId = myId;
                                ultProblema->qtdValores = diasPraTras - ignorados;
                                memcpy(ultProblema->valores, diasUsados, (diasPraTras - ignorados) * sizeof(int));



//                                struct tm data = {.tm_mday = d, .tm_year = anoAux->ano - 1900};
//                                time_t dataConv = mktime(&data);
//                                data = *localtime(&dataConv);
//                                char strData[20];
//                                strftime(strData, 20, "%d/%m/%y", &data);
//                                printf("Pre-ri-go [myId: %i, dia %s, P(%i, %i): %i, média: %f, desvio: %f\n", myId, strData,
//                                        j - 90, k - 180, posicao->medicoes[d], media, desvioPadrao);
                                //                            printf("Pre-ri-go [ano: %i, dia %i, P(%i, %i): %i, média: %f, desvio: %f\n", anoAux->ano, d,
                                //                                    j - 90, k - 180, posicao->medicoes[d], media, desvioPadrao);
                                //                        }
                            }
                        }

                    }
                }
            }
        }
    }
    gettimeofday(&tempoFinal, NULL);

    Problema* probAux = &problemas;
    while (probAux->proximo != NULL) {
        if (probAux->data == 0) {
            probAux = probAux->proximo;
            continue;
        }
        if (probAux->proximo != NULL) {
            int cmp = problemaComparador(probAux, probAux->proximo);
            if (cmp < 0) {
                // trocar
                Problema* p1 = probAux->anterior;
                Problema* p2 = probAux;
                Problema* p3 = probAux->proximo;
                p1->proximo = p3;
                p2->anterior = p3;
                p2->proximo = p3->proximo;
                p3->anterior = p1;
                p3->proximo = p2;
            } else {
                probAux = probAux->proximo;
            }
        }

    }
    probAux = &problemas;
    while (probAux->proximo != NULL) {
        if (probAux->data == 0) {
            probAux = probAux->proximo;
            continue;
        }

        struct tm data = {.tm_mday = probAux->data % 1000, .tm_year = probAux->data / 1000 - 1900};
        time_t dataConv = mktime(&data);
        data = *localtime(&dataConv);
        char strData[20];
        strftime(strData, 20, "%d/%m/%y", &data);
        printf("Pre-ri-go [myId: %i, dia %s, P(%i, %i): %i, média: %f, desvio: %f, [", probAux->tId, strData,
                probAux->lat, probAux->lon, probAux->medidaAtual, probAux->media, probAux->desvio);
        for(int i = 0; i < probAux->qtdValores; i++){
            printf("%i, ", probAux->valores[i]);
        }
        printf("]\n");

        probAux = probAux->proximo;
    }
    int sleep = 1;
    while (sleep++ < 100000000);

    //
    printf("Executado em %lu milisegundos \n", (tempoFinal.tv_sec - tempoInicial.tv_sec) * 1000  + (tempoFinal.tv_usec - tempoInicial.tv_usec) / 1000);
    printf("\n");


}

int problemaComparador(Problema* prob1, Problema* prob2) {
    if (prob2->data < prob1->data) {
        return -1;
    } else if (prob2->data == prob1->data) {
        if (prob2->lat < prob1->lat) {
            return -1;
        } else if (prob2->lat == prob1->lat) {
            if (prob2->lon < prob1->lon) {
                return -1;
            } else if (prob2->lon == prob1->lon) {
                return 0;
            } else {
                return 1;
            }
        }

    }
}

void sequencial(int argc, char const *argv[]) {
    //    int latIni = -40, latFin = -15, lonIni = -70, lonFin = -50, diaIni = 1, diaFin = 366;
    int latIni = -90, latFin = 90, lonIni = -180, lonFin = 180, diaIni = 1, diaFin = 366;
    printf("Arc: %i", argc);
    //    if (argc == 7) {
    //        printf("Arv: %s %s %s %s %s %s\n", argv[1],  argv[2], argv[3], argv[4], argv[5], argv[6]);
    //        latIni = atoi(argv[1]);
    //        latFin = atoi(argv[2]) + 90;
    //        lonIni = atoi(argv[3]) + 180;
    //        lonFin = atoi(argv[4]) + 180;
    //        diaIni = atoi(argv[5]) - 1;
    //        diaFin = atoi(argv[6]);
    //    }
    latIni += 90;
    latFin += 90;
    lonIni += 180;
    lonFin += 180;
    diaIni -= 1;


    ListaArquivos pastaBase;
    pastaBase.nome = "/mnt/695f96e3-9d30-4f93-a1a5-2f9262536915/omi/";
    int nrArquivos = listarArquivos(&pastaBase);

    Ano primeiro;
    primeiro.ano = 0;
    primeiro.proximo = NULL;
    primeiro.anterior = NULL;
    ListaArquivos* arquivo = pastaBase.proximo;
    while (arquivo) {
        lerArquivo(&primeiro, arquivo->nome);
        arquivo = arquivo->proximo;
    }


    Ano* anoAux = &primeiro;
    while ((anoAux = anoAux->proximo) != NULL) {
        for (int j = latIni; j < latFin; ++j) {
            for (int k = lonIni; k < lonFin; ++k) {
                Posicao* posicao = anoAux->posicoes[j * 180 + k];
                if (posicao == NULL) {
                    continue;
                }
                for (int d = diaIni; d < diaFin; d++) {
                    int med = posicao->medicoes[d];

                    if (med > 0) {
                        int somaMedida = med;
                        int somaQuadrados = pow(med, 2);
                        int diasPraTras = 1;
                        int diaAux = d;
                        int ignorados = 0;
                        while (diaAux + ignorados > d - 15 && (d - diasPraTras) >= 0) {
                            int medicao = posicao->medicoes[d - diasPraTras];
                            //                            printf("Dia %i, medição %i\n", d, medicao);
                            if (medicao > 0) {
                                somaMedida += medicao;
                                somaQuadrados += pow(medicao, 2);
                            } else {
                                ignorados++;
                            }
                            diaAux--;
                            diasPraTras++;

                        }
                        float media;
                        float desvioPadrao;
                        calcularMedia(somaMedida, diasPraTras - ignorados, somaQuadrados, (float*) &media, (float*) &desvioPadrao);


                        if (posicao->medicoes[d] < media - (3.5 * desvioPadrao)) {
                            struct tm data = {.tm_mday = d, .tm_year = anoAux->ano - 1900};
                            time_t dataConv = mktime(&data);
                            data = *localtime(&dataConv);
                            char strData[20];
                            strftime(strData, 20, "%d/%m/%y", &data);
                            printf("Pre-ri-go [dia %s, P(%i, %i): %i, média: %f, desvio: %f\n", strData,
                                    j - 90, k - 180, posicao->medicoes[d], media, desvioPadrao);
                            //                            printf("Pre-ri-go [ano: %i, dia %i, P(%i, %i): %i, média: %f, desvio: %f\n", anoAux->ano, d,
                            //                                    j - 90, k - 180, posicao->medicoes[d], media, desvioPadrao);
                            //                        }
                        }
                    }

                }
            }
        }
    }
}

int listarArquivos(ListaArquivos* pastaBase) {
    DIR *d;
    struct dirent *dir;
    d = opendir(pastaBase->nome);
    int qtdArquivos = 0;
    regex_t reg;
    regcomp(&reg, ".*[.]txt", 0);
    if (d) {
        ListaArquivos* arquivo = pastaBase;
        while ((dir = readdir(d)) != NULL) {
            //            if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)){
            if (regexec(&reg, dir->d_name, 0, NULL, 0) != 0) {
                continue;
            }

            qtdArquivos++;
            //printf("%s\n", dir->d_name);
            arquivo->proximo = malloc(sizeof (ListaArquivos));
            arquivo = arquivo->proximo;
            arquivo->nome = malloc(sizeof (dir->d_name) + sizeof (pastaBase->nome));
            strcpy(arquivo->nome, pastaBase->nome);
            strcat(arquivo->nome, dir->d_name);
        }

        closedir(d);
    }
    return qtdArquivos;

}

void lerArquivo(Ano* agAno, char file[]) {
    char *buffer = NULL;
    size_t nBuffer = 0;
    size_t nRead;
    FILE *in = fopen(file, "r");
    int nLinha = 0;


    int qtdLatitudes = 0;



    //    printf("Teste s: %i\n", sizeof (buffer[0]));
    char medicao[3] = "";
    int nCharMedicao = 0;
    int qtdLeitura = 0;
    int dia = 0;
    while ((nRead = getline(&buffer, &nBuffer, in)) > 0 && nRead < 100) {
        if (nLinha++ < 3) {
            if (nLinha == 1) {
                int nChar = 0;
                char diaStr[3];
                while (nChar < 3) {
                    diaStr[nChar] = buffer[6 + nChar];
                    nChar++;
                }
                nChar = 0;
                char anoStr[4];
                while (nChar < 4) {
                    anoStr[nChar] = buffer[18 + nChar];
                    nChar++;
                }
                int ano = atoi(anoStr);
                dia = atoi(diaStr);
                while (ano > agAno->ano) {
                    if (agAno->proximo) {
                        agAno = agAno->proximo;
                    } else {
                        agAno->proximo = malloc(sizeof (Ano));
                        agAno->proximo->anterior = agAno;
                        agAno->proximo->ano = ano;
                        agAno = agAno->proximo;
                    }
                }
                if (ano < agAno->ano) {
                    Ano* anoAnterior = agAno->anterior;
                    Ano* anoProximo = agAno;
                    agAno = malloc(sizeof (Ano));
                    agAno->ano = ano;
                    anoAnterior->proximo = agAno;
                    agAno->anterior = anoAnterior;
                    anoProximo->anterior = agAno;
                    agAno->proximo = anoProximo;
                }
            }
            continue;
        }

        //printf("n/B:%i/%s", nLinha, buffer);


        for (int i = 1; i < nRead - 1; i++) {

            char c = buffer[i];
            if (qtdLeitura == 359) {
                int j = i + 1;
                while ((c = buffer[j++]) != 'l');
                if (c == 'l') {
                    char lat[5];
                    int nCharLat = 0;
                    j = j + 3;
                    while ((c = buffer[j++]) != '=');
                    while ((c = buffer[j++]) == ' ');
                    do {
                        lat[nCharLat++] = c;
                    } while ((c = buffer[j++]) != ' ' && c != '\n');
                    //printf("Ql%i\n", qtdLatitudes);

                    qtdLeitura = 0;
                    qtdLatitudes++;
                    break;
                }
            }

            medicao[nCharMedicao++] = c;

            if (nCharMedicao == 3) {
                int m = atoi(medicao);
                int posicao = qtdLatitudes * 180 + qtdLeitura++;
                if (!agAno->posicoes[posicao]) {
                    agAno->posicoes[posicao] = malloc(sizeof (Posicao));
                    for (int i = 0; i < 366; i++) {
                        agAno->posicoes[posicao]->medicoes[i] = 0;
                    }
                }
                agAno->posicoes[posicao]->medicoes[dia] = m;
                bzero(medicao, 3);
                nCharMedicao = 0;
            }

        }

    }
    fclose(in);
}

int calcularMedia(int somaMedicao, int nroDias, int somaQuadrados, float* media, float* desvioPadrao) {
    *media = (float) somaMedicao / nroDias;
    if (*media > 1000) {
        printf("abacate");
    }
    float variancia = (float_t) somaQuadrados - (nroDias * pow(*media, 2));
    variancia = variancia / (nroDias - 1);

    *desvioPadrao = sqrt(variancia);
}