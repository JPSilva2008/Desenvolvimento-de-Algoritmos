#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_INVESTIDORES 100
#define MAX_ADMINS 10
#define SENHA_ADMIN_FIXA "123456" // Senha fixa obrigatória para administradores

// Estruturas
typedef struct {
    char nome[100];
    char cpf[12];    // 11 dígitos + '\0'
    char senha[7];   // 6 dígitos + '\0'
    double reais;
    double bitcoin;
    double ethereum;
    double ripple;
} Investidor;

typedef struct {
    char nome[100];
    char cpf[12];
} Administrador;

typedef struct {
    double bitcoin;
    double ethereum;
    double ripple;
} Cotacao;

// Protótipos das funções
void carregarCotacoes(Cotacao *cot);
void salvarCotacoes(Cotacao cot);
int carregarInvestidores(Investidor invs[]);
void salvarInvestidores(Investidor invs[], int total);
int carregarAdministradores(Administrador admins[]);
void salvarAdministradores(Administrador admins[], int total);

void registrarExtrato(Investidor inv, char *detalhes, double valorOp, double ct, double tx);
void consultarExtrato(char *cpf);
void atualizarCotacoes(Cotacao *cot);
int buscarInvestidorPorCPF(Investidor invs[], int total, char *cpf);
int buscarAdminPorCPF(Administrador admins[], int total, char *cpf);
void obterDataHora(char *buffer, int maxLen);

void cadastrarNovoInvestidor(Investidor invs[], int *totalInvs);
void cadastrarNovoAdmin(Administrador admins[], int *totalAdmins);

// Menus
void menuAdmin(Investidor invs[], int *totalInvs, Cotacao *cot);
void menuInvestidor(Investidor *inv, Cotacao *cot, Investidor invs[], int totalInvs);

int main() {
    srand(time(NULL));
    Investidor invs[MAX_INVESTIDORES];
    int totalInvs = carregarInvestidores(invs);

    Administrador admins[MAX_ADMINS];
    int totalAdmins = carregarAdministradores(admins);

    if (totalAdmins == 0) {
        strcpy(admins[0].nome, "Admin_Padrao");
        strcpy(admins[0].cpf, "11111111111");
        totalAdmins = 1;
        salvarAdministradores(admins, totalAdmins);
    }

    Cotacao cot;
    carregarCotacoes(&cot);

    int opcaoPrincipal;

    do {
        printf("\n========================================\n");
        printf("    EXCHANGE DE CRIPTOMOEDAS - FEI      \n");
        printf("========================================\n");
        printf("1. Area do Investidor\n");
        printf("2. Area do Administrador\n");
        printf("3. Sair do Programa\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcaoPrincipal) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch (opcaoPrincipal) {
            case 1: { // ÁREA DO INVESTIDOR
                int subOpcao;
                printf("\n=== AREA DO INVESTIDOR ===\n");
                printf("1. Fazer Login\n");
                printf("2. Cadastrar-se (Novo Investidor)\n");
                printf("3. Voltar\n");
                printf("Escolha uma opcao: ");

                if (scanf("%d", &subOpcao) != 1) {
                    while(getchar() != '\n');
                    break;
                }

                if (subOpcao == 1) {
                    char cpf[12];
                    char senha[7];

                    printf("\n--- LOGIN DO INVESTIDOR ---\n");
                    printf("Digite o seu CPF: ");
                    scanf("%11s", cpf);
                    printf("Digite a sua senha (6 digitos): ");
                    scanf("%6s", senha);

                    int idx = buscarInvestidorPorCPF(invs, totalInvs, cpf);
                    if (idx != -1 && strcmp(invs[idx].senha, senha) == 0) {
                        printf("\nLogin efetuado com sucesso! Bem-vindo(a), %s\n", invs[idx].nome);
                        menuInvestidor(&invs[idx], &cot, invs, totalInvs);
                        salvarInvestidores(invs, totalInvs);
                    } else {
                        printf("\n[Erro] CPF ou senha incorretos, ou investidor nao cadastrado!\n");
                    }
                } else if (subOpcao == 2) {
                    printf("\n--- AUTOCADASTRO DE INVESTIDOR ---\n");
                    cadastrarNovoInvestidor(invs, &totalInvs);
                }
                break;
            }
            case 2: { // ÁREA DO ADMINISTRADOR
                int subOpcao;
                printf("\n=== AREA DO ADMINISTRADOR ===\n");
                printf("1. Fazer Login como Admin\n");
                printf("2. Cadastrar novo Administrador\n");
                printf("3. Voltar\n");
                printf("Escolha uma opcao: ");

                if (scanf("%d", &subOpcao) != 1) {
                    while(getchar() != '\n');
                    break;
                }

                if (subOpcao == 1) {
                    char cpf_admin[12];
                    char senha_admin[7];

                    printf("\n--- LOGIN DO ADMINISTRADOR ---\n");
                    printf("Digite o seu CPF: ");
                    scanf("%11s", cpf_admin);
                    printf("Digite a senha fixa de administrador (6 digitos): ");
                    scanf("%6s", senha_admin);

                    int idxAdmin = buscarAdminPorCPF(admins, totalAdmins, cpf_admin);
                    
                    if (idxAdmin != -1 && strcmp(senha_admin, SENHA_ADMIN_FIXA) == 0) {
                        printf("\nLogin de Administrador efetuado com sucesso! Bem-vindo, %s\n", admins[idxAdmin].nome);
                        menuAdmin(invs, &totalInvs, &cot);
                    } else {
                        printf("\n[Erro] CPF nao cadastrado como admin ou senha fixa incorreta!\n");
                    }
                } else if (subOpcao == 2) {
                    printf("\n--- AUTOCADASTRO DE ADMINISTRADOR ---\n");
                    cadastrarNovoAdmin(admins, &totalAdmins);
                }
                break;
            }
            case 3:
                printf("\nEncerrando o programa. Ate logo!\n");
                break;
            default:
                printf("\n[Erro] Opcao invalida!\n");
                break;
        }
    } while (opcaoPrincipal != 3);

    return 0;
}

// --- PERSISTÊNCIA E FUNÇÕES AUXILIARES ---

void carregarCotacoes(Cotacao *cot) {
    FILE *f = fopen("cotacoes.txt", "r");
    if (!f) {
        cot->bitcoin = 350000.00;
        cot->ethereum = 18000.00;
        cot->ripple = 3.50;
        salvarCotacoes(*cot);
        return;
    }
    fscanf(f, "%lf %lf %lf", &cot->bitcoin, &cot->ethereum, &cot->ripple);
    fclose(f);
}

void salvarCotacoes(Cotacao cot) {
    FILE *f = fopen("cotacoes.txt", "w");
    if (f) {
        fprintf(f, "%.2f %.2f %.2f\n", cot.bitcoin, cot.ethereum, cot.ripple);
        fclose(f);
    }
}

int carregarInvestidores(Investidor invs[]) {
    FILE *f = fopen("investidores.txt", "r");
    if (!f) return 0;
    int i = 0;
    while (fscanf(f, "%11s %6s %99s %lf %lf %lf %lf", 
           invs[i].cpf, invs[i].senha, invs[i].nome, 
           &invs[i].reais, &invs[i].bitcoin, &invs[i].ethereum, &invs[i].ripple) == 7) {
        for (int j = 0; j < strlen(invs[i].nome); j++) {
            if (invs[i].nome[j] == '_') invs[i].nome[j] = ' ';
        }
        i++;
        if (i >= MAX_INVESTIDORES) break;
    }
    fclose(f);
    return i;
}

void salvarInvestidores(Investidor invs[], int total) {
    FILE *f = fopen("investidores.txt", "w");
    if (!f) return;
    for (int i = 0; i < total; i++) {
        char nomeTemp[100];
        strcpy(nomeTemp, invs[i].nome);
        for (int j = 0; j < strlen(nomeTemp); j++) {
            if (nomeTemp[j] == ' ') nomeTemp[j] = '_';
        }
        fprintf(f, "%s %s %s %.2f %.4f %.4f %.4f\n", 
                invs[i].cpf, invs[i].senha, nomeTemp, 
                invs[i].reais, invs[i].bitcoin, invs[i].ethereum, invs[i].ripple);
    }
    fclose(f);
}

int carregarAdministradores(Administrador admins[]) {
    FILE *f = fopen("administradores.txt", "r");
    if (!f) return 0;
    int i = 0;
    while (fscanf(f, "%11s %99s", admins[i].cpf, admins[i].nome) == 2) {
        for (int j = 0; j < strlen(admins[i].nome); j++) {
            if (admins[i].nome[j] == '_') admins[i].nome[j] = ' ';
        }
        i++;
        if (i >= MAX_ADMINS) break;
    }
    fclose(f);
    return i;
}

void salvarAdministradores(Administrador admins[], int total) {
    FILE *f = fopen("administradores.txt", "w");
    if (!f) return;
    for (int i = 0; i < total; i++) {
        char nomeTemp[100];
        strcpy(nomeTemp, admins[i].nome);
        for (int j = 0; j < strlen(nomeTemp); j++) {
            if (nomeTemp[j] == ' ') nomeTemp[j] = '_';
        }
        fprintf(f, "%s %s\n", admins[i].cpf, nomeTemp);
    }
    fclose(f);
}

int buscarInvestidorPorCPF(Investidor invs[], int total, char *cpf) {
    for (int i = 0; i < total; i++) {
        if (strcmp(invs[i].cpf, cpf) == 0) return i;
    }
    return -1;
}

int buscarAdminPorCPF(Administrador admins[], int total, char *cpf) {
    for (int i = 0; i < total; i++) {
        if (strcmp(admins[i].cpf, cpf) == 0) return i;
    }
    return -1;
}

void cadastrarNovoInvestidor(Investidor invs[], int *totalInvs) {
    if (*totalInvs >= MAX_INVESTIDORES) {
        printf("\n[Erro] Limite maximo de investidores atingido.\n");
        return;
    }

    Investidor novo;
    printf("Digite o seu nome completo (sem espacos, use sublinhado _): ");
    scanf("%99s", novo.nome);
    
    for (int j = 0; j < strlen(novo.nome); j++) {
        if (novo.nome[j] == '_') novo.nome[j] = ' ';
    }

    printf("Digite o seu CPF (11 digitos, sem pontos/tracos): ");
    scanf("%11s", novo.cpf);

    if (buscarInvestidorPorCPF(invs, *totalInvs, novo.cpf) != -1) {
        printf("\n[Erro] Ja existe um investidor cadastrado com este CPF!\n");
        return;
    }

    printf("Crie uma senha numerica (6 digitos): ");
    scanf("%6s", novo.senha);

    novo.reais = 0.0;
    novo.bitcoin = 0.0;
    novo.ethereum = 0.0;
    novo.ripple = 0.0;

    invs[*totalInvs] = novo;
    (*totalInvs)++;
    salvarInvestidores(invs, *totalInvs);

    char filename[30];
    sprintf(filename, "extrato_%s.txt", novo.cpf);
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "Nome: %s\nCPF: %s\n\n", novo.nome, novo.cpf);
        fclose(f);
    }

    printf("\n[Sucesso] Cadastro de investidor realizado com sucesso!\n");
}

void cadastrarNovoAdmin(Administrador admins[], int *totalAdmins) {
    if (*totalAdmins >= MAX_ADMINS) {
        printf("\n[Erro] Limite maximo de administradores atingido.\n");
        return;
    }

    Administrador novo;
    char senhaDigitada[7];

    printf("Digite o nome do administrador (sem espacos, use _): ");
    scanf("%99s", novo.nome);
    for (int j = 0; j < strlen(novo.nome); j++) {
        if (novo.nome[j] == '_') novo.nome[j] = ' ';
    }

    printf("Digite o CPF do administrador (11 digitos): ");
    scanf("%11s", novo.cpf);

    if (buscarAdminPorCPF(admins, *totalAdmins, novo.cpf) != -1) {
        printf("\n[Erro] Ja existe um administrador cadastrado com este CPF!\n");
        return;
    }

    printf("Digite a senha fixa de autorizacao (6 digitos): ");
    scanf("%6s", senhaDigitada);

    if (strcmp(senhaDigitada, SENHA_ADMIN_FIXA) != 0) {
        printf("\n[Erro] Senha fixa incorreta! Cadastro de administrador negado.\n");
        return;
    }

    admins[*totalAdmins] = novo;
    (*totalAdmins)++;
    salvarAdministradores(admins, *totalAdmins);

    printf("\n[Sucesso] Administrador cadastrado com sucesso!\n");
}

void obterDataHora(char *buffer, int maxLen) {
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    strftime(buffer, maxLen, "%d-%m-%Y %H:%M", t);
}

void registrarExtrato(Investidor inv, char *detalhes, double valorOp, double ct, double tx) {
    char filename[30];
    sprintf(filename, "extrato_%s.txt", inv.cpf);
    
    FILE *check = fopen(filename, "r");
    int novoArquivo = (check == NULL);
    if (check) fclose(check);

    FILE *f = fopen(filename, "a");
    if (!f) return;

    if (novoArquivo) {
        fprintf(f, "Nome: %s\nCPF: %s\n\n", inv.nome, inv.cpf);
    }

    char dataHora[20];
    obterDataHora(dataHora, sizeof(dataHora));

    fprintf(f, "%s %s %.2f CT: %.2f TX: %.2f REAL: %.2f BTC: %.4f ETH: %.4f XRP: %.4f\n",
            dataHora, detalhes, valorOp, ct, tx, inv.reais, inv.bitcoin, inv.ethereum, inv.ripple);
    fclose(f);
}

void consultarExtrato(char *cpf) {
    char filename[30];
    sprintf(filename, "extrato_%s.txt", cpf);
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("\nNenhum registo de extrato encontrado para este investidor.\n");
        return;
    }
    printf("\n--- EXTRATO DE OPERACOES ---\n");
    char linha[256];
    while (fgets(linha, sizeof(linha), f)) {
        printf("%s", linha);
    }
    fclose(f);
}

void atualizarCotacoes(Cotacao *cot) {
    double varBtc = ((double)rand() / RAND_MAX) * 0.10 - 0.05;
    double varEth = ((double)rand() / RAND_MAX) * 0.10 - 0.05;
    double varXrp = ((double)rand() / RAND_MAX) * 0.10 - 0.05;

    cot->bitcoin += cot->bitcoin * varBtc;
    cot->ethereum += cot->ethereum * varEth;
    cot->ripple += cot->ripple * varXrp;

    salvarCotacoes(*cot);
    printf("\n[Mercado] Cotacoes atualizadas com sucesso!\n");
    printf("Novo Bitcoin: R$ %.2f\n", cot->bitcoin);
    printf("Novo Ethereum: R$ %.2f\n", cot->ethereum);
    printf("Novo Ripple: R$ %.2f\n", cot->ripple);
}

// --- MENU ADMINISTRADOR ---
void menuAdmin(Investidor invs[], int *totalInvs, Cotacao *cot) {
    int opcao;
    do {
        printf("\n----------------------------------------\n");
        printf("           MENU ADMINISTRADOR           \n");
        printf("----------------------------------------\n");
        printf("1. Cadastrar novo investidor\n");
        printf("2. Excluir investidor\n");
        printf("3. Consultar saldo de um investidor\n");
        printf("4. Consultar extrato de um investidor\n");
        printf("5. Atualizar cotacao das criptomoedas\n");
        printf("6. Sair do Painel Admin\n");
        printf("----------------------------------------\n");
        printf("Escolha uma opcao: ");
        
        if (scanf("%d", &opcao) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch (opcao) {
            case 1:
                cadastrarNovoInvestidor(invs, totalInvs);
                break;
            case 2: {
                char cpf_busca[12];
                printf("Digite o CPF do investidor a ser excluido: ");
                scanf("%11s", cpf_busca);

                int idx = buscarInvestidorPorCPF(invs, *totalInvs, cpf_busca);
                if (idx == -1) {
                    printf("\n[Erro] Investidor nao encontrado!\n");
                } else {
                    printf("\nInvestidor Encontrado:\nNome: %s | CPF: %s\n", invs[idx].nome, invs[idx].cpf);
                    char confirmacao;
                    printf("Deseja realmente excluir? (S/N): ");
                    scanf(" %c", &confirmacao);

                    if (confirmacao == 'S' || confirmacao == 's') {
                        for (int i = idx; i < *totalInvs - 1; i++) {
                            invs[i] = invs[i + 1];
                        }
                        (*totalInvs)--;
                        salvarInvestidores(invs, *totalInvs);
                        printf("\n[Sucesso] Investidor excluido com sucesso!\n");
                    } else {
                        printf("\nExclusao cancelada.\n");
                    }
                }
                break;
            }
            case 3: {
                char cpf_busca[12];
                printf("Digite o CPF do investidor: ");
                scanf("%11s", cpf_busca);

                int idx = buscarInvestidorPorCPF(invs, *totalInvs, cpf_busca);
                if (idx == -1) {
                    printf("\n[Erro] Investidor nao encontrado!\n");
                } else {
                    printf("\n--- SALDO DO INVESTIDOR (ADMIN) ---\n");
                    printf("Nome: %s\n", invs[idx].nome);
                    printf("CPF: %s\n", invs[idx].cpf);
                    printf("Reais: R$ %.2f\n", invs[idx].reais);
                    printf("Bitcoin: %.4f\n", invs[idx].bitcoin);
                    printf("Ethereum: %.4f\n", invs[idx].ethereum);
                    printf("Ripple: %.4f\n", invs[idx].ripple);
                }
                break;
            }
            case 4: {
                char cpf_busca[12];
                printf("Digite o CPF do investidor: ");
                scanf("%11s", cpf_busca);

                int idx = buscarInvestidorPorCPF(invs, *totalInvs, cpf_busca);
                if (idx == -1) {
                    printf("\n[Erro] Investidor nao encontrado!\n");
                } else {
                    consultarExtrato(cpf_busca);
                }
                break;
            }
            case 5:
                atualizarCotacoes(cot);
                break;
            case 6:
                printf("\nSaindo do painel administrativo...\n");
                break;
            default:
                printf("\n[Erro] Opcao invalida!\n");
                break;
        }
    } while (opcao != 6);
}

// --- MENU INVESTIDOR ---
void menuInvestidor(Investidor *inv, Cotacao *cot, Investidor invs[], int totalInvs) {
    int opcao;
    char senhavalidada[7];

    do {
        printf("\n    PAINEL DO INVESTIDOR: %s\n", inv->nome);
        printf("1. Consultar saldo\n");
        printf("2. Consultar extrato de operacoes\n");
        printf("3. Depositar reais\n");
        printf("4. Sacar reais\n");
        printf("5. Comprar criptomoedas\n");
        printf("6. Vender criptomoedas\n");
        printf("7. Atualizar cotacao\n");
        printf("8. Sair (Logout)\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) {
            while(getchar() != '\n');
            continue;
        }

        if (opcao == 1 || opcao == 2 || opcao == 5 || opcao == 6) {
            printf("Confirme sua senha numerica de 6 digitos: ");
            scanf("%6s", senhavalidada);

            if (strcmp(inv->senha, senhavalidada) != 0) {
                printf("\nAcesso restrito para a operacao. Senha incorreta!\n");
                continue;
            }
        }

        switch (opcao) {
            case 1:
                printf("\n--- SALDO ATUAL ---\n");
                printf("Nome: %s\n", inv->nome);
                printf("CPF: %s\n", inv->cpf);
                printf("Reais: R$ %.2f\n", inv->reais);
                printf("Bitcoin: %.4f\n", inv->bitcoin);
                printf("Ethereum: %.4f\n", inv->ethereum);
                printf("Ripple: %.4f\n", inv->ripple);
                break;

            case 2:
                consultarExtrato(inv->cpf);
                break;

            case 3: {
                double valordeposito;
                printf("Informe o valor em R$ para depositar: ");
                scanf("%lf", &valordeposito);

                if (valordeposito <= 0) {
                    printf("\nO valor do deposito deve ser maior que zero.\n");
                } else {
                    inv->reais += valordeposito;
                    registrarExtrato(*inv, "+ REAL", valordeposito, 0.0, 0.0);
                    salvarInvestidores(invs, totalInvs);
                    printf("\nDeposito efetuado! Novo saldo em Reais: R$ %.2f\n", inv->reais);
                }
                break;
            }

            case 4: {
                double valorsaque;
                printf("Informe o valor que deseja sacar (R$): ");
                scanf("%lf", &valorsaque);
                if (valorsaque <= 0 || valorsaque > inv->reais) {
                    printf("\nValor invalido ou saldo insuficiente em Reais.\n");
                } else {
                    inv->reais -= valorsaque;
                    registrarExtrato(*inv, "- REAL", valorsaque, 0.0, 0.0);
                    salvarInvestidores(invs, totalInvs);
                    printf("\nSaque efetuado! Novo saldo em Reais: R$ %.2f\n", inv->reais);
                }
                break;
            }

            case 5: {
                double qtd;
                int Cripto;
                double custobruto, taxa, custoTotal;

                printf("\n--- COTACOES DE COMPRA ---\n");
                printf("1. Bitcoin (BTC) - R$ %.2f (Taxa: 2%%)\n", cot->bitcoin);
                printf("2. Ethereum (ETH) - R$ %.2f (Taxa: 1%%)\n", cot->ethereum);
                printf("3. Ripple (XRP) - R$ %.2f (Taxa: 1%%)\n", cot->ripple);

                printf("Escolha a criptomoeda (1-Bitcoin, 2-Ethereum, 3-Ripple): ");
                scanf("%d", &Cripto);

                printf("Informe a quantidade de compra: ");
                scanf("%lf", &qtd);

                if (qtd <= 0) {
                    printf("\nQuantidade invalida.\n");
                    break;
                }

                switch (Cripto) {
                    case 1: 
                        custobruto = qtd * cot->bitcoin;
                        taxa = custobruto * 0.02;
                        custoTotal = custobruto + taxa;
                        if (custoTotal > inv->reais) {
                            printf("\nSaldo insuficiente. Necessario: R$ %.2f\n", custoTotal);
                        } else {
                            inv->reais -= custoTotal;
                            inv->bitcoin += qtd;
                            registrarExtrato(*inv, "+ BTC", custobruto, cot->bitcoin, taxa);
                            salvarInvestidores(invs, totalInvs);
                            printf("\nCompra de Bitcoin realizada com sucesso!\n");
                        }
                        break;

                    case 2: 
                        custobruto = qtd * cot->ethereum;
                        taxa = custobruto * 0.01;
                        custoTotal = custobruto + taxa;
                        if (custoTotal > inv->reais) {
                            printf("\nSaldo insuficiente. Necessario: R$ %.2f\n", custoTotal);
                        } else {
                            inv->reais -= custoTotal;
                            inv->ethereum += qtd;
                            registrarExtrato(*inv, "+ ETH", custobruto, cot->ethereum, taxa);
                            salvarInvestidores(invs, totalInvs);
                            printf("\nCompra de Ethereum realizada com sucesso!\n");
                        }
                        break;

                    case 3: 
                        custobruto = qtd * cot->ripple;
                        taxa = custobruto * 0.01;
                        custoTotal = custobruto + taxa;
                        if (custoTotal > inv->reais) {
                            printf("\nSaldo insuficiente. Necessario: R$ %.2f\n", custoTotal);
                        } else {
                            inv->reais -= custoTotal;
                            inv->ripple += qtd;
                            registrarExtrato(*inv, "+ XRP", custobruto, cot->ripple, taxa);
                            salvarInvestidores(invs, totalInvs);
                            printf("\nCompra de Ripple realizada com sucesso!\n");
                        }
                        break;

                    default:
                        printf("\nCriptomoeda invalida.\n");
                        break;
                }
                break;
            }

            case 6: {
                int escolhaCripto;
                double qtd, valorbruto, taxa, valorliquido;

                printf("\n--- COTACOES DE VENDA ---\n");
                printf("1. Bitcoin (BTC)  - R$ %.2f (Taxa: 3%%)\n", cot->bitcoin);
                printf("2. Ethereum (ETH) - R$ %.2f (Taxa: 2%%)\n", cot->ethereum);
                printf("3. Ripple (XRP) - R$ %.2f (Taxa: 1%%)\n", cot->ripple);

                printf("Escolha a criptomoeda (1-Bitcoin, 2-Ethereum, 3-Ripple): ");
                scanf("%d", &escolhaCripto);

                printf("Informe a quantidade que deseja vender: ");
                scanf("%lf", &qtd);

                if (qtd <= 0) {
                    printf("\nQuantidade invalida.\n");
                    break;
                }
                
                switch (escolhaCripto) {
                    case 1: 
                        if (qtd > inv->bitcoin) {
                            printf("\nQuantidade insuficiente de Bitcoin.\n");
                        } else {
                            valorbruto = qtd * cot->bitcoin;
                            taxa = valorbruto * 0.03;
                            valorliquido = valorbruto - taxa;
                            inv->bitcoin -= qtd;
                            inv->reais += valorliquido;
                            registrarExtrato(*inv, "- BTC", valorbruto, cot->bitcoin, taxa);
                            salvarInvestidores(invs, totalInvs);
                            printf("\nVenda de Bitcoin realizada com sucesso!\n");
                        }
                        break;

                    case 2: 
                        if (qtd > inv->ethereum) {
                            printf("\nQuantidade insuficiente de Ethereum.\n");
                        } else {
                            valorbruto = qtd * cot->ethereum;
                            taxa = valorbruto * 0.02;
                            valorliquido = valorbruto - taxa;
                            inv->ethereum -= qtd;
                            inv->reais += valorliquido;
                            registrarExtrato(*inv, "- ETH", valorbruto, cot->ethereum, taxa);
                            salvarInvestidores(invs, totalInvs);
                            printf("\nVenda de Ethereum realizada com sucesso!\n");
                        }
                        break;

                    case 3: 
                        if (qtd > inv->ripple) {
                            printf("\nQuantidade insuficiente de Ripple.\n");
                        } else {
                            valorbruto = qtd * cot->ripple;
                            taxa = valorbruto * 0.01;
                            valorliquido = valorbruto - taxa;
                            inv->ripple -= qtd;
                            inv->reais += valorliquido;
                            registrarExtrato(*inv, "- XRP", valorbruto, cot->ripple, taxa);
                            salvarInvestidores(invs, totalInvs);
                            printf("\nVenda de Ripple realizada com sucesso!\n");
                        }
                        break;

                    default:
                        printf("\nOpcao de criptomoeda invalida.\n");
                        break;
                }
                break;
            }

            case 7: 
                atualizarCotacoes(cot);
                break;

            case 8:
                printf("\nA terminar sessao do investidor...\n");
                break;

            default:
                printf("\nOpcao invalida. Escolha entre 1 e 8.\n");
                break;
        }
    } while (opcao != 8);
}