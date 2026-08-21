<img src="doc/logo.png" align="right" height="90" />

# rAthena Comercio

Distribuição de código-fonte do emulador rAthena personalizado para o projeto **Comercio**. Esta versão foi preparada para ser publicada em um host Linux e compilada pelo administrador a partir dos fontes, sem executáveis Windows, DLLs, símbolos de depuração, arquivos de Visual Studio, logs ou backups de trabalho.

> Este repositório combina a base rAthena com sistemas customizados de gameplay, NPCs, bancos e integrações desenvolvidos para este emulador. A pasta `Pasta Manus/rathena - Copia` permanece como referência funcional local e não faz parte desta distribuição.

[Projeto rAthena](https://github.com/rathena/rathena) · [Wiki oficial](https://github.com/rathena/rathena/wiki) · [Fórum rAthena](https://rathena.org/board) · [Licença da base](LICENSE)

---

## Sumário

1. [Características desta distribuição](#1-características-desta-distribuição)
2. [Sistemas customizados](#2-sistemas-customizados)
3. [Requisitos](#3-requisitos)
4. [Instalação em Linux](#4-instalação-em-linux)
5. [Configuração inicial](#5-configuração-inicial)
6. [Banco de dados](#6-banco-de-dados)
7. [Execução dos servidores](#7-execução-dos-servidores)
8. [Estrutura do projeto](#8-estrutura-do-projeto)
9. [Solução de problemas](#9-solução-de-problemas)
10. [Publicação no GitHub](#10-publicação-no-github)
11. [Créditos e licença](#11-créditos-e-licença)

## 1. Características desta distribuição

O `rathena comercio` é uma distribuição **source-only para Linux**. O pacote contém os fontes C/C++, scripts NPC, bancos YAML, configurações, documentação, ferramentas de build e arquivos de persistência necessários para compilar e executar o emulador.

Os binários Windows, arquivos `.bat`, soluções do Visual Studio, DLLs, símbolos `.pdb`/`.ilk`, logs, dumps, backups e artefatos de validação foram removidos intencionalmente. Isso reduz o conteúdo da distribuição e evita publicar arquivos específicos da máquina de desenvolvimento.

Esta árvore foi derivada da versão local limpa do emulador e mantém a revisão-base do rAthena utilizada pelo projeto. O cliente do jogo, GRF, `data`, `System`, `SystemEN`, arquivos Lua e patch visual não estão incluídos nesta distribuição do servidor.

## 2. Sistemas customizados

A tabela abaixo resume os sistemas que fazem parte desta versão. Cada sistema deve ser testado separadamente antes de ser ativado em produção.

| Sistema | Descrição | Arquivos e áreas principais |
|---|---|---|
| **AutoAttack** | Permite que o próprio personagem faça farming automático mediante o item rental 56330. Inclui menu de configuração, habilidades aprendidas das árvores de 1ª, 2ª e 3ª classe, seleção de monstros do mapa atual, filtros de itens, poções, persistência e modo AFK. | `src/map/autoattack*`, `npc/custom/autoattack_menu.txt`, `sql-files/autoattack.sql` |
| **Autofarm** | Sistema separado de farming automático com menu, persistência de recompensas e itens de aluguel. | `src/map/autofarm*`, `npc/custom/autofarm_menu.txt`, `conf/autofarm.conf`, `sql-files/autofarm_rewards.sql` |
| **Population/FakePlayer** | Motor de personagens artificiais com perfis, comportamento de campo e dungeon, combate, navegação, chat de proximidade e configurações de população. | `src/map/population_engine*`, `db/population_*.yml`, `conf/battle/population_engine.conf`, `npc/custom/population` |
| **Season/CustomRate** | Sistema de temporadas e taxas customizadas com escolha de rate, scripts de login, recompensas e encerramento de temporada. | `src/map/customrate*`, `db/custom/customrate_db.yml`, `conf/battle/customrate.conf`, `npc/custom/season` |
| **Hunting Missions** | NPC de missões de caça com quatro monstros por missão, recompensas configuráveis e loja de troca por moeda através do sistema de barter. | `npc/custom/quests/hunting_missions.txt`, arquivos de barter e bancos relacionados |
| **Idiomas dos scripts** | Suporte utilizado pelos scripts customizados para inglês, espanhol e português brasileiro por meio de `#langtype`, além dos diálogos multilíngues do servidor. | `npc/custom/functions/langtype_dialog.txt`, scripts customizados |
| **Itens e integrações customizadas** | Itens de aluguel, módulos de AutoAttack, caixas de duração, status customizados, comandos e integrações de persistência SQL. | `db/import`, `db/custom`, `conf`, `src/map`, `sql-files` |

Os sistemas estão presentes na árvore para facilitar a implantação, mas isso não significa que todas as funções devam ser habilitadas simultaneamente. Ative e valide cada conjunto de NPCs, fontes, bancos e configurações de forma controlada.

### AutoAttack e banco SQL

Antes de testar a persistência do AutoAttack, importe manualmente `sql-files/autoattack.sql` no banco de dados utilizado pelo servidor. O item rental 56330 e as caixas 56329, 56331 e 56332 devem existir nos bancos de importação/customização do emulador. O modo AFK depende da integração correspondente no servidor e deve ser testado em um ambiente controlado.

### Population/FakePlayer e banco SQL

O Population Engine possui bancos YAML próprios e, quando utilizado com integração externa ou painel, pode exigir a importação de `sql-files/population_engine/cp_population_stats.sql`. Leia as configurações antes de aumentar a quantidade de personagens artificiais, pois população, navegação e combate podem aumentar o consumo de CPU e memória.

## 3. Requisitos

Os requisitos exatos dependem da distribuição Linux, do número de jogadores e da quantidade de sistemas ativados. Como ponto de partida, utilize uma distribuição Linux suportada pelo host, um compilador C++17, CMake, Make, Git, MariaDB/MySQL e as bibliotecas de desenvolvimento necessárias.

| Recurso | Mínimo para testes | Recomendado para produção |
|---|---:|---:|
| CPU | 2 vCPUs | 4 ou mais vCPUs |
| Memória | 2 GB | 4 GB ou mais |
| Armazenamento | 2 GB livres | SSD com espaço para logs e banco |
| Sistema | Linux 64-bit atualizado | Distribuição LTS atualizada |
| Banco | MariaDB/MySQL compatível | MariaDB/MySQL com backup automático |

Em Debian/Ubuntu, um ponto de partida comum é instalar `build-essential`, `cmake`, `git`, `pkg-config`, o cliente de desenvolvimento MariaDB/MySQL, PCRE e zlib. Os nomes exatos dos pacotes podem variar entre distribuições; confirme as dependências do host antes do build.

## 4. Instalação em Linux

Clone ou envie esta pasta para o host. Depois, entre no diretório do emulador:

```bash
cd /caminho/para/rathena-comercio
```

A compilação deve ser feita fora da árvore de fontes. O projeto CMake exige uma pasta de build separada por padrão:

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel "$(nproc)"
```

Em hosts com poucos recursos, reduza o paralelismo, por exemplo `--parallel 2`. Ao final, os executáveis serão gerados na árvore conforme as regras do CMake desta base. Confira os nomes e permissões antes de iniciar os serviços.

Como alternativa, utilize o fluxo Autotools fornecido pela base, caso seja o padrão adotado pelo seu host:

```bash
./configure
make -j"$(nproc)"
```

Não execute o build como `root`. Use um usuário próprio do servidor e conceda ao processo somente as permissões necessárias.

## 5. Configuração inicial

Antes do primeiro start, revise os arquivos de configuração em `conf/`, especialmente as conexões com o banco, portas, endereços de bind, nomes dos servidores, grupos de comandos e configurações de cada sistema customizado.

Os arquivos em `conf/import/`, `db/import/`, `db/custom/` e `npc/custom/` devem ser tratados como personalizações do projeto. Não substitua bancos oficiais inteiros para adicionar uma entrada customizada; prefira os arquivos de importação e customização já utilizados pela árvore.

Os scripts NPC que contêm português ou espanhol devem permanecer em uma codificação compatível com o parser configurado do servidor. Ao editar esses arquivos em Linux, preserve a codificação usada pelo projeto e valide o carregamento no console antes de colocar o servidor online.

## 6. Banco de dados

Crie previamente os bancos do login, char e map conforme a instalação padrão do rAthena. Importe as tabelas oficiais exigidas pela sua configuração e, depois, importe as tabelas dos sistemas customizados que serão utilizados.

Exemplo genérico para um arquivo SQL customizado:

```bash
mysql -u usuario -p nome_do_banco < sql-files/autoattack.sql
```

Os arquivos SQL customizados desta distribuição incluem:

| Arquivo | Finalidade |
|---|---|
| `sql-files/autoattack.sql` | Persistência do AutoAttack. |
| `sql-files/autofarm_rewards.sql` | Persistência das recompensas do Autofarm. |
| `sql-files/population_engine/cp_population_stats.sql` | Estatísticas auxiliares do Population Engine/painel, quando utilizadas. |

Faça backup do banco antes de importar ou atualizar tabelas. O agente que prepara esta distribuição não possui acesso ao seu banco de produção; a importação deve ser realizada pelo administrador do host.

## 7. Execução dos servidores

A ordem tradicional de inicialização é login, char e map. Execute os binários a partir da pasta do emulador ou configure serviços systemd separados para cada processo:

```bash
./login-server
./char-server
./map-server
```

O `web-server` somente deve ser iniciado se fizer parte da arquitetura do seu host. Em produção, prefira unidades systemd com usuário dedicado, diretório de trabalho definido, reinício controlado e logs encaminhados para o journal ou para uma pasta externa ao código-fonte.

Não publique senhas, tokens, dumps de banco, logs de produção ou arquivos de configuração com credenciais dentro de um repositório Git público.

## 8. Estrutura do projeto

| Diretório | Conteúdo |
|---|---|
| `src/` | Código C/C++ dos servidores e sistemas customizados. |
| `conf/` | Configurações do login, char, map e sistemas. |
| `db/` | Bancos oficiais e customizações YAML/TXT. |
| `npc/` | Scripts, NPCs, quests, warps e menus. |
| `sql-files/` | Scripts de criação e atualização de tabelas SQL. |
| `doc/` | Documentação técnica e exemplos da base. |
| `3rdparty/` | Dependências, módulos e arquivos auxiliares usados pelo build. |
| `tools/` | Ferramentas auxiliares; scripts específicos de Windows foram excluídos desta distribuição Linux. |
| `generated/` | Arquivos gerados mantidos pela árvore de fontes quando necessários ao build. |

O diretório `patchRO/Data` e os arquivos do cliente não fazem parte deste pacote. Eles devem ser distribuídos separadamente, de acordo com o patch e o cliente utilizados pelo servidor.

## 9. Solução de problemas

Quando um servidor não inicia ou desconecta, consulte primeiro a mensagem completa exibida no console. Erros de YAML, codificação, banco, porta ou script normalmente informam o arquivo e a linha que precisam ser corrigidos.

| Sintoma | Primeira verificação |
|---|---|
| Falha no build | Versão do compilador, CMake, dependências e saída completa do build. |
| Erro ao carregar NPC | Caminho importado, sintaxe, codificação e existência de funções chamadas por `callfunc`. |
| Tabela SQL ausente | Nome do banco, usuário, permissões e importação do arquivo correto. |
| Item customizado inexistente | Arquivo em `db/import` ou `db/custom`, ID duplicado e carregamento do item DB. |
| AutoAttack sem persistência | Importação de `sql-files/autoattack.sql` e conexão SQL do map-server. |
| Population consumindo muitos recursos | Quantidade de perfis ativos, intervalos de IA, mapas e logs de combate. |

Não apague os logs para esconder um erro. Guarde uma cópia fora da árvore do código e registre a revisão e a configuração usadas para reproduzir o problema.

## 10. Publicação no GitHub

Para publicar esta distribuição, crie um repositório novo e envie somente o conteúdo de `rathena comercio`. A pasta `.git` da referência local não foi incluída, permitindo que o repositório remoto tenha seu próprio histórico.

Exemplo:

```bash
cd rathena-comercio
git init
git add .
git commit -m "Initial Linux source distribution"
git branch -M main
git remote add origin https://github.com/SEU_USUARIO/SEU_REPOSITORIO.git
git push -u origin main
```

Antes do `git add`, revise os arquivos para garantir que não há senhas, IPs privados, tokens, dumps, logs ou dados pessoais. A base rAthena fornece orientações próprias de contribuição e licenciamento; este repositório deve manter os avisos de copyright existentes e documentar claramente as alterações customizadas.

## 11. Créditos e licença

Este projeto utiliza a base rAthena, um projeto de servidor MMORPG escrito em C++. Consulte o repositório oficial e o arquivo `LICENSE` incluído nesta distribuição para as condições aplicáveis à base.[1]

Os sistemas customizados, scripts e ajustes específicos do servidor Comercio pertencem ao projeto que os desenvolveu, salvo quando um arquivo indicar outra licença ou crédito. Antes de redistribuir componentes de terceiros, confirme a licença, os avisos de copyright e as regras de redistribuição correspondentes.

### Referências

[1]: https://github.com/rathena/rathena — Repositório oficial do rAthena.
[2]: https://github.com/rathena/rathena/wiki — Wiki oficial do rAthena.
[3]: https://rathena.org/board — Fórum oficial da comunidade rAthena.
