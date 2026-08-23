# rAthena Comercio

**Projeto de JokerSama — distribuição Linux do emulador rAthena personalizado para o projeto Comercio.**

Este repositório reúne a base [rAthena](https://github.com/rathena/rathena) com os sistemas customizados, NPCs, bancos, configurações e adaptações desenvolvidos para este projeto. A distribuição contém somente a árvore-fonte do servidor e os arquivos necessários para compilação em Linux. Executáveis Windows, DLLs, símbolos de depuração, logs de desenvolvimento, backups locais, arquivos do cliente e GRFs não fazem parte deste repositório.

> Este é um projeto de JokerSama. Os créditos e licenças da base rAthena e de componentes de terceiros continuam válidos e devem ser preservados.

**Repositório do projeto:** [github.com/JokerSamaBR/new4themu](https://github.com/JokerSamaBR/new4themu)  
**Base:** [rAthena oficial](https://github.com/rathena/rathena)  
**Documentação da base:** [Wiki oficial](https://github.com/rathena/rathena/wiki)  
**Comunidade:** [Fórum rAthena](https://rathena.org/board)

## Sumário

1. [Objetivo e escopo](#1-objetivo-e-escopo)
2. [Sistemas incluídos](#2-sistemas-incluídos)
3. [Comandos importantes](#3-comandos-importantes)
4. [Status icons](#4-status-icons)
5. [Requisitos](#5-requisitos)
6. [Instalação em Linux](#6-instalação-em-linux)
7. [Configuração inicial](#7-configuração-inicial)
8. [Banco de dados SQL](#8-banco-de-dados-sql)
9. [Execução dos servidores](#9-execução-dos-servidores)
10. [Estrutura das personalizações](#10-estrutura-das-personalizações)
11. [Solução de problemas](#11-solução-de-problemas)
12. [Créditos, licenças e autoria](#12-créditos-licenças-e-autoria)

## 1. Objetivo e escopo

O `rathena comercio` é uma distribuição-fonte para administradores que desejam compilar o próprio servidor em um host Linux. A árvore foi preparada a partir do emulador local de JokerSama e mantém as customizações aplicadas no servidor atual, organizadas preferencialmente em `src/custom`, `db/import`, `db/custom`, `conf/import`, `conf/msg_conf` e `npc/custom`.

A distribuição não inclui o patch do cliente. Os arquivos `stateicon`, TGA, Lua, `System`, `SystemEN`, GRF, executável Ragexe e demais arquivos do cliente devem ser mantidos e distribuídos separadamente pelo administrador do servidor.

A pasta local `new4th_Backup31122025` foi usada como referência durante algumas adaptações, mas não faz parte do projeto e não é sobrescrita por este repositório.

## 2. Sistemas incluídos

Os sistemas abaixo estão presentes na árvore do projeto. A ativação de cada sistema depende dos arquivos de configuração e dos NPCs incluídos em `npc/scripts_custom.conf`.

| Sistema | O que faz | Como usar ou ativar | Arquivos principais |
|---|---|---|---|
| **AutoAttack** | Permite configurar ataque básico, uso de habilidades, seleção de alvos, poções, filtros e persistência do modo automático. O sistema também possui integração com o modo AFK. | Use o item de acesso configurado, abra o menu e ajuste as opções. O item padrão da configuração atual é o ID `56330`. | `src/map/autoattack*`, `npc/custom/autoattack_menu.txt`, `sql-files/autoattack.sql`, configurações `aa_*` |
| **AutoAttack AFK** | Permite manter o personagem no mapa enquanto o jogador se desconecta, conforme as regras do sistema AFK. | Ative o AutoAttack e use `@afk` quando o personagem estiver preparado. Teste primeiro em ambiente controlado. | `src/map/autoattack*`, `src/map/afk*` quando presente, `conf` e `npc/custom` relacionados |
| **Population Engine / Fake Players** | Cria personagens artificiais com perfis, comportamento de cidade e campo, navegação, combate, seleção de habilidades e chat de proximidade. | Administradores usam `@populate`, `@reloadpopenginedb` e as configurações em `conf/battle/population_engine.conf`. | `src/map/population_engine*`, `db/population_*.yml`, `npc/custom/population`, `conf/battle/population_engine.conf` |
| **Season / CustomRate** | Permite escolher uma taxa individual por personagem, aplicar duração de temporada, entregar recompensas, executar scripts de marco e restaurar a taxa normal ao terminar. | O personagem escolhe pelo NPC de temporada. Administradores usam `@season`, `@season reload` e `@season reset`. | `src/map/customrate*`, `db/custom/customrate_db.yml`, `npc/custom/season`, configurações `customrate_*` |
| **Hunting Missions** | Entrega missões de caça com quatro monstros diferentes, selecionados por faixa de nível, recompensas configuráveis e loja de troca por barter. | Fale com o NPC de Hunting Missions. Para localizar um monstro, use `@whereis <nome>` quando o comando estiver disponível no grupo da conta. | `npc/custom/quests/hunting_missions.txt`, `npc/re/merchants/barters`, bancos e imports relacionados |
| **ModPk / Individual PvP Mode** | Ativa um modo PvP individual por mapa, com mapas protegidos ou forçados, efeitos visuais, status icon, restrições de teleporte, relações de party/guild/alliance, regras especiais de dano, EXP/drop e penalidades configuráveis. | Use `@pvpmode` em um mapa permitido. Os mapas são definidos em `npc/custom/modpk/map_flag.txt`. | `src/map/pc.cpp`, `src/map/clif.cpp`, `src/map/battle.cpp`, `src/map/atcommand.cpp`, `src/map/map.hpp`, `npc/custom/modpk/map_flag.txt` |
| **Idiomas do servidor** | Mantém mensagens e scripts preparados para Inglês, Espanhol e Português Brasileiro. O map-server atual usa `LANG_ENABLE=0x082` para SPN e POR, além do Inglês padrão. | Use `@langtype eng`, `@langtype spn` ou `@langtype por`. | `src/custom/defines_pre.hpp`, `conf/msg_conf`, `npc/custom/functions/langtype_dialog.txt` e scripts multilíngues |
| **Status Icons de sistemas** | Liga status reais do servidor aos EFSTs do cliente para mostrar AutoAttack, VIP, temporada e ModPk com tempo e descrição. | O servidor precisa ser recompilado após alterações em `src`; o cliente precisa usar uma GRF com os Lua e TGA corretos. | `src/map/status.hpp`, `src/map/script_constants.hpp`, `db/import/status.yml`, `npc/custom/status_icons`, arquivos do cliente fora deste repositório |
| **NPCs e customizações Comercio** | Reúne NPCs, quests, lojas, diálogos e ajustes específicos do projeto. | Ative ou desative cada script em `npc/scripts_custom.conf`. | `npc/custom`, `npc/scripts_custom.conf`, `conf/import` |

### Sistemas não incluídos como ativos

O Autofarm legado baseado em clones não deve ser considerado ativo apenas porque existem arquivos de código na árvore. O menu legado permanece desativado durante a migração. O sistema CraftTrees não faz parte desta distribuição atual. Não ative esses componentes sem adaptar e testar todos os arquivos correspondentes.

## 3. Comandos importantes

Os comandos dependem do grupo da conta configurado em `conf/groups.yml` e em `conf/atcommands.yml`. Um comando pode existir no código e ainda assim não estar liberado para jogadores comuns.

| Comando | Parâmetros | Função |
|---|---|---|
| `@pvpmode` | nenhum | Alterna o PvP individual do personagem, respeitando mapas protegidos e regras da ModPk. |
| `@afk` | nenhum | Entra no modo AFK integrado ao AutoAttack e desconecta mantendo o personagem conforme a implementação do sistema. |
| `@populate` | `<quantidade> [mapa]` | Cria shells do Population Engine em tempo de execução. |
| `@populate stop` | nenhum | Para a população criada em runtime. |
| `@populate stats` | nenhum | Mostra estatísticas da população. |
| `@populate status` | nenhum | Mostra o estado do Population Engine. |
| `@reloadpopenginedb` | nenhum | Recarrega os bancos YAML do Population Engine sem reiniciar o processo, quando permitido pela configuração. |
| `@season` | nenhum | Mostra a temporada/rate individual atual. |
| `@season reload` | nenhum | Recarrega o banco de temporadas, conforme a implementação ativa. |
| `@season reset` | nenhum | Remove a temporada individual do personagem, conforme o grupo de acesso. |
| `@rates` | nenhum | Mostra as taxas efetivas exibidas pelo servidor, incluindo o efeito da temporada quando aplicável. |
| `@langtype` | `<idioma>` | Altera o idioma de mensagens do personagem. Use `eng`, `spn` ou `por`. |
| `@whereis` | `<monstro>` | Mostra mapas onde o monstro aparece, quando disponível para o grupo da conta. |
| `@go` / `@warp` / `@rura` / `@mapmove` | depende do comando | Podem ser bloqueados pela ModPk em mapas ou condições configuradas. |

Use `@commands` ou `@help <comando>` para verificar a permissão e a descrição carregada pelo map-server. Não libere comandos administrativos para jogadores comuns.

## 4. Status icons

O método oficial usado neste projeto é:

```text
SC_* real no servidor
        -> db/import/status.yml
        -> EFST_* do cliente
        -> stateiconimginfo.lub e stateiconinfo.lub
        -> arquivo .tga dentro da GRF
```

O `db/custom/statusicon_db.yml` não é o mecanismo principal desta versão. O ícone só aparece quando todas as partes da cadeia estão presentes e o Ragexe está lendo a GRF correta.

| Sistema | Status do servidor | EFST | Arquivo visual esperado |
|---|---|---|---|
| AutoAttack | `SC_AUTOATTACK` | `EFST_AUTOATTACK` | `autoataque.tga` |
| VIP | `SC_VIPSTATE` | `EFST_VIPSTATE` | `vip_system.tga` |
| ModPk | `SC_PVPMODE` | `EFST_PK_ICON` | `pk_icon.tga` |
| Temporada 1x temporária | `SC_SEASON_1X_TEMP` | `EFST_SEASON_1X_TEMP` | `xp1xtemp.tga` |
| Temporada normal 1x | `SC_SEASON_1X` | `EFST_SEASON_1X` | `xp1x.tga` |
| Temporada 25x | `SC_SEASON_25X` | `EFST_SEASON_25X` | `xp25x.tga` |
| Temporada 50x | `SC_SEASON_50X` | `EFST_SEASON_50X` | `xp50x.tga` |
| Temporada 100x | `SC_SEASON_100X` | `EFST_SEASON_100X` | `xp100x.tga` |
| Temporada 200x reservado | `SC_SEASON_200X` | `EFST_SEASON_200X` | `xp200x.tga` |

Para criar um novo ícone, adicione um `SC_*` no final de `src/map/status.hpp`, exporte-o em `src/map/script_constants.hpp` se for usado por scripts, acrescente o vínculo em `db/import/status.yml`, registre o mesmo número EFST no cliente, ligue o EFST ao TGA em `stateiconimginfo.lub` e configure a descrição em `stateiconinfo.lub`. Depois recompile o map-server e reempacote o cliente.

A duração real é controlada pelo servidor usando `sc_start` ou `status_change_start`. A configuração Lua do cliente apenas apresenta o contador e a descrição; ela não substitui o timer do servidor.

O tutorial detalhado fica no pacote de desenvolvimento local de JokerSama, em `Packs_systemas_Mods_Npc/Mods/StatusIcons/README_StatusIcons_Metodo_ModPk.md`. Essa pasta de pacotes não é incluída nesta distribuição Linux para manter a árvore do emulador limpa.

## 5. Requisitos

A distribuição deve ser compilada no host Linux onde o servidor será executado. Os requisitos variam conforme a distribuição e a quantidade de jogadores, NPCs e Fake Players.

| Recurso | Para testes | Para produção inicial |
|---|---:|---:|
| CPU | 2 vCPUs | 4 ou mais vCPUs |
| Memória | 2 GB | 4 GB ou mais |
| Armazenamento | 2 GB livres | SSD com espaço para logs e banco |
| Sistema | Linux 64-bit atualizado | Distribuição LTS atualizada |
| Banco | MariaDB/MySQL compatível | MariaDB/MySQL com backup automático |
| Compilador | GCC ou Clang com C++17 | GCC/Clang atualizados |
| Ferramentas | Git, CMake, Make, pkg-config | As mesmas, com bibliotecas de desenvolvimento instaladas |

Em Debian ou Ubuntu, instale inicialmente as ferramentas de compilação, Git, CMake, pkg-config, zlib, PCRE e os headers do MariaDB/MySQL. Os nomes dos pacotes podem variar entre versões da distribuição.

## 6. Instalação em Linux

Clone o repositório privado ou copie a árvore para o host Linux:

```bash
git clone https://github.com/JokerSamaBR/new4themu.git rathena-comercio
cd rathena-comercio
```

Se o repositório for privado, configure uma chave SSH ou autenticação do GitHub antes do clone. Não coloque tokens dentro de scripts, arquivos de configuração ou commits.

Instale as dependências da distribuição. Em Debian/Ubuntu, um ponto de partida é:

```bash
sudo apt update
sudo apt install -y build-essential cmake git pkg-config \
  libmariadb-dev libpcre3-dev zlib1g-dev
```

Compile fora da árvore de fontes:

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel "$(nproc)"
```

Em um host com pouca memória, reduza o paralelismo:

```bash
cmake --build . --parallel 2
```

A base também possui fluxo Autotools. Use-o somente se for o padrão adotado pelo administrador:

```bash
./configure
make -j"$(nproc)"
```

Não compile ou execute os servidores como `root`. Use um usuário próprio e mantenha os logs fora da árvore pública quando possível.

## 7. Configuração inicial

Antes do primeiro start, configure as conexões com o banco, os endereços, as portas, os nomes dos servidores, os grupos de comandos e as permissões em `conf/`. Os arquivos de importação devem ser revisados antes de entrar em produção.

As principais áreas de configuração são:

| Área | Arquivos | O que revisar |
|---|---|---|
| Banco e servidores | `conf/` | IPs, portas, credenciais, nomes e permissões. |
| Rates gerais | `conf/battle/` | EXP, drop, zeny e regras gerais. |
| Population Engine | `conf/battle/population_engine.conf`, `db/population_*.yml` | Quantidade, intervalos, mapas, IA, combate, chat e limites por ciclo. |
| CustomRate | `db/custom/customrate_db.yml`, configurações `customrate_*` | Rates, duração, recompensas, scripts e efeitos. |
| ModPk | `npc/custom/modpk/map_flag.txt`, `conf/battle/misc.conf` | Mapas bloqueados/forçados, dano, EXP/drop, FLEE, warps e penalidades. |
| NPCs | `npc/scripts_custom.conf` | Scripts que serão carregados ou mantidos comentados. |
| Idiomas | `src/custom/defines_pre.hpp`, `conf/msg_conf` | `LANG_ENABLE=0x082` e imports ENG/SPN/POR. |
| Status icons | `db/import/status.yml` | Status `SC_*` e EFSTs correspondentes. |

Para manter a árvore limpa, não copie backups, logs de compilação, executáveis Windows ou arquivos do patch do cliente para este repositório.

## 8. Banco de dados SQL

Crie os bancos `login`, `char` e `map` conforme a documentação da base rAthena. Importe primeiro as tabelas oficiais e depois somente os arquivos customizados dos sistemas que serão utilizados.

Exemplos:

```bash
mysql -u usuario -p nome_do_banco < sql-files/autoattack.sql
mysql -u usuario -p nome_do_banco < sql-files/autofarm_rewards.sql
```

| Arquivo | Uso |
|---|---|
| `sql-files/autoattack.sql` | Persistência do AutoAttack. |
| `sql-files/autofarm_rewards.sql` | Persistência do Autofarm legado, somente se esse sistema for reativado. |
| `sql-files/population_engine/cp_population_stats.sql` | Estatísticas auxiliares do Population Engine ou painel, quando utilizadas. |

Os IDs de itens customizados precisam existir nos bancos de importação do servidor. Não substitua o `item_db` oficial inteiro para adicionar itens do projeto; use `db/import` ou `db/custom` conforme a estrutura da árvore.

Faça backup do banco antes de qualquer importação. A manutenção do MySQL/MariaDB deve ser feita pelo administrador do host; este repositório não contém acesso ao banco de produção.

## 9. Execução dos servidores

Após a compilação, execute os serviços na ordem tradicional:

```bash
./login-server
./char-server
./map-server
```

Em produção, use unidades `systemd` separadas ou outro supervisor de processos. Defina um usuário dedicado, diretório de trabalho, política de reinício e destino dos logs.

Exemplo conceitual de verificação dos processos:

```bash
ps aux | grep -E 'login-server|char-server|map-server'
```

Não inicie uma segunda instância do mesmo map-server na mesma porta. Antes de testar uma nova compilação, encerre a instância anterior e confirme que o executável usado pertence à árvore correta.

## 10. Estrutura das personalizações

| Diretório | Finalidade |
|---|---|
| `src/` | Código C/C++ da base e dos sistemas integrados. |
| `src/custom/` | Configurações e extensões personalizadas do projeto. |
| `conf/` | Configurações do login, char, map, mensagens e sistemas. |
| `db/` | Bancos oficiais e customizações YAML/TXT. |
| `db/import/` | Entradas adicionais sem substituir os bancos oficiais. |
| `db/custom/` | Bancos customizados específicos, como temporadas. |
| `npc/` | NPCs, quests, menus, warps e eventos. |
| `npc/custom/` | Scripts customizados do projeto Comercio. |
| `sql-files/` | Estruturas e migrações SQL. |
| `doc/` | Documentação da base. |
| `3rdparty/` | Dependências e código auxiliar usado pela compilação. |
| `tools/` | Ferramentas-fonte e scripts compatíveis com a distribuição. |
| `generated/` | Arquivos gerados necessários ao build. |

Os pacotes locais de desenvolvimento em `Packs_systemas_Mods_Npc` e os backups em `emuladorBackup` ficam fora desta árvore de distribuição.

## 11. Solução de problemas

Leia a mensagem completa do console e anote o arquivo e a linha indicados. Não tente esconder o erro removendo o log ou substituindo arquivos inteiros.

| Sintoma | Verificações iniciais |
|---|---|
| `Invalid Status` ou `Icon ... is invalid` | Confirme o `SC_*`, o `EFST_*`, as exportações e o `db/import/status.yml`. Recompile o map-server. |
| Ícone não aparece, mas o prompt está normal | Confira a GRF usada pelo Ragexe, a prioridade entre GRFs, o caminho do TGA e as entradas nos três `.lub` do cliente. |
| `Message ... not found for langtype 2` | Confirme `LANG_ENABLE=0x082`, os arquivos `map_msg_spn.conf` e `map_msg_por.conf` e use o map-server recompilado. |
| Erro de sintaxe em NPC | Confira o cabeçalho, os labels, as funções `callfunc`, a codificação e se o script está incluído em `npc/scripts_custom.conf`. |
| `No database Header was found` | Confirme o cabeçalho e o `Type` do YAML. Não misture formatos de bancos diferentes. |
| Item inexistente | Confirme se o item está em `db/import`/`db/custom` e se o ID está disponível no banco usado pelo servidor. |
| AutoAttack sem persistência | Importe `sql-files/autoattack.sql` e confirme a conexão SQL do map-server. |
| Population consumindo muita CPU | Reduza quantidade de shells, intervalos de IA, detecção de monstros, chat e limite por ciclo em `population_engine.conf`. |
| Servidor desconecta ao escolher temporada | Leia o console, confirme o `customrate_db.yml`, não use scripts duplicados de login e teste com o map-server recém-compilado. |

Após alterar qualquer arquivo em `src`, recompile os servidores afetados. Alterações apenas em NPC, YAML ou configuração normalmente exigem reiniciar o map-server, mas não substituem uma recompilação quando o código-fonte foi alterado.

## 12. Créditos, licenças e autoria

Este projeto é mantido por **JokerSama** e foi organizado para o projeto **Comercio**. Os comentários de autoria das adaptações customizadas utilizam `By: JokerSama` quando aplicável.

A base do servidor é o [rAthena](https://github.com/rathena/rathena). Mantenha o arquivo `LICENSE`, os avisos de copyright e os créditos presentes nos arquivos originais. Sistemas ou scripts de terceiros podem possuir licenças próprias; verifique cada arquivo antes de redistribuir ou modificar.

Este repositório não concede direitos sobre o cliente Ragnarok Online, GRFs, sprites, músicas, imagens, arquivos Lua proprietários ou outros componentes que não estejam sob uma licença compatível. O cliente e o patch devem ser tratados separadamente pelo administrador.

### Referências

[1]: https://github.com/rathena/rathena — Repositório oficial do rAthena.  
[2]: https://github.com/rathena/rathena/wiki — Wiki oficial do rAthena.  
[3]: https://rathena.org/board — Fórum da comunidade rAthena.  
[4]: https://github.com/JokerSamaBR/new4themu — Repositório privado do projeto rAthena Comercio de JokerSama.
