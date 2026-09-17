# World Yggdrasil Online — Clean Source Edition

This package is a **clean source distribution of the known-working WYO rAthena tree**.
It is intentionally conservative: WYO gameplay/runtime content was not rebased onto a newer rAthena database.

## What is preserved

The following areas are byte-for-byte identical to the uploaded working WYO emulator:

- `db/`
- `npc/`
- `conf/`
- `sql-files/`

The `src/` tree is also preserved except for one Linux build portability line in `src/map/Makefile.in`: Unix Make excludes the internal `population_engine/*` implementation files because they are already unity-included by `population_engine.cpp`.

The Windows Visual Studio solution/project files are unchanged from the working WYO tree.

## Windows / Microsoft Visual Studio

1. Run the optional source check:

   ```powershell
   powershell -ExecutionPolicy Bypass -File tools\wyo-windows\verify-source.ps1
   ```

2. Open `rAthena.sln`.
3. Select the same configuration/platform you normally use for WYO, typically `Release | x64`.
4. Build the solution.
5. Configure MariaDB/IP/inter-server credentials as usual.

Generated `.exe`, `.pdb`, `.obj`, `.ilk`, `.vs` and local `*.vcxproj.user` files are deliberately not shipped. Visual Studio recreates them.

See `doc/WYO/WYO_BUILD_WINDOWS.md`.

## Debian / Linux

Install dependencies:

```bash
sudo tools/wyo-linux/bootstrap-debian.sh
```

Return to a **normal non-root user**, then:

```bash
tools/wyo-linux/verify-source.sh
WYO_COMPILER=clang JOBS=1 tools/wyo-linux/build.sh
```

This creates:

- `login-server`
- `char-server`
- `map-server`
- `web-server`

The standard rAthena build flow also works:

```bash
./configure --enable-epoll
make clean
make -j1 server
```

`JOBS=1` is recommended for the first build because the WYO `skill.cpp` unity translation unit is large.

See `doc/WYO/WYO_BUILD_LINUX.md`.

## Database

For a **new/empty** rAthena database, import the standard schemas first:

```bash
mysql -u <db-user> -p <db-name> < sql-files/main.sql
mysql -u <db-user> -p <db-name> < sql-files/logs.sql
mysql -u <db-user> -p <db-name> < sql-files/web.sql
```

WYO also contains project-specific schemas such as:

- `sql-files/autoattack.sql`
- `sql-files/autofarm_rewards.sql`
- `sql-files/population_engine/cp_population_stats.sql`

If you are moving an **existing WYO server**, prefer restoring your existing WYO database backup rather than recreating it from scratch; that preserves accounts, characters, custom tables and data exactly as your running server expects.

## Production configuration

The source intentionally keeps the original rAthena/WYO defaults, including the standard inter-server `s1/p1` and local MariaDB defaults. Before exposing a host to the Internet:

- create a dedicated MariaDB user instead of using database `root`;
- change the inter-server user/password in the `login` table and server configuration;
- configure public/bind IPs for the host;
- run the emulator as an unprivileged OS user, not root;
- firewall database and internal service ports appropriately.

Prefer placing host-specific overrides in `conf/import/` so the preserved WYO baseline remains easy to compare.

## Integrity

`doc/WYO/WYO_RUNTIME_PRESERVATION.sha256` contains hashes for all files under `db/`, `npc/`, `conf/` and `sql-files/`. The Linux verifier checks this manifest before a build.

See `doc/WYO/WYO_CLEAN_SOURCE_AUDIT.md` for the audit details.
