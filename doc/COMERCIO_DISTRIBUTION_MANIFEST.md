# Manifesto da distribuição Linux — rAthena Comercio

## Origem

Esta distribuição foi criada a partir de `Pasta Manus/rathena`, que por sua vez foi recriada preservando `Pasta Manus/rathena - Copia` como referência funcional. A referência original não foi modificada.

## Conteúdo mantido

Foram mantidos os fontes C/C++, CMake, Autotools, bancos, configurações, scripts NPC, documentação, ferramentas portáveis, arquivos de licença e os sistemas customizados do emulador Comercio.

Os sistemas identificados na árvore incluem AutoAttack, Autofarm, Population/FakePlayer, Season/CustomRate, Hunting Missions, diálogos multilíngues e suas integrações de itens, status e SQL.

## Conteúdo excluído

A distribuição não contém executáveis Windows (`.exe`), DLLs (`.dll`), símbolos de depuração (`.pdb`, `.ilk`), soluções e projetos do Visual Studio (`.sln`, `.vcxproj`), scripts Windows (`.bat`), scripts macOS (`.scpt`), bibliotecas Windows, logs, dumps, backups, snapshots, arquivos temporários ou relatórios de validação.

A pasta `.git` não foi incluída. O administrador pode inicializar um novo repositório Git e escolher a estratégia de histórico desejada.

## Verificação realizada

A árvore contém 5.203 arquivos e inclui `CMakeLists.txt`, `configure`, `configure.ac`, `Makefile.in`, `install.sh`, `athena-start`, `function.sh`, `src`, `conf`, `db`, `npc`, `sql-files` e `doc`.

A auditoria por extensões e padrões proibidos não encontrou arquivos Windows ou artefatos temporários na distribuição. O CMake não foi executado nesta máquina de desenvolvimento porque o comando `cmake` não está instalado no ambiente remoto utilizado para a cópia. A validação final de compilação deve ser feita no host Linux com os comandos do README.

## Recomendação de publicação

Antes de fazer o primeiro `git add`, revise credenciais, IPs, senhas, tokens, dumps de banco e configurações privadas. Mantenha logs e backups fora do repositório de código e utilize serviços systemd ou outro supervisor do host para executar os servidores em produção.
