# Sistemas customizados do rAthena Comercio

Este documento resume a organização dos sistemas customizados incluídos na distribuição Linux. Ele serve como mapa de manutenção para o administrador do host e não substitui os comentários presentes nos fontes, scripts ou arquivos de configuração.

## AutoAttack

O AutoAttack permite que o próprio personagem faça farming automático enquanto o módulo rental estiver disponível. O ponto de entrada do menu é `npc/custom/autoattack_menu.txt`, o núcleo está em `src/map/autoattack.cpp` e `src/map/autoattack_script.cpp`, e a persistência é definida em `sql-files/autoattack.sql`.

O menu foi estruturado para separar configuração, habilidades de ataque, buffs, poções, itens, seleção de monstros e outras opções. A lista de habilidades deve ser filtrada pelas skills aprendidas pelo personagem nas árvores de classe suportadas. A seleção de monstros utiliza os monstros disponíveis no mapa atual. As regras de duração, mapas permitidos e timer devem ser conferidas em `conf/battle/feature.conf` e nos arquivos de configuração importados.

O item principal é o módulo rental 56330, recebido pelas caixas configuradas nos bancos de itens. Importe a tabela SQL antes de testar recursos que dependem de persistência.

## Autofarm

O Autofarm é mantido como sistema separado do AutoAttack. Seus fontes ficam em `src/map/autofarm.cpp` e `src/map/autofarm.hpp`; o menu está em `npc/custom/autofarm_menu.txt`, a configuração em `conf/autofarm.conf` e as recompensas persistentes em `sql-files/autofarm_rewards.sql`.

Não habilite ou teste AutoAttack e Autofarm simultaneamente sem confirmar que os itens, status, comandos e tabelas SQL não estão compartilhando IDs ou nomes de funções indevidamente.

## Population/FakePlayer

O Population Engine contém o núcleo em `src/map/population_engine.cpp`, módulos auxiliares no diretório `src/map/population_engine/`, bancos YAML em `db/population_*.yml` e configuração principal em `conf/battle/population_engine.conf`.

O sistema pode criar personagens artificiais com perfis de comportamento, combate, navegação, chat e vendedores, conforme os bancos e flags habilitados. A quantidade de população, frequência de IA e número de mapas ativos devem ser ajustados gradualmente e monitorados pelo consumo de CPU, memória e tamanho dos logs.

Quando o painel ou estatísticas externas forem utilizadas, confira `sql-files/population_engine/cp_population_stats.sql` antes de importar a tabela.

## Season/CustomRate

O sistema de temporada utiliza `src/map/customrate.cpp`, `src/map/customrate.hpp`, `db/custom/customrate_db.yml`, `conf/battle/customrate.conf` e os scripts em `npc/custom/season/`. O fluxo de login e a escolha de temporada devem ser testados com um personagem novo e com um personagem que já tenha a temporada registrada, pois esses dois casos percorrem caminhos diferentes.

Mantenha recompensas, scripts de início e scripts de encerramento documentados no banco customrate. Evite executar scripts de recompensa antes de confirmar a existência de todos os itens citados.

## Hunting Missions

As missões de caça estão em `npc/custom/quests/hunting_missions.txt`. As missões são configuradas por faixas e listas de monstros, e a loja de recompensas deve usar os arquivos de barter definidos em `npc/custom` ou nos caminhos de importação correspondentes.

Antes de publicar, verifique se os monstros escolhidos pertencem a mapas normais, se MVPs e monstros especiais estão excluídos conforme a regra do projeto e se o item moeda existe no banco de itens.

## Idiomas dos scripts

Os diálogos multilíngues utilizam `#langtype` e funções auxiliares em `npc/custom/functions/langtype_dialog.txt`. Os scripts customizados podem selecionar inglês, espanhol ou português brasileiro de acordo com a configuração do personagem.

No Linux, não converta automaticamente todos os scripts para UTF-8 sem testar. A codificação usada pelo parser precisa ser preservada para que os acentos sejam exibidos corretamente no cliente e para que o carregamento dos NPCs não falhe.

## Processo de ativação recomendado

A forma mais segura de publicar esta distribuição é ativar os sistemas em etapas. Primeiro compile a base e inicie os três servidores principais. Depois importe e teste o SQL do AutoAttack, valide o item e o menu, e só então habilite Population/FakePlayer ou Season/CustomRate. Registre a revisão, o commit e as alterações de configuração em cada etapa.
