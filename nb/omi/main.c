#include <dirent.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

typedef struct tipo_leituras {
    int lat;
    int valores[360];

} Leituras;

typedef struct tipo_leitura_data {
    int ano;
    int dia;
    Leituras latitudes[180];
} LeituraData;

typedef struct tipo_quinzena {
    int somaMedicao;
    int nroDias;
    int qudrados;
    float media;
    float desvioPadrao;
    
} Quinzena;

typedef struct tipo_posicao {
    Quinzena quinzenas[26];
} Posicao;

typedef struct tipo_ano {
    int ano;
    Posicao* posicoes[64800];
} Ano;

typedef struct tipo_lista_arquivos {
    char *nome;
    struct tipo_lista_arquivos* proximo;
} ListaArquivos;





double pow(double n, double e);
double sqrt(double n);
void bzero(void *s, size_t n);
FILE *fopen(const char *filename, const char *mode);
int fclose(FILE *a_file);
size_t getline(char **lineptr, size_t *n, FILE *stream);
char *strptime(const char *buf, const char *format, struct tm *tm);
void lerArquivo(LeituraData* leituraData, char file[]);
void agruparQuinzena(LeituraData leituras[], int nroDias, Ano *ano);
void calcularMediaQuinzena(Quinzena *quinzena);
int calcularMedia(int somaMedicao, int nroDias, int somaQuadrados, float* media, float* desvioPadrao);
int listarArquivos(ListaArquivos* pastaBase);

int cmpLeituraData(const void* argl1, const void* argl2){
    LeituraData* l1 = (LeituraData*)argl1;
    LeituraData* l2 = (LeituraData*)argl2;
    if(l1->ano == l2->ano){
        if(l1->dia > l2->dia){
            return 1;
        } else if(l1->dia < l2->dia){
            return -1;
        } else {
            return 0;
        }
    } else {
        return l1->ano > l2->ano ? 1 : 0;
    }
}
int main() {


    ListaArquivos pastaBase;
    pastaBase.nome = "/mnt/695f96e3-9d30-4f93-a1a5-2f9262536915/omi/2012/";
    int nrArquivos = listarArquivos(&pastaBase);
    LeituraData* leiturasPorData = malloc(sizeof(LeituraData) * nrArquivos);
    int nArq = 0;
    ListaArquivos* arquivo = pastaBase.proximo;
    while (nArq < nrArquivos) {
        lerArquivo(&leiturasPorData[nArq++], arquivo->nome);
        arquivo = arquivo->proximo;
    }

//    lerArquivo(&leiturasPorData[0], "/mnt/695f96e3-9d30-4f93-a1a5-2f9262536915/omi/L3_ozone_omi_20130507.txt");
//    lerArquivo(&leiturasPorData[1], "/mnt/695f96e3-9d30-4f93-a1a5-2f9262536915/omi/L3_ozone_omi_20130508.txt");

//    for (int i = 0; i < nrArquivos; ++i) {
//        for (int j = 0; j < 180; ++j) {
//            Leituras leitura = leiturasPorData[i].latitudes[j];
//            printf("data: %i\n", leiturasPorData[i].dia);
//            printf("Latitude: %i\n", leitura.lat);
//        }
//    }
    Ano anos[3];
    qsort(leiturasPorData, nrArquivos, sizeof(LeituraData), cmpLeituraData);
    
    //##################################################################################
    // Modo últimos 15 dias: cálcula a média dos últimos 15 dias em relação ao dia atual.
    for (int h = 0; h < nrArquivos; h++) {
        LeituraData leituraData = leiturasPorData[h];
        if(leituraData.ano ==0){
            continue;
        }
        int qz = leituraData.dia / 15;
        for (int j = 0; j < 180; ++j) {
            Leituras leitura = leituraData.latitudes[j];
            for (int k = 0; k < 360; ++k) {
                if(leitura.valores[k] > 0) {
//                    if(leituraData.ano == 2013 && leituraData.dia == 295){
//                        printf("Deveria  ter problema aqui....");
//                    } 
//                    if((j - 90) == 64 && (k-180) == -156 && leituraData.ano == 2013 && qz == 16){
//                        printf("pausa pro debug");
//                    }
                    int somaMedida = leitura.valores[k];
                    int somaQuadrados = pow(leitura.valores[k], 2);
                    int diasPraTras = 1;
                    int diaAux = leituraData.dia;
                    while(diaAux > leituraData.dia - 15 && (h - diasPraTras) >= 0) {
                        LeituraData leituraDataAux = leiturasPorData[h - diasPraTras];
                        diaAux = leituraDataAux.dia;
                        int medicao = leituraDataAux.latitudes[j].valores[k];
                        somaMedida += medicao;
                        somaQuadrados += pow(medicao, 2);
                        diasPraTras++;
                        
                    }
                    float media;
                    float desvioPadrao;
                    calcularMedia(somaMedida, diasPraTras, somaQuadrados, (float*)&media, (float*)&desvioPadrao);
                    
                    
                    if(leitura.valores[k] < media - 1.5 * desvioPadrao){
//                        if (j - 90 < -60 || j - 90 > 0){
//                            continue;
//                        }
//                        if(leituraData.dia == 27 && leituraData.ano == 2013){
//                        if((j - 90) == -54 && (k-180) == -3 && leituraData.ano == 2013 && leituraData.dia > 15 && leituraData.dia < 32){
//                        if((j - 90) == 64 && (k-180) == -156 && leituraData.ano == 2013 && qz == 16){
                            printf("Pre-ri-go [ano: %i, qz: %i, dia %i, P(%i, %i): %i, média: %f, desvio: %f\n", leituraData.ano, qz, leituraData.dia, 
                                    j - 90, k - 180, leitura.valores[k], media, desvioPadrao);
//                        }
                    }
                }
            }
        }
    }
    //##################################################################################
    // Modo agrupar quinzena: Cria grupos de quinzenas e calcula as médias por grupo
    
    /*
    agruparQuinzena(leiturasPorData, nrArquivos, anos);
    
    
    for (int h = 0; h < nrArquivos; h++) {
        LeituraData leituraData = leiturasPorData[h];
        if(leituraData.ano ==0){
            continue;
        }
        int qz = leituraData.dia / 15;
        for (int j = 0; j < 180; ++j) {
            Leituras leitura = leituraData.latitudes[j];
            for (int k = 0; k < 360; ++k) {
                if(leitura.valores[k] > 0) {
//                    if(leituraData.ano == 2013 && leituraData.dia == 295){
//                        printf("Deveria  ter problema aqui....");
//                    } 
//                    if((j - 90) == 64 && (k-180) == -156 && leituraData.ano == 2013 && qz == 16){
//                        printf("pausa pro debug");
//                    }
                    calcularMedia(&(anos[leituraData.ano - 2012].posicoes[j * 360 + k])->quinzenas[qz]);
                    float media = (anos[leituraData.ano - 2012].posicoes[j * 360 + k])->quinzenas[qz].media;
                    float desvioPadrao = (anos[leituraData.ano - 2012].posicoes[j * 360 + k])->quinzenas[qz].desvioPadrao;
                    if(leitura.valores[k] < media - 1.5 * desvioPadrao){
                        if (j - 90 < -60 || j - 90 > 0){
                            continue;
                        }
//                        if(leituraData.dia == 27 && leituraData.ano == 2013){
//                        if((j - 90) == -54 && (k-180) == -3 && leituraData.ano == 2013 && leituraData.dia > 15 && leituraData.dia < 32){
//                        if((j - 90) == 64 && (k-180) == -156 && leituraData.ano == 2013 && qz == 16){
                            printf("Pre-ri-go [ano: %i, qz: %i, dia %i, P(%i, %i): %i, média: %f, desvio: %f\n", leituraData.ano, qz, leituraData.dia, 
                                    j - 90, k - 180, leitura.valores[k], media, desvioPadrao);
//                        }
                    }
                }
            }
        }
    }
     */
    

    /* code */
    return 0;
}

int listarArquivos(ListaArquivos* pastaBase){
    DIR *d;
    struct dirent *dir;
    d = opendir(pastaBase->nome);
    int qtdArquivos = 0;
    if (d) {
        ListaArquivos* arquivo = pastaBase;
        while ((dir = readdir(d)) != NULL) {
            if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0){
                continue;
            }
            
            qtdArquivos++;
            printf("%s\n", dir->d_name);
            arquivo->proximo = malloc(sizeof(ListaArquivos));
            arquivo = arquivo->proximo;
            arquivo->nome = malloc(sizeof(dir->d_name) + sizeof(pastaBase->nome));
            strcpy(arquivo->nome, pastaBase->nome);
            strcat(arquivo->nome, dir->d_name);
        }

        closedir(d);
    }
    return qtdArquivos;

}

void lerArquivo(LeituraData* leiturasPorData, char file[]) {
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
    while ((nRead = getline(&buffer, &nBuffer, in)) > 0 && nRead < 100) {
        if (nLinha++ < 3) {
            if (nLinha == 1) {
                int nChar = 0;
                char dia[3];
                while (nChar < 3) {
                    dia[nChar] = buffer[6 + nChar];
                    nChar++;
                }
                leiturasPorData->dia = atoi(dia);
                nChar = 0;
                char ano[4];
                while (nChar < 4) {
                    ano[nChar] = buffer[18 + nChar];
                    nChar++;
                }
                leiturasPorData->ano = atoi(ano);
                

            }
            continue;
        }
        if (nLinha == 2704) {
            printf("abacate");
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
                    }                    while ((c = buffer[j++]) != ' ' && c != '\n');
                    //printf("Ql%i\n", qtdLatitudes);
                    leiturasPorData->latitudes[qtdLatitudes].lat = atoi(lat);
                    qtdLeitura = 0;
                    qtdLatitudes++;
                    break;
                }
            }

            medicao[nCharMedicao++] = c;

            if (nCharMedicao == 3) {
                int m = atoi(medicao);
                leiturasPorData->latitudes[qtdLatitudes].valores[qtdLeitura++] = m;
                bzero(medicao, 3);
                nCharMedicao = 0;
            }

        }

    }
    fclose(in);
}
//
void agruparQuinzena(LeituraData leituras[], int qtdDias, Ano *anos ){
    //        agrupador.ano

    //2.498.904
    //Quinzena quinzena = quinzenas[qz];
    int contaInicializacoes = 0;
    int contaReutilizacoes = 0;
    for (int i = 0; i < qtdDias; ++i) {
        LeituraData leituraData = leituras[i];
//        if(!(anos[leituraData.ano - 2012])){
//            anos[leituraData.ano - 2012] = malloc(sizeof(Ano));
//        }
//        Ano ano = anos[leituraData.ano - 2012];
        if(leituraData.ano ==0){
            continue;
        }
        int qz = leituraData.dia / 15;
        for (int j = 0; j < 180; ++j) {
            Leituras leitura = leituraData.latitudes[j];
            for (int k = 0; k < 360; ++k) {
                if(leitura.valores[k] > 0) {
                    if (!(anos[leituraData.ano - 2012].posicoes[j * 360 + k])) {
                        contaInicializacoes++;
                        //printf("Inicializando ano %i, posição [%i, %i]\n", leituraData.ano - 2012, j, k);
                        (anos[leituraData.ano - 2012].posicoes[j * 360 + k]) = malloc(sizeof (Posicao));
                        for(int l = 0; l < 26; l++){
                            (anos[leituraData.ano - 2012].posicoes[j * 360 + k])->quinzenas[l].nroDias = 0;
                            (anos[leituraData.ano - 2012].posicoes[j * 360 + k])->quinzenas[l].somaMedicao = 0;
                            (anos[leituraData.ano - 2012].posicoes[j * 360 + k])->quinzenas[l].qudrados = 0;

                        }
                        //                        (ano.posicoes[j*360 + k])->quinzenas[qz].quinzena = 0;
//                        (anos[leituraData.ano - 2012].posicoes[j * 360 + k])->quinzenas[qz].somaMedicao = 0;
                    } else {
                        contaReutilizacoes++;
                    }
//                    if (j * 360 + k == 8640 && qz == 16 ){
//                    if (j * 360 + k == 55464 && qz == 16 ){
//                        printf("abacate");
//                    }
                    (anos[leituraData.ano - 2012].posicoes[j * 360 + k])->quinzenas[qz].nroDias += 1;
                    //                    (ano.posicoes[j*360 + k])->quinzenas[qz].quinzena  = qz;
                    (anos[leituraData.ano - 2012].posicoes[j * 360 + k])->quinzenas[qz].qudrados += pow(leitura.valores[k], 2);
                    (anos[leituraData.ano - 2012].posicoes[j * 360 + k])->quinzenas[qz].somaMedicao += leitura.valores[k];
                }
            }

        }
    }



}

void calcularMediaQuinzena(Quinzena *quinzena){
        quinzena->media = (float)quinzena->somaMedicao / (float)quinzena->nroDias;
        if(quinzena->media > 1000 ){
            printf("abacate");
        }
        float variancia = (float_t)quinzena->qudrados - (quinzena->nroDias * pow(quinzena->media, 2));
        variancia = variancia/(quinzena->nroDias - 1);
        
        quinzena->desvioPadrao = sqrt(variancia);
}
int calcularMedia(int somaMedicao, int nroDias, int somaQuadrados, float* media, float* desvioPadrao){
        *media = (float)somaMedicao / nroDias;
        if(*media > 1000 ){
            printf("abacate");
        }
        float variancia = (float_t)somaQuadrados - (nroDias * pow(*media, 2));
        variancia = variancia/(nroDias - 1);
        
        *desvioPadrao = sqrt(variancia);
}